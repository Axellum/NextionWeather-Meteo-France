# Nextion Weather - Météo France ☀️🌧️

Ecran Nextion pour l'affichage des prévisions météo, alertes météo locales et la pluie sur 1h00 de Météo-France via un ESP32. Ce projet a pour but d'afficher l'intégralité des prévisions de la semaine sur un écran déporté autonome ou relié à la domotique.

On peut en discuter ici : [Forum HACF - Nextion Météo France](https://forum.hacf.fr/t/nextion-meteo-france/22525)

## 🎯 Présentation des 4 Architectures
Ce projet a évolué pour s'adapter à toutes les configurations matérielles et logicielles. Des plus simples (V1) aux plus performantes (V2) ou totalement indépendantes (V3/V4). Choisissez celle qui vous correspond dans les différents sous-dossiers !
# Mise en garde:
- Le 03/03/2026: Optimisations du code par Antiravity et Gemini: Meilleur syncronisation des donnée, codage moins volumineu et plus optimisé.
- Le 17/07/2024: Envois des donner par pages: Les donner des page sont charge et change a chaque mise a jurs des capteurs, ca donne une meilleur reactivité. Le buffer du nextion est en overflow (mais pas de probleme visible d affichage), a voir si j arrive a resoudre le probleme.
- Le 15/07/2024: Remise en marche des prévisions suite au changement de l'API et débug des titres des prévisions journaliére. Bref, la version 3.5 remarche!
- En partant du principe qu'il est plus judicieux de partie de ce projet pour le compléter ensuite, j'ai indexer 100 (99 sur l'index Nextion) images pour facilitée d'autres intégrations.
- Le codage est à l'effigie de mon orthographe, fais de brique et de broc. Le fichier contient mon code de travail, pas mal de lignes sont inutile pour cette version, mais peut permetre de compléter plus tard l'écran. Il y as beaucoup de répition que devrait être fais par des boucles, peut être pour plus tard.

### 🏗️ V1 : L'Architecture Classique (HA Templates + ESPHome)
*Idéale pour débuter et comprendre comment Home Assistant transmet ses états à ESPHome.*
* **Principe :** Home Assistant génère plus de 130 capteurs virtuels (Templates) pour décortiquer intégralement chaque attribut officiel Météo-France. ESPHome s'abonne à ces 130 capteurs pour les afficher.
* ✅ **Avantages :** Zéro code C++ dans l'ESP. Facile à lire sous Home Assistant.
* ❌ **Inconvénients :** Surcharge de la base de données (recorder HA). Sensible aux redémarrages (Logs d'erreurs).

### 🚀 V2 : L'Architecture Haute Performance ("Service Push") - **Recommandée !**
*L'architecture parfaite pour une domotique moderne, hyper réactive, et sans faux capteur.*
* **Principe :** Zéro template sous Home Assistant. C'est HA qui regroupe les JSON bruts et les "pousse" vers l'ESP32 via des Services/Actions natives. L'ESP32 gère seul la conversion en images via de véloces scripts C++.
* ✅ **Avantages :** 70% de code yaml en moins, base de données HA ultra-allégée, délai d'affichage instantané (< 500ms) pour la pluie. L'ESP gère l'heure en totale autonomie sans serveur Wi-Fi interne (SNTP).
* ❌ **Inconvénients :** Nécessite de configurer une routine `automations:` sous HA.

### 🌍 V3 : L'Architecture Standalone "Open-Meteo" (Arduino)
*Idéale si vous n'avez pas de serveur domotique Home Assistant chez vous.*
* **Principe :** 100% C++ sur l'IDE Arduino. L'ESP32 se connecte en WiFi et attaque directement l'API gratuite d'Open-Meteo sans compte.
* ✅ **Avantages :** Aucun serveur requis. Plug & Play n'importe où avec juste un login Internet.
* ❌ **Inconvénients :** Perte de la vigilance couleur départementale et de la pluie à 5 min radar (Ce sont des spécificités inaccessibles via cette API tierce).

### ⚡ V4 : L'Architecture Standalone "Météo-France" (Arduino)
*Pour les puristes voulant se passer de domotique locale (HA) tout en conservant 100% des fonctions natives de Météo-France.*
* **Principe :** Un exploit technique ! L'ESP32 se connecte en HTTPS et reproduit l'application smartphone Météo France en usurpant leur "Token" de sécurité. Filtrage temp flux JSON intensifs pour la RAM.
* ✅ **Avantages :** Zéro serveur, restaure les Vigilances Françaises et la pluie Radar sur l'ESP autonome !
* ❌ **Inconvénients :** Fragile sur le long terme. Si Météo France renouvelle la clé Mobile d'ici quelques années, l'écran crashera et la communauté devra chercher un nouveau Token pour reflasher.

---

## 📸 Captures d'écran (Nextion 3.5' et 2.8')

![Nextion meteo france 3.5](/Demo%20ecran%203.5.png)

Au menu :
* Date, horloge locale (NTP/HA).
* Temps Actuel Météo-France.
* Probabilités gel, neige, et indice U.V.
* Prévisions sur 3 jours, ou agenda 8 jours. (Geste de balayage ou boutons)
* Températures et Icones détaillées Heure par Heure (8 H).
* Avertissements vigilances de votre Département (40 par défaut) & Pluie à 5 mins radar (V2/V4).
* Bouton d'intensité tactile (Luminosité Ecran)

---

## 🛠️ Installations (Pré-requis)

* **Matériel :** Un écran Nextion 3.5 ou 2.8 (série Basic/Enhanced/Intelligent), un module ESP32.
* **Logiciels :** Home Assistant, module ESPHome ou le logiciel Arduino IDE (Suivant la route choisie).

### 📖 Procédure "Service Push" (La V2 Recommandée)
1. Installez le fichier `.tft` (`Ecran 3.5/NextionWeather_3.5_3pages_v01.tft`) sur votre écran Nextion via carte SD.
2. Créez un nouveau périphérique dans votre ESPHome et flashez le contenu du fichier `esphome_meteo_35_V2.yaml`.
3. Configurez l'intégration officielle **Météo-France** dans Home Assistant.
4. Ouvrez `automations_meteo_nextion_V2.yaml`, et collez l'automatisation. **Pensez à remplacer `saint_vincent_de_tyrosse` par _votre_ ville et `40_weather_alert` par l'alerte de _votre_ département.** 

### 📖 Procédures Autonomes (La V3 / V4 IDE)
1. Installez l'écran de la même façon (Le fichier TFT est universel).
2. Depuis le gestionnaire de bibliothèque IDE Arduino, téléchargez `ArduinoJson` (v6+) et ouvrez le code `.ino` choisi.
3. Renseignez `ssid`, `password` locaux. Précisez `LATITUDE` et `LONGITUDE`. (Pour la V4, changez la string `DEPARTEMENT`).
4. Flasher sur l'ESP32. Reliez les pins série (RX/TX). Enjoy !

---
*Mentions : Conception par **Axell**. Aide et conseils de pbranly, Nico_206 sur hacf.fr. Architecture technique et code Standalone par **Antigravity I.A.***
