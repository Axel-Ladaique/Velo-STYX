/* CODE TEST 20/09/2025 19h15 => Main PCB Envoie Json ESP32S3 --> écran.  échange info Vesc <--> esp32S3 (Main PCB)
//!\\ PCB V0.3
//21/09 Ajout du capteur de cadence
//20/09 Passage en PCB V0.3, changement de pin et lecture du capteur de couple
//13/08 Fix distance + Ajout d’un variateur pour le freinage regeneratif. Test de ralentir automatiquement au dessus de 25km peu concluant
//23/07/2025 Si un fichier existe, creation d'un nouveau fichier pour ne pas ecraser le ficher precedent sur la carte SD 
//20/07/25 Enregistrement des données envoyés par le GPS/ecran. 
//21/06 remise a plat apres les test en I2c et bluetooth car ca ne marchait pas on va ensuite tester de metre le gps sur l’écran.
 Validation => récupère les données du VESC et les envoie sur l’écran. 
 Si le VESC est débranché, envoie des données aléatoires.
 Si VESC rebranché, les données reviennent sur l’écran.

 Validation 2. Recuperation de la valeur du frein et de la gachette des gaz avec envoie sur l'ecran.
 Validation 3 : envoie de la commande au vesc : tunes a revoir mais frein et acceerateur fonctionnent
 22/01 19h30 => Fix distance & vitesse 
 25/01 12h50 => re fix distance, fix calcul d’envoie au vesc 
 25/01 14h40 => changement de la vitesse du serial screen en 115200 Bauds, optimisation du temps d'exectuion de la loop.
 01/02 17h30 => changement sur les envoie de donné au vesc, probleme de multiplicateur réglé.
 07/02 Fix Ah charged/discharged ==> Wh charged Discharged. || fix distance facteur 12,1

Modification du 09/05/2025 pour la nouvelle roue. Changement diamètre, nb poles pour vitesse, 

/* Code écrit par Axel LADAIQUE */
/* Ce code a pour but de traiter et d'envoyer les informations du vélo STYX sur un écran de dev avec ESP32 S3 et une carte SD

Configuration Board => 
- Esp32S3 dev module
- Port Serial (voir problème port/driver)
- Flash Size : 16MB
- Partition scheme : 8M with SPIFF ...
- PSRAM : disable

/ ! \ Arduino IDE v2.3.2 || TFT_eSPI v2.5.43 || esp32 by Espressif v2.0.12
*/

#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <VescUart.h>

// UART pour la communication avec l'écran
HardwareSerial SerialScreen(1); // UART1 pour l'écran
#define TX_PIN_SCREEN 15 // Pin TX vers le récepteur (écran)
#define RX_PIN_SCREEN 16 // Pin RX (non utilisé ici)

// UART pour la communication avec le VESC
HardwareSerial SerialVESC(2); // UART2 pour le VESC
VescUart vesc; // Objet pour la bibliothèque VESC
#define TX_PIN_VESC 17 // Pin TX pour le VESC
#define RX_PIN_VESC 18 // Pin RX pour le VESC

// Pins supplémentaires pour alimentation et lecture analogique
#define PIN_FREIN_DROIT 2
#define PIN_FREIN_GAUCHE 1
#define PIN_GAZ 13
const int freinInf = 1050;
const int freinSup = 2500;
const int freinInf_G = freinInf;
const int freinSup_G = freinSup;
const int freinInf_D = freinInf;
const int freinSup_D = freinSup;

const int gazInf = 985;
const int gazSup = 3125;


#include <SD.h>
#include <SPI.h>
// Déclaration des pins SPI personnalisées
#define CS_PIN_SD 39
#define MOSI_PIN 38
#define MISO_PIN 36
#define CLK_PIN 37
File dataFile;

#define PIN_COUPLE 12
#define PIN_CANDENCE 14

volatile unsigned long lastPulseMicros = 0;
volatile unsigned long pulseIntervalMicros = 0;
volatile unsigned long pulseCountCad = 0;

