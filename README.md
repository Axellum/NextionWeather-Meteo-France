# NextionWeather-Meteo-France
Ecran Nextion pour l'affichage des prévisions météo, alertes météo sur votre région, pluie sur 1h00 de Météo France par ESPHome.



![](/PXL_20230327_060610910.jpg)

Au menu:
- Date et horloge.
- Temps actuel.
- Détecteur gel, neige, couleur pour la quantitée d'U.V..
- Prévision sur trois jours (+ du jour en cour), avec températures mini/maxi.
- Luminositée de l'écran réglable sur les icones "Journée" et "3 jours".
- Couleurs des températures des sondes en fonction de la valeur géré par l'écran.
- Changement de couleur de texte et de l'icone de vigilence par l'écran, via un champ cacher.
- Alerte vigilence Météo France par icones et couleur de la date.
- Pluie sur 1h00, prochaine averce et icone de couleur pour la probabilitée.

![](/Demo%20ecran%203.5.png)

Vous trouverez sur ce git:
- Le fichier GIMP afin de pouvoir modifier l'esthétique, adapter les icones ex.
- Le fichier HMI pour l'écran Nextion.
- Le fichier yaml pour l'esp32, à adapter en fonction de ses capteurs / interrupteur.
- Le fichier configuration.yaml de Home assistant, à adapter aussi avec son integration Météo France et ses capteurs.

Les icônes viennent de https://icon-icons.com/fr/pack/The-Weather-is-Nice-Today/1370, création de Laura Reen. J'ai parfois apporté de légère modifications.

# Prérequi:

- Homa assistant.
- ESPHome.
- Un écran Nextion 3.5 basic ou mieux.
- Un module esp32.

# Installation:
Home assistant:
- Installer l'intégration Météo France sur HA avec deux ville proche, réglez en une sur les prévision par heures.
- Copié la partie Sensor du fichier configuration.yaml dans un éditeur de texte (notepad+++).
- Faite un 'remplacer par' pour: 40_weather_alert avec votre 'région'_weather_alert.
- Faite un 'remplacer par' pour: saint_vincent_de_tyrosse avec votre entitée weather réglé sur jours.
- Faite un 'remplacer par' pour: saint_geours_de_maremne avec votre entitée weather réglé sur heures.
- Copié le tout dans la partie sensor de votre fichier configuration.yaml.
- Tester les erreurs, redémarer.

Ecran Nextion:
- Installer le fichier "Ecran 3.5"/"NextionWeather 3.5.tft" dans votre écran Nextion basic 3.5.

L'esp:
- Préparer votre ESP avec ESPHome.
- Copié le texte à partir de " uart: " du fichier "Ecran 3.5"/"meteo35.yaml" en plus du code généré par ESPHome.
- Installer le code sur votre ESP relier à l'écran.
- Redémarer.



