//CODE FONTIONEL 25/01 15h00 => Affichage sur ecran + réception Json ESP32S3
//fix de la jauge de droite (mtn dans le bon sens)
//Ajout page de demarrage 
//fix distance et rec
//Fix serial 115200
//preparation touch screen (touch screen ready

/* Code écrit par Axel LADAIQUE */
/*Ce code a pour but d'afficher les informations reçues du vélo STYX sur une carte de dev avec esp32 S3

Configuration Board => 
-Esp32S3 dev module
-Port Serial(voir problème port/driver)
-Flash Size : 16MB
-Partition scheme : 16M Flash (3MB APP/9.9MB FATFS)
-PSRAM : QSPI PSRAM

/ ! \ voir dans le dossier la librairie qui fonctionne avec les modifications déjà effectuées.

/ ! \ Arduino IDE v2.unsigned long startTime = millis(); // Temps de début
const unsigned long maxDelay = 15000; // Délai maximum de 15 secondes (en millisecondes)

while (true) {
    // Si l'écran est touché
    if (touch.available()) {
        if (touch.data.y < 45) { // Si la position Y est dans la zone souhaitée
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

unsigned long startTime = millis(); // Temps de début
const unsigned long maxDelay = 15000; // Délai maximum de 15 secondes (en millisecondes)

while (true) {
    // Si l'écran est touché
    if (touch.available()) {
        if (touch.data.y < 45) { // Si la position Y est dans la zone souhaitée
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
                                                                                                                                                                                                                                                                                    >0