float cadenceRPM = 0.0;
const int pulsesPerRevCad = 36;  // nombre d'impulsions par tour (1 aimant = 1 impulsion)
#define WINDOW_MS 500         // Fenêtre pour moyenne glissante
#define TIMEOUT_MS 300        // Timeout sans impulsion pour remettre cadence à 0
#define ALPHA 0.3             // Filtrage exponentiel

float cadenceFiltered = 0;     // Valeur moyenne lissée
unsigned long lastCadenceUpdate = 0;
volatile unsigned long lastPulseTime = 0; // mis à jour dans l'ISR


bool rec = false;
String currentFilename;

String generateUniqueFilename() {
  String baseFilename = "/STYXvelo_P2_AL";
  String extension = ".csv";
  String filename;
  int counter = 1;
  
  // Vérifier si le fichier de base existe
  filename = baseFilename + extension;
  if (!SD.exists(filename)) {
    return filename; // Le fichier de base n'existe pas, on peut l'utiliser
  }
  
  // Chercher un nom de fichier libre avec un numéro
  do {
    filename = baseFilename + "_" + String(counter) + extension;
    counter++;
  } while (SD.exists(filename) && counter < 1000); // Protection contre boucle infinie
  
  return filename;
}

void IRAM_ATTR isr_cadence() {
  unsigned long now = micros();
  unsigned long interval = now - lastPulseMicros;
  if (interval > 20000) { // ignore <20ms (anti-rebond)
    pulseIntervalMicros = interval;
    pulseCountCad++;
    lastPulseMicros = now;
    lastPulseTime = millis(); // <- pour le timeout
  }
}




void setup() {
  // Initialisation des UART
  SerialScreen.begin(115200, SERIAL_8N1, RX_PIN_SCREEN, TX_PIN_SCREEN);
  SerialVESC.begin(115200, SERIAL_8N1, RX_PIN_VESC, TX_PIN_VESC);

  // Association explicite du port série avec l'objet VESC
  vesc.setSerialPort(&SerialVESC);

  // Moniteur série pour le débogage
  Serial.begin(115200);
  delay(1000);



  // Configuration des pins de frein, gaz et couple pour lecture analogique
  pinMode(PIN_FREIN_GAUCHE, INPUT);
  pinMode(PIN_FREIN_DROIT, INPUT);
  pinMode(PIN_GAZ, INPUT);
  pinMode(PIN_COUPLE, INPUT);
  pinMode(PIN_CANDENCE, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_CANDENCE), isr_cadence, FALLING);
  analogReadResolution(12);           // Résolution ADC : 12 bits
  analogSetAttenuation(ADC_11db);     // Plage de lecture : 0 à 3.3V

  Serial.println("Émetteur prêt avec :");
  Serial.println("- UART1 : TX=15, RX=16 pour le récepteur (écran)");
  Serial.println("- UART2 : TX=17, RX=18 pour le VESC");

  
  SPI.begin(CLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN_SD);
  // Initialisation de la carte SD
  if (!SD.begin(CS_PIN_SD, SPI)) {
    Serial.println("Erreur : Impossible d'initialiser la carte SD !");
    //while (true);
  }
  Serial.println("Carte SD initialisée avec succès.");

  // Générer un nom de fichier unique
  currentFilename = generateUniqueFilename();
  Serial.println("Nom du fichier : " + currentFilename);
  
  // Ouvrir le fichier pour écriture (crée un nouveau fichier)
  dataFile = SD.open(currentFilename, FILE_WRITE);
  if (dataFile) {
    // Écriture des en-têtes
    dataFile.println("Temps,Tension,Vitesse,GazFrein,WHCharged,WHDischarged,Distance,CurrentIn,MotorCurrent,Lat,Lon,Alt,Vsat,Cap,Sat,HDOP,Heure,Date,Couple,CadenceRPM");
    rec = true;
    dataFile.close();
  } else {
    Serial.println("Erreur : Impossible de créer le fichier " + currentFilename + " !");
    rec = false;
  }

}

float coef_couple = 1.0;
float current_motor;
int lastsd_MS=0;
float vari_vesc = 0;
int nb = 0;
float couple_moy =0;

