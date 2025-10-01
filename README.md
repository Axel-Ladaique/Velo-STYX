# Velo-STYX
Codes et modèles 3D pour s'inspirer ou fabriquer le vélo styx chez soi 
## BOM : Liste des composants
Voici ci dessous la Liste des composants.

[BOM Excel (Original)](https://github.com/Axel-Ladaique/Velo-STYX/blob/main/01_BOM%20Liste%20des%20composants/1_BOM%20STYX%20V2.5.xlsx)

[BOM google sheet](https://docs.google.com/spreadsheets/d/19vOGE_QkLGgu38JTzU2YtKTNE64U9HrzXLUYEj6seYQ/edit?hl=fr&gid=0#gid=0)

<img width="1133" height="657" alt="Capture d’écran 2025-09-30 à 20 37 25" src="https://github.com/user-attachments/assets/3ee97ba2-1bb6-4fdb-8d03-57951fa890c3" />

# Informations concernant les fichiers STEP

- Une fois l’archive décompressée, vous trouverez un fichier au format **.STEP**.  
- Ce fichier contient les modèles 3D présentés ci-dessous :
<img width="1064" height="572" alt="Capture d’écran 2025-09-22 à 15 25 45" src="https://github.com/user-attachments/assets/d49a0492-0c5e-486f-a2c4-605321a87d47" />

# Fabrication de la carte PCB

Pour fabriquer la carte PCB, il suffit d’envoyer le fichier V0.3 (compressé en .zip) à un fabricant en ligne comme PCBWay. Une fois la commande validée, il faut régler le paiement : petit conseil, choisir la livraison lente revient beaucoup moins cher et reste fiable.
À la réception des cartes, commencez par souder les connecteurs PH2.0, puis les connecteurs JST. Une fois les soudures terminées, insérez l’ESP32 dans le sens indiqué sur la PCB, et la carte est prête à l’emploi.

Ci-dessous, la carte PCB V0.3 : vue de dessus, vue de niveau et vue de dessous.
<p align="left">
  <img width="250" alt="PCB_V0 3_top_view" src="https://github.com/user-attachments/assets/b1e63832-8149-417a-ab32-6b162f6985c9" />
  <img width="250" alt="PCB_V0 3_layer_view" src="https://github.com/user-attachments/assets/356efb8a-082e-4e4e-9af9-c0efc16f8346" />
  <img width="250" alt="PCB_V0 3_bottom_view" src="https://github.com/user-attachments/assets/04891eed-ac97-4158-a4ce-5c7713187e3c" />
</p>

## Marche à suivre pour la roue motorisé
Pour avoir une roue motorisé qui foncitonne avec la frenage regeneratif, il nous faut un moteur particulier. Notre moteur doit etre un moteur sans roue libre. Pour cela on peut opter pour un moteur direct drive (DD) ou un geard motor. 


### Moteur direct drive
Pour la version direct drive, c'est plutôt simlpe, il suffit d'acheter un moteur Direct drive intégré dans une roue arrière de vélo.
On nottera que le moteur direct drive est plus lourd et encombrant. Ce n'est pas la version retenue pour le vélo STYX v2.5 (Utilisé dans le vélo STYX V1.0)
Voici la roue que j'ai utilisé (v1.0) : [Pack Hub moteur roue arriere 26" 36V 500W](https://www.amazon.fr/dp/B0CFFTQTP3?ref=ppx_yo2ov_dt_b_product_details&th=1) 

⚠️ Cette roue est une roue 26" et n'est pas adapté pour la V2.5 (nombre d'aimant, capteur hall). Il faudra donc faire des modifications pour adapter une roue de ce type sur la V2.5
Cette roue est proposé comme une alternative à la solution geared motor

Ci-dessous, la roue avec moteur direct drive sur le velo STYX V1.0 et le moteur ouvert.
<p align="left">
  <img height="500" alt="PCB_V0 3_top_view" src="https://github.com/user-attachments/assets/e6a866ee-024e-47e8-a3d6-72a5b08896ef" />
  <img height="500" alt="PCB_V0 3_layer_view" src="https://github.com/user-attachments/assets/c795ef1c-c037-49cf-bbf0-44a205996821" />
</p>

### geared motor
Sur la version 2.5 du vélo STYX, un moteur réducé est utilisé. Cela permet de reduire l'encombrement mais aussi et surtout le poid, c'est pour cela que la plupart des vélo électriques sur le marché sont équipés de ce type de moteur. Le moteur réducé est composé d'un moteur electrique entrainte un reducteur planetaire. Ce reducteur reduit la vitesse de rotation mais augmente le couple de sortie. Aussi, sur les vélos electriques classiques ce reducteur fait aussi office de roue libre. cela veut dire que le moteur entraine la roue dans un sens de rotation mais pas dans l'autre. Cela permet que le moteur entraine la roue lorsque l'assistance est demandé, mais que lorsque la roue tourne, la moteur n'est pas entrainé. cela augmente l'efficaité car le moteur génère une trainé electromagnetique losqu'il est entrainé.
Or, dans notre système, nous ne voulons pas avoir de roue libre car notre moteur doit etre entrainé par la roue pour faire du freinage régéneratif. C'est pour cela que notre moteur doit être modifié par rapport au moteurs généralement vendu dans le commerce.

Ci-dessous, des images de porte satélite avec et sans roue libre.
<p align="left">
  <img height="300" alt="PCB_V0 3_top_view" src="https://github.com/user-attachments/assets/53034b32-ec19-4984-8eb3-1d23eb7abf58" />
  <img height="300" alt="PCB_V0 3_layer_view" src="https://github.com/user-attachments/assets/c29f9612-a29e-4aa5-a4ad-a2ef45bedf2e" />
</p>


