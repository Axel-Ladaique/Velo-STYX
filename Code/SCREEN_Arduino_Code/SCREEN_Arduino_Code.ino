//CODE FONTIONEL 20/07 16h30 => Affichage sur ecran + réception Json ESP32S3
//20/07 Fix transmission/Enregistrement des données GPS. Fix affichage vitesse et attfichage début.
//16/07 ajout du gps A essayer avec le reste du vélo (verifier l'envoie des données gps lors de l'envoie/reception de la commande message = "____GPS")
//21/06 remise a plat apres les test I2C et bleutooth car ca ne fonctionne pas. 
// 16_16/03 16h40 ===> correction aute orthographe + 1pxl jauges
//fix de la jauge de droite (mtn dans le bon sens)
//Ajout page de demarrage 
//fix distance et rec
//Fix serial 115200
//preparation touch screen (touch screen ready
//toucher pour passer validé

/* Code écrit par Axel LADAIQUE */
/*Ce code a pour but d'afficher les informations reçues du vélo STYX sur une carte de dev avec esp32 S3

Configuration Board => 
-Esp32S3 dev module
-Port Serial(voir problème port/driver)
-Flash Size : 16MB
-Partition scheme : 16M Flash (3MB APP/9.9MB FATFS)
-PSRAM : QSPI PSRAM

/ ! \ voir dans le dossier la librairie qui fonctionne avec les modifications déjà effectuées.

/ ! \ Arduino IDE v2.3.2 || TFT_eSPI v2.5.43 || esp32 by Espressif v2.0.12
*/


#include <TFT_eSPI.h>  // Inclure la bibliothèque TFT_eSPI
#include "LOGO_V1_240x240.h"
#include "CST816S.h"
#include "Image_FOND_240x240.h"
TFT_eSPI tft = TFT_eSPI();  // Création de l'objet pour l'écran
TFT_eSprite imgFond = TFT_eSprite (&tft);
TFT_eSprite speed = TFT_eSprite (&tft);
TFT_eSprite spriteJauge = TFT_eSprite (&tft);
TFT_eSprite spriteTension = TFT_eSprite (&tft);
#include <ArduinoJson.h>
#include <HardwareSerial.h>

HardwareSerial mySerial(1);  // Utilise UART1

CST816S touch(6, 7, 13, 5); // sda, scl, rst, irq           //----------------------------------------------------------------------------------------------

#include <TinyGPS++.h>
#define GPS_RX_PIN 15
#define GPS_TX_PIN 16
HardwareSerial SerialGPS(2);
TinyGPSPlus gps;


void setup() {
  
  Serial.begin(115200);
  delay(1000);  // Attente que la communication série soit prête
  SerialGPS.begin(115200, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);  // UART GPS

  Serial.println("Attente des données GPS...");

  tft.init();              // Initialiser l'écran
  tft.setRotation(2);      // Définir l'orientation (1 pour horizontal, ajuster si besoin)
  tft.fillScreen(TFT_WHITE); // Remplir l'écran en blanc

  tft.setSwapBytes(true);
  tft.pushImage (0,0, 240, 240, LOGO_V1_240x240);    
  imgFond.createSprite(240, 240);
  speed.createSprite(120,80);
  spriteJauge.createSprite(240,240);
  spriteTension.createSprite(30,10);
  imgFond.setSwapBytes(true);
  imgFond.setColorDepth(8);
  tft.setTextColor(TFT_BLACK);
  mySerial.begin(115200, SERIAL_8N1, 17, 18);  // Configurer les pins pour UART1
  
  
  Serial.println("Récepteur prêt.");
  touch.begin();                                 //----------------------------------------------------------------------------------------------

  
  if (true){
    int y=0;
    while (y>-50){
      
      tft.pushImage (0,y, 240, 240, LOGO_V1_240x240);
      y--;
      if (y>-5){delay(60);}
      if (y>-10){delay(30);}
      if (y>-20){delay(10);}
      if (y<-30){delay(10);}
      if (y<-40){delay(30);}
      if (y<-45){delay(60);}
      delay(10);
      if (touch.available()) {
        if (touch.data.y > 0) { // Si la position Y est dans la zone souhaitée
          Serial.println("Écran touché ! Quitte la boucle.");
          break; // Quitter la boucle
        }
      }
    }
    
    //tft.drawString("! Attention ! " ,30, 102, 4);
    tft.drawString("Ce velo est un prototype en cours " ,5, 95, 2);
    tft.drawString("de developpement Le frein moteur" ,3, 110, 2);
    tft.drawString("recharge les supercondensateurs." ,5, 125, 2);
    tft.setTextColor(TFT_RED);
    tft.drawString("Attention : T < 50 V !" ,10, 145, 4);
    //tft.drawString("Ne pas depasser 35 V !" ,10, 140, 2);
    //tft.drawString("Faire attention lors de l'utilisation" ,15, 155, 2);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("Bonne route !" ,25, 170, 2);
    tft.drawString("Prototype 2 - Axel Ladaique" ,33, 185, 2);
    tft.drawString("  Juil. 2025" ,82, 223, 1);
    tft.drawString ("Toucher pour passer", 63, 210, 1);
    
    unsigned long startTime = millis(); // Temps de début
    const unsigned long maxDelay = 30000; // Délai maximum de 15 secondes (en millisecondes)

    while (true) {
        // Si l'écran est touché
        if (touch.available()) {
            if (touch.data.y > 0) { // Si la position Y est dans la zone souhaitée
                Serial.println("Écran touché ! Quitte la boucle.");
                break; // Quitter la boucle
            }
        }

        // Vérifie si 15 secondes se sont écoulées
        if (millis() - startTime >= maxDelay) {
            Serial.println("15 secondes se sont écoulées, fin de la boucle.");
            break; // Quitter la boucle après 15 secondes
        }

        delay(50); // Pause pour éviter une surcharge CPU
    }

  }
  
  
  
  //tft.pushImage (0,0, 240, 240, Image_FOND_240x240) ;

}
float x=0;
int angleDeb = 45;    //45  Angle du Bas
int angleFin = 135;   //135 Angle du Haut
float angleJauge = 0;