void loop() {
  int millisdeb = millis();
  nb = nb+1;

  // Lecture des valeurs analogiques
  int frein_G = analogRead(PIN_FREIN_GAUCHE);
  int frein_D = analogRead(PIN_FREIN_DROIT);
  //Serial.print("Valeur frein_G Brute (pin 19) : ");
  //Serial.println(frein_G);
  frein_G = map(frein_G, freinInf_G, freinSup_G, 0, -90);
  //Serial.print("Valeur frein_D Brute (pin 12) : ");
  //Serial.println(frein_D);
  frein_D = map(frein_D, freinInf_D, freinSup_D, 0, -90);
  int frein = frein_G + frein_D;
  if (frein < -90) {frein = -90;}

  int gaz = analogRead(PIN_GAZ);
  //Serial.print("Valeur gaz (pin 20) : ");
  //Serial.println(gaz);
  gaz = map(gaz, gazInf, gazSup, 0, 90);
  float couple = analogRead(PIN_COUPLE);
  Serial.print("couple brut : " );
  Serial.print(couple);
  couple = 0.0267*couple-22.948;
  if (couple<0){
    couple = 0;
  }
  //float cadence = cadenceRPM;
  Serial.print("||  couple : " );
  Serial.print(couple);
  Serial.print(" Nm    nb : " );
  Serial.print(nb);
  couple_moy = (couple_moy+couple);
  Serial.print("   || couple_moy : " );
  Serial.print(couple_moy/nb);
  // Calcul cadence (RPM)
  unsigned long now = millis();
  // Calcul toutes les WINDOW_MS
  if (now - lastCadenceUpdate >= WINDOW_MS) {
      lastCadenceUpdate = now;

      noInterrupts();
      unsigned long pulses = pulseCountCad;   // nombre d’impulsions dans la fenêtre
      pulseCountCad = 0;                       // remise à zéro
      interrupts();

      float cadenceRPM = 0.0;
      if (pulses > 0) {
          float revPerSec = (float)pulses / (float)pulsesPerRevCad / (WINDOW_MS / 1000.0);
          cadenceRPM = revPerSec * 60.0;
      }

      // Timeout si plus d’impulsion depuis TIMEOUT_MS
      if (now - lastPulseTime > TIMEOUT_MS) {
          cadenceRPM = 0.0;
      }

      // Filtrage exponentiel pour lisser
      cadenceFiltered = (1 - ALPHA) * cadenceFiltered + ALPHA * cadenceRPM;

      Serial.print("Cadence filtrée (RPM) : ");
      Serial.println(cadenceFiltered);
  }
  //delay(1000);
  // Affichage des valeurs analogiques
  /*Serial.print("Valeur frein (pin 12) : ");
  Serial.println(frein);
  Serial.print("Valeur gaz (pin 14) : ");
  Serial.println(gaz);*/
  float gazfrein = 0;
  if (gaz>90){gaz=90;}
  
  if (frein<0) {gazfrein=frein;}
  else {
    if (gaz <0){gazfrein=0;}
    else{gazfrein=gaz;}
  }

  //Serial.println( "temps gazfrein : " + String (millis() - millisdeb));
  //Serial.print("Valeur gazFREIN : ");
  //Serial.println(gazfrein);

  // Préparation des données JSON
  StaticJsonDocument<500> doc;

  doc["Gaz/frein"] = gazfrein;
  doc["Temps seconde"] = millis() / 1000.0;

  float vitesse = 0;

  if (vesc.getVescValues()) {
    // Lecture des valeurs du VESC
    //Serial.println("Vesc");
    doc["Tension"] = vesc.data.inpVoltage;
    float rpm = vesc.data.rpm/10;                         // The ’10’ is the number of pole pairs in the motor. the motors have 52 poles, therefore 26 pole pairs
    vitesse = rpm*60*PI*(28*0.0254)/(1000)/4.4;       //vitesse en Km/h
    //Serial.println("RPM :" + String(rpm) + "vitesse :" + String(vitesse));
    doc["Vitesse"] = vitesse; // Convertir RPM en vitesse
    doc["WHcharged"] = vesc.data.wattHoursCharged;
    doc["WHdischarged"] = vesc.data.wattHours;
    float tach = vesc.data.tachometer/(20*3);           // The '52' is the number of motor poles multiplied by 3 (3hall sensor I think)
    //Serial.println ("tach : " + String(tach));
    float distance = tach*3.142*(0.001)*(28.0*0.0254)/4.4;       // 3.142*(0.0001)*(26.0*0.0254) //26*0.0254 = diametre de la roue (26 pouces *0,0254). Je ne sais pas pk *12.1
    doc["Distance"] = distance;
    //Serial.println ("Disatnce : " + String(distance));
    doc["Current_in"] = vesc.data.avgInputCurrent;
    doc["Message"] = "VESC_OK";
  } else {
    Serial.println("pas vesc");
    doc["Tension"] = 99.99;
    doc["Vitesse"] = 99.9; // Convertir RPM en vitesse
    doc["WHcharged"] = 99.99;
    doc["WHdischarged"] = 99.99;
    doc["Distance"] = 999.99;
    doc["Current_in"] = 99.99;
    // En cas d'échec de lecture du VESC
    doc["Message"] = "VESC_NOK";
  }
  current_motor=vesc.data.avgMotorCurrent;
  doc["Rec"] = rec;
  //Serial.println("doc Rec" + String(rec));
  // Sérialisation des données JSON
  String jsonOutput;
  serializeJson(doc, jsonOutput);

  //Serial.println( "temps json+infovesc : " + String (millis() - millisdeb));

  // Envoi des données JSON via UART (écran)
  SerialScreen.println(jsonOutput);
  //Serial.println("Longueur de jsonOutput : " + String(jsonOutput.length()));

  //Serial.println( "temps send ecran : " + String (millis() - millisdeb));

  // Affichage pour le débogage
  //Serial.println("Données envoyées :");
  //Serial.println(jsonOutput);

  // Enregistrer les données dans un fichier CSV
  if (vesc.getVescValues()) {
    if (millis()-lastsd_MS>500){
      if (rec==true){
      
        Serial.println("if milis + rec ");
        if (SerialScreen.available()) {
          Serial.println("serial screen");
          // Lecture de la chaîne JSON envoyée
          String receivedData = SerialScreen.readStringUntil('\n');
          receivedData.trim(); // Supprime les espaces ou retours à la ligne en début et fin de chaîne

          // Affiche la chaîne brute reçue pour débogage
          Serial.print("Données reçues brutes : ");
          Serial.println(receivedData);

          // Parse le JSON reçu
          StaticJsonDocument<256> doc_gps;
          DeserializationError error = deserializeJson(doc_gps, receivedData);

          if (error) {
            Serial.print(F("Erreur de parsing JSON: "));
            Serial.println(error.c_str());
            return;
          }

          // Récupérer les champs du JSON GPS
          float latitude     = doc_gps["lat"];       // Latitude (Float)
          float longitude    = doc_gps["lon"];     // Longitude (Float)
          float altitude     = doc_gps["alt"];     // Altitude (Float)
          float vitesse_sat  = doc_gps["spd"];     // Vitesse satellite (Float)
          float cap          = doc_gps["hdg"];     // Cap (Float)
          int satellites     = doc_gps["sat"];     // Nombre de satellites (Int)
          float hdop         = doc_gps["hdop"];    // Précision horizontale (Float)
          unsigned long heure_GMT = doc_gps["tim"]; // Heure brute GMT (ULong)
          unsigned long date       = doc_gps["dat"]; // Date brute (ULong)

          // Affichage des données GPS pour débogage
          Serial.println("Données GPS reçues et parsées :");
          Serial.print("Latitude : "); Serial.println(latitude, 6);
          Serial.print("Longitude : "); Serial.println(longitude, 6);
          Serial.print("Altitude (m) : "); Serial.println(altitude);
          Serial.print("Vitesse satellite (km/h) : "); Serial.println(vitesse_sat);
          Serial.print("Cap (°) : "); Serial.println(cap);
          Serial.print("Satellites : "); Serial.println(satellites);
          Serial.print("HDOP : "); Serial.println(hdop);
          Serial.print("Heure GMT brute : "); Serial.println(heure_GMT);
          Serial.print("Date brute : "); Serial.println(date);

          dataFile = SD.open(currentFilename, FILE_APPEND);
          if (dataFile) {
            rec=true;
            dataFile.print(doc["Temps seconde"].as<float>());
            dataFile.print(",");
            dataFile.print(doc["Tension"].as<float>());
            dataFile.print(",");
            dataFile.print(doc["Vitesse"].as<float>());
            dataFile.print(",");
            dataFile.print(doc["Gaz/frein"].as<float>());
            dataFile.print(",");
            dataFile.print(doc["WHcharged"].as<float>());
            dataFile.print(",");
            dataFile.print(doc["WHdischarged"].as<float>());
            dataFile.print(",");
            dataFile.print(doc["Distance"].as<float>());
            dataFile.print(",");
            dataFile.print(doc["Current_in"].as<float>());
            dataFile.print(",");
            dataFile.print(current_motor);
            dataFile.print(",");
            dataFile.print(doc_gps["lat"].as<float>(),6);
            dataFile.print(",");
            dataFile.print(doc_gps["lon"].as<float>(),6);
            dataFile.print(",");
            dataFile.print(doc_gps["alt"].as<float>());
            dataFile.print(",");
            dataFile.print(doc_gps["spd"].as<float>());
            dataFile.print(",");
            dataFile.print(doc_gps["hdg"].as<float>());
            dataFile.print(",");
            dataFile.print(doc_gps["sat"].as<int>());
            dataFile.print(",");
            dataFile.print(doc_gps["hdop"].as<float>());
            dataFile.print(",");
            dataFile.print(doc_gps["tim"].as<unsigned long>());
            dataFile.print(",");
            dataFile.print(doc_gps["dat"].as<unsigned long>());
            dataFile.print(",");
            dataFile.print(couple_moy/nb);
            dataFile.print(",");
            dataFile.println(cadenceFiltered);
            dataFile.close();
          } else {
            rec=false;
          //Serial.println("Erreur : Impossible d'écrire dans le fichier datalog.csv !");
          } 
        }
      } 
      nb = 0;
      couple_moy =0;
      lastsd_MS=millis();
    }
  }

    //Serial.println( "temps sd : " + String (millis() - millisdeb));
  //-------------------------------------------------------------------------------------------
  //Envoie de données au Vesc 
  float comVesc = 0;
  comVesc = gazfrein/90;  //remet entre -1 et 1
  
  //Serial.println("comVesc avant : " + String(comVesc));
  //Serial.println("comvesc" + String(comVesc));
  if (comVesc<-0.1){
    if (vari_vesc>(comVesc*25)){
      vari_vesc = vari_vesc - 0.4;  
    }else{
      vari_vesc=comVesc*25;
    }
    vesc.setBrakeCurrent(vari_vesc); // entre 0 et -25A
  }
  Serial.print("vari_vesc : ");
  Serial.println(vari_vesc);
  Serial.print("comvesc *25 : ");
  Serial.println(comVesc*25);

  if (comVesc>0.1){
    vari_vesc = 0;
    comVesc=comVesc*15;
    vesc.setCurrent(comVesc);        //entre 0 et 20A
    
  }
  if (vitesse>25){
    vesc.setRPM(820); //820 RPM <=> 25 km/h
    Serial.print("au dessus de 25km/h : ");
    Serial.println(vitesse);
    comVesc=0;
  }
  if (comVesc>-0.1 && comVesc<0.1){ 
    comVesc=0; 
    vari_vesc = 0;
    vesc.setCurrent(comVesc);
  }

  //Serial.println( "temps fin : " + String (millis() - millisdeb));
  
  //Serial.println("comVesc apres : " + String(comVesc));
  


  //-------------------------------------------------------------------------------------------


  //delay(40); // Pause avant prochaine boucle
  //delay(2000);
}













