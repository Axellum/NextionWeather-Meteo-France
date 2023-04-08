
# NextionWeather-Meteo-France
Ecran Nextion pour l'affichage des prévisions météo, alertes météo sur votre région, pluie sur 1h00 de Météo France par ESPHome. Code préparé sous Home assistant, passage sur un ESP32 par ESPHome, affichage sur écran Nextion 3.5 basic.

# Mise en garde:

- 07/04/2023: Passage sur trois pages, gestion en global. Ajout des jours de semaine en titre des prévision. Version 2.8 basique à tester.

- Le codage est à l'effigie de mon orthographe, fais de brique et de broc. Le fichier contient mon code de travail, pas mal de lignes sont inutile pour cette version, mais peut permetre de compléter plus tard l'écran. Il y as beaucoup de répition que devrait être fais par des boucles, peut être pour plus tard.

# Présentation:

![](/Demo%20ecran%203.5.png)

Sur les icones de prévisions en bas ce trouve les "boutons": au centre la luminositée, à droite les prévisions sur 8 jours, à gauche sur 8h00. L'icone principale du temps, l'heure et la date peuvent faire des boutons, pour des pages, interupteurs, apelle de script ex..

Au menu:
- Date et horloge.
- Temps actuel.
- Détecteur gel, neige, couleur pour la quantité d'U.V..
- Prévision sur trois jours (+ du jour en cour), avec températures mini/maxi.
- Luminosité de l'écran réglable sur les icones "Journée" et "3 jours".
- Couleurs des températures des sondes en fonction de la valeur géré par l'écran.
- Changement de couleur de texte et de l'icone de vigilance par l'écran.
- Alerte vigilance Météo France par icones et couleur de la date.
- Pluie sur 1h00, prochaine averse et icone de couleur pour la probabilité.

- Dev: Page de prévision sur 8 jours. OK
- Dev: Page de prévision sur 8h00. OK
- Dev: Page d'infos pour les autres infos de l'API Météo France.
- Dev: Affichage des jours de semaine en titre des prévisions. OK
- Dev: Visuel des prévisions du cumul prévus de pluie.

Ecran 3.5:
![Nextion meteo france 3.5](/PXL_20230327_060610910.jpg)
Ecran 2.8:          
![Nextion meteo france 2.8](/PXL_20230408_131318487.jpg)


Vous trouverez sur ce git:
- Le fichier GIMP afin de pouvoir modifier l'esthétique, adapter les icones ex.
- Le fichier HMI pour l'écran Nextion.
- Le fichier yaml pour l'esp32.
- Le fichier configuration.yaml de Home assistant, à adapter aussi avec son intégration Météo France.

Les icônes viennent de https://icon-icons.com/fr/pack/The-Weather-is-Nice-Today/1370, création de Laura Reen. J'ai parfois apporté de légère modifications.

# Prérequis:

- Homa assistant.
- ESPHome.
- Un écran Nextion 3.5 basic ou mieux.
- Un module esp32.

# Installation:
Home assistant:
- Installer l'intégration Météo France sur HA avec deux ville proche, réglez en une sur les prévision par heures.
- Ouvrer le fichier "template_sensors/meteo_nextion" dans un éditeur de texte (notepad+++).
- Faite un 'remplacer par' pour: 40_weather_alert avec votre 'région'_weather_alert.
- Faite un 'remplacer par' pour: saint_vincent_de_tyrosse avec votre entitée weather réglé sur jours.
- Faite un 'remplacer par' pour: saint_geours_de_maremne avec votre entitée weather réglé sur heures.
- Créer un dossier template_sensors/ au niveau de votre fichier configuration.yaml et placer votre fichier meteo_nextion.yaml modifié.
- Ajouter les ligne "template: / binary_sensor: !include_dir_merge_list template_binary_sensors" comme dans le fichier configuration.yaml.
- Tester les erreurs, redémarrer.

Ecran Nextion:
- Installer le fichier "Ecran 3.5"/"NextionWeather_3.5_3pages_v01.tft" dans votre écran Nextion basic 3.5 (ou "2.8_Test/NextionWeather_2.8_3pages_v01.tft" pour les 2.8 basique, ou avec le .tft générer par les fichier .hmi présent dans outils aprés modifications).

L'esp:
- Préparer votre ESP avec ESPHome.
- Copié le texte à partir de " uart: " du fichier "Ecran 3.5"/"meteo35.yaml" (ou 2.8_test/esphome_meteo_28.yaml) en plus du code généré par ESPHome précédament créer.
- Installer le code sur votre ESP relier à l'écran.
- Redémarrer.
- Intégrer le nouvelle appareil dans Home Assistant.

Merci à pbranly pour ça participation.