float tension = 0;           // Tension (Float)
float vitesse = 0;           // Vitesse (Float)
float gazFrein = 0;        // Gaz/frein (Float)
float whCharged = 0;       // WHcharged (Float)
float whDischarged = 0; // WHdischarged (Float)
float tempsSeconde = 0;// Temps seconde (Float)
float distance = 0;         // Distance (Float)
bool rec = false;                    // Rec (Bool)
float currentIn = 0;      // Current_in (Float)
const char* message = "";     // Message (String)
String currentIn_str = "";

String getGPSJson() {
  StaticJsonDocument<256> doc;

  doc["lat"]  = gps.location.lat();
  doc["lon"]  = gps.location.lng();
  doc["alt"]  = gps.altitude.meters();
  doc["spd"]  = gps.speed.kmph();          // vitesse en km/h
  doc["hdg"]  = gps.course.deg();          // cap en degrés
  doc["sat"]  = gps.satellites.value();    // nb de satellites
  doc["hdop"] = gps.hdop.hdop();           // précision horizontale
  doc["tim"]  = gps.time.value();          // ex: 11325600 pour 11:32:56.00
  doc["dat"]  = gps.date.value();          // ex: 160725 pour 16/07/2025

  String output;
  serializeJson(doc, output);
  return output;
}


