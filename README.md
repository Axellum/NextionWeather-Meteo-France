# NextionWeather-Meteo-France
Ecran Nextion pour l'affichage des prévisions météo, alertes météo sur votre région, pluie sur 1h00 de Météo France par ESPHome.

# Prérequi:

- Homa assistant.
- ESPHome.
- Un écran Nextion 3.5 basic ou mieux.
- Un module esp32.

# Installation:

- Installer l'intégration Météo France sur HA avec deux ville proche, régler en une sur les prévision par heures.
- Copié la partie Sensor du fichier configuration.yaml dans un éditeur de texte (notepad+++).
- Faite un 'remplacer par' pour: 40_weather_alert avec votre 'région'_weather_alert.
- Faite un 'remplacer par' pour: saint_vincent_de_tyrosse avec votre entitée weather réglé sur jours.
- Faite un 'remplacer par' pour: saint_geours_de_maremne avec votre entitée weather réglé sur heures.
- Copié le tout dans la partie sensor de votre fichier configuration.yaml.
- Tester les erreurs, redémarer.

- Préparer votre ESP avec ESPHome.
- Copié le texte à partir de " uart: ".
- Installer le code sur votre ESP relier à l'écran.