void loop() {
  unsigned long t0 = millis();  // début de la mesure
  
  while (SerialGPS.available()) {
    char c = SerialGPS.read();
    gps.encode(c); // Traitement de la trame
    
    if (gps.location.isUpdated()) {
      String gpsJson = getGPSJson();
      Serial.println(gpsJson);
      mySerial.println(gpsJson);

    }
  }
  
  


  if (mySerial.available()) {
    // Lecture de la chaîne JSON envoyée
    String receivedData = mySerial.readStringUntil('\n');
    receivedData.trim(); // Supprime les espaces ou retours à la ligne en début et fin de chaîne

    // Affiche la chaîne brute reçue pour débogage
    Serial.print("Données reçues brutes : ");
    Serial.println(receivedData);

    // Parse le JSON reçu
    StaticJsonDocument<500> doc; // Taille ajustée pour inclure toutes les données
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error) {
        Serial.print(F("Erreur de parsing JSON: "));
        Serial.println(error.c_str());
        return;
    }

    // Récupérer les champs du JSON
    tension = doc["Tension"];           // Tension (Float)
    vitesse = doc["Vitesse"];           // Vitesse (Float)
    gazFrein = doc["Gaz/frein"];        // Gaz/frein (Float)
    whCharged = doc["WHcharged"];       // WHcharged (Float)
    whDischarged = doc["WHdischarged"]; // WHdischarged (Float)
    tempsSeconde = doc["Temps seconde"];// Temps seconde (Float)
    distance = doc["Distance"];         // Distance (Float)
    rec = doc["Rec"];                    // Rec (Bool)
    currentIn = doc["Current_in"];      // Current_in (Float)
    message = doc["Message"];     // Message (String)

    // Affichage des données pour débogage
    Serial.println("Données reçues et parsées :");
    Serial.print("Tension : "); Serial.println(tension);
    Serial.print("Vitesse : "); Serial.println(vitesse);
    Serial.print("Gaz/Frein : "); Serial.println(gazFrein);
    Serial.print("WHchargé : "); Serial.println(whCharged);
    Serial.print("WHdéchargé : "); Serial.println(whDischarged);
    Serial.print("Temps seconde : "); Serial.println(tempsSeconde);
    Serial.print("Distance : "); Serial.println(distance);
    Serial.print("Rec : "); Serial.println(rec ? "true" : "false");
    Serial.print("Courant entrant : "); Serial.println(currentIn);
    Serial.print("Message : "); Serial.println(message);
  }

  if (String(message).substring(4, 6) == "GPS") {
      // Envoie le JSON GPS sur la liaison mySerial
      String gpsJson = getGPSJson();
      mySerial.println(gpsJson);
    }

  // Affiche l'image de fond
  //imgFond.pushImage(0, 0, 240, 240, Design_sans_titre_3); 
  imgFond.pushImage(0, 0, 240, 240, Image_FOND_240x240);

  speed.createSprite(240,240);
  speed.setColorDepth(8);
  speed.fillSprite(TFT_TRANSPARENT);
  speed.setTextColor(TFT_BLACK,TFT_TRANSPARENT);
  //Serial.print("AV PRINT Tension : "); Serial.println(tension);
  //Serial.print("AV PRINT Vitesse : "); Serial.println(vitesse);
  if (vitesse<10)  {
    if (vitesse>0.1) {speed.drawString("0"+String(vitesse,1),55, 73, 6);}
    else { if (vitesse>-0.1){ speed.drawString("00.0",55, 73, 6);} else {speed.drawString(String(vitesse,1),60, 73, 6);}}
  }
  else            {speed.drawString(String(vitesse,1),55, 73, 6);}
  speed.drawString("km/h",155, 97, 2);
  int Nb_sat = gps.satellites.value();
  float Alt = gps.altitude.meters();
  speed.drawString("sat : " + String(Nb_sat) + "; Alt :" + String(Alt,1),60,45,1);
  speed.pushToSprite(&imgFond, 0, 0, TFT_TRANSPARENT);
  speed.deleteSprite();

  spriteTension.createSprite(240,240);
  spriteTension.setColorDepth(8);
  spriteTension.fillSprite(TFT_TRANSPARENT);
  spriteTension.setTextColor(TFT_BLACK,TFT_TRANSPARENT);
  spriteTension.drawString("T = " + String(tension) + " V",60, 132, 4);
  if (currentIn > -10 && currentIn < 10) {currentIn_str = "0" + String(abs(currentIn));} 
  else {currentIn_str=String(abs(currentIn));}
  if (rec==true) {spriteTension.fillCircle(120, 10, 5, TFT_RED);}
  

  spriteTension.drawString("I =   " + currentIn_str + " A",60, 22, 4);
  if (currentIn>0)  {spriteTension.drawString("+",82, 24, 4);}
  else              {spriteTension.drawString("-",82, 24, 4);}
  spriteTension.drawString("D = " + String(distance) + " km",60, 160, 4);
  spriteTension.pushToSprite(&imgFond, 0, 0, TFT_TRANSPARENT);
  spriteTension.deleteSprite();

  x++;
  if (x > 500) x = 0;

  // Superpose la sprite contenant le texte sur l'image de fond
  speed.pushToSprite(&imgFond, 65, 60);  // Ajuste les coordonnées selon la position du texte
  spriteTension.pushToSprite(&imgFond, 0, 0);

  // Affiche l'image de fond avec le texte superposé sur l'écran
  
  //angleDeb=angleDeb+2;

  
  spriteJauge.createSprite(240,240);
  spriteJauge.setColorDepth(8);
  
  spriteJauge.fillSprite(TFT_TRANSPARENT);
  angleJauge=round(abs(gazFrein));
  //Serial.println("angleJauge : "+ String(angleJauge));
  if (gazFrein<0){
    //Serial.println("angleDeb : "+ String(angleDeb) + "  abs(angleJauge) : "+ String(abs(angleJauge)));
    //spriteJauge.drawArc(120, 120, 117, 115, angleDeb, angleFin, TFT_BLUE, TFT_TRANSPARENT);   //gauche //Red --> Blue || Green --> red || Blue --> Green  //  angleDeb = 45 Angle du Bas // angleFin = 135 Angle du Haut
    spriteJauge.drawArc(120, 120, 117, 114, angleDeb, angleDeb+abs(angleJauge), TFT_RED, TFT_TRANSPARENT);
  }
  if (gazFrein>0){
    spriteJauge.drawArc(120, 120, 117, 114, 360-angleFin+90-abs(angleJauge), 360-angleDeb, TFT_GREEN, TFT_TRANSPARENT);            //droite180
  }
  
  
  spriteJauge.pushToSprite(&imgFond, 0, 0, TFT_TRANSPARENT);
  
  imgFond.pushSprite(0, 0);

  spriteJauge.deleteSprite();
  //delay(1000);
  
  unsigned long duration = millis() - t0;  // durée de la boucle
  Serial.print("⏱️ Loop time: ");
  Serial.print(duration);
  Serial.println(" ms");
  

  
}



















