/*
  Meteo35 - Version V3 (Standalone / Autonome)
  Compatible : ESP32 dans l'IDE Arduino.
  
  Description : Ce code se connecte directement au Wi-Fi, récupère l'heure 
  via NTP, lance des requêtes HTTPS vers l'API Open-Meteo (car l'API 
  Météo-France nécessite un token OAuth lourd à gérer sur microcontrôleur) 
  et pilote un écran Nextion sur les broches matérielles (Serial2).
  
  Auteur : Antigravity IA 
  Date : Mars 2026
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ==============================================================================
// 1. CONFIGURATION RÉSEAU ET MÉTÉO
// ==============================================================================
const char* ssid     = "TON_WIFI_SSID";
const char* password = "TON_WIFI_PASSWORD";

// Coordonnées GPS (Ex: Saint-Vincent-de-Tyrosse)
const String LATITUDE  = "43.66";
const String LONGITUDE = "-1.30";

// URL de l'API Open-Meteo (Gratuite, sans clé d'API, intégre les modèles Arome/Arpege de Météo-France)
const String apiUrl = "https://api.open-meteo.com/v1/forecast?latitude=" + LATITUDE + 
                      "&longitude=" + LONGITUDE + 
                      "&current=temperature_2m,relative_humidity_2m,weather_code" +
                      "&hourly=temperature_2m,precipitation,weather_code" +
                      "&daily=weather_code,temperature_2m_max,temperature_2m_min,uv_index_max,precipitation_probability_max" +
                      "&timezone=Europe%2FParis&forecast_days=9";

// Fréquence de rafraîchissement météo en millisecondes (ex: 15 minutes)
const unsigned long WEATHER_UPDATE_INTERVAL = 15 * 60 * 1000;
unsigned long lastWeatherUpdate = 0;

// Configuration Heure NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;       // Décalage horaire (Paris = +1h)
const int   daylightOffset_sec = 3600;  // Heure d'été (+1h)

// Configuration Port Série Nextion (ESP32: RX=16, TX=17 pour Serial2)
#define NEXTION_SERIAL Serial2
#define NEXTION_RX 16
#define NEXTION_TX 17

// ==============================================================================
// 2. FONCTONS UTILITAIRES NEXTION
// ==============================================================================

// Envoie une commande texte simple au Nextion (et termine par 3 octets 0xFF)
void nextionSendCmd(String cmd) {
  NEXTION_SERIAL.print(cmd);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

// Modifie le texte d'un composant Nextion (ex: "p[0].temp_1","25.4")
void nextionSetText(String component, String text) {
  nextionSendCmd(component + ".txt=\"" + text + "\"");
}

// Modifie la valeur (nombre) d'un composant Nextion
void nextionSetVal(String component, int val) {
  nextionSendCmd(component + ".val=" + String(val));
}

// Modifie l'image (pic) d'un composant Nextion
void nextionSetPic(String component, int picId) {
  nextionSendCmd(component + ".pic=" + String(picId));
}

// Convertit un code météo WMO (Open-Meteo) vers l'ID d'image Nextion (0 à 32)
// Basé sur le mapping que nous avions fait en ESPHome/Météo France
int weatherCodeToPic(int wmoCode, bool isDay = true) {
  switch (wmoCode) {
    case 0: return isDay ? 22 : 25; // Ciel clair (Clear/sunny / clear-night)
    case 1: 
    case 2: return 20; // Peu nuageux (partlycloudy)
    case 3: return 19; // Couvert (cloudy)
    case 45:
    case 48: return 23; // Brouillard (fog)
    case 51:
    case 53:
    case 55: return 26; // Bruine (rainy léger)
    case 61: return 26; // Pluie faible
    case 63: return 27; // Pluie modérée (pouring léger)
    case 65: return 27; // Pluie forte (pouring)
    case 71:
    case 73:
    case 75:
    case 77: return 29; // Neige (snowy)
    case 80:
    case 81:
    case 82: return 26; // Averses de pluie
    case 85:
    case 86: return 29; // Averses de neige
    case 95: return 31; // Orage (lightning)
    case 96:
    case 99: return 24; // Orage avec grêle/pluie (lightning-rainy)
    default: return 3;  // Inconnu
  }
}

// Convertit le WMO en ID d'image pour "l'icône principale du jour"
// (Utilisait les IDs 4, 5, 7, etc. dans ton fichier HA d'origine)
int weatherCodeToMainIcon(int wmoCode, bool isDay = true) {
  switch (wmoCode) {
    case 0: return isDay ? 7 : 10;
    case 1:
    case 2: return 5;
    case 3: return 4;
    case 45:
    case 48: return 8;
    case 51:
    case 53:
    case 55:
    case 61:
    case 80: return 11; // rainy
    case 63:
    case 65:
    case 81:
    case 82: return 12; // pouring
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86: return 14; // snowy
    case 95: return 16; // lightning
    case 96:
    case 99: return 9;  // lightning-rainy
    default: return 2;  // inconnu
  }
}

// ==============================================================================
// 3. RECUPERATION ET TRAITEMENT DES DONNEES
// ==============================================================================

void fetchWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Erreur: WiFi non connecté.");
    return;
  }

  HTTPClient http;
  Serial.println("Recherche météo: " + apiUrl);
  http.begin(apiUrl);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    
    // Allocation mémoire pour JSON (La taille est large pour contenir 9 jours)
    DynamicJsonDocument doc(24576); 
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("Erreur de décodage JSON: ");
      Serial.println(error.c_str());
    } else {
      Serial.println("JSON météo décodé avec succès ! Mise à jour écran...");
      updateNextionDisplay(doc);
    }
  } else {
    Serial.printf("Erreur HTTP: %d\n", httpCode);
  }
  http.end();
}

void updateNextionDisplay(DynamicJsonDocument& doc) {
  // --------- Météo Actuelle (Current) ---------
  float currentTemp = doc["current"]["temperature_2m"];
  int currentHum = doc["current"]["relative_humidity_2m"];
  int currentWmo = doc["current"]["weather_code"];
  
  nextionSetVal("p[0].temp_1", (int)currentTemp);
  nextionSetVal("p[0].hum_1", currentHum);
  nextionSetPic("p[0].icon", weatherCodeToMainIcon(currentWmo, true)); // Assume jour

  // --------- Proba Pluie et UV (Extrapolés depuis le tableau Daily jour[0]) ---------
  int probaPluie = doc["daily"]["precipitation_probability_max"][0];
  int indexUV = (int)doc["daily"]["uv_index_max"][0];

  nextionSetPic("p[0].alerte_rain", (probaPluie < 9) ? 80 : (probaPluie < 40) ? 79 : (probaPluie < 70) ? 78 : 77);
  nextionSetPic("p[0].uv", (indexUV < 5) ? 85 : (indexUV < 7) ? 84 : (indexUV < 9) ? 83 : (indexUV < 11) ? 82 : 81);

  // --------- Prévisions Journalières (9 Jours) ---------
  for (int j = 0; j < 9; j++) {
    int wmo = doc["daily"]["weather_code"][j];
    float tmax = doc["daily"]["temperature_2m_max"][j];
    float tmin = doc["daily"]["temperature_2m_min"][j];
    int picId = weatherCodeToPic(wmo, true);

    if (j == 0) {
      nextionSetPic("p[0].icon_jour", picId);
      nextionSetText("p[0].T_min_jour", String(tmin, 0));
      nextionSetText("p[0].T_max_jour", String(tmax, 0));
    } 
    else if (j == 1) {
      nextionSetPic("p[0].icon_dem", picId);   nextionSetPic("p[2].icon_dem", picId);
      nextionSetText("p[0].T_min_dem", String(tmin, 0));  nextionSetText("p[2].T_min_dem", String(tmin, 0));
      nextionSetText("p[0].T_max_dem", String(tmax, 0));  nextionSetText("p[2].T_max_dem", String(tmax, 0));
    } 
    else {
      String pre = String(j + 1) + "j"; // "2j", "3j"...
      int page = (j <= 3) ? 0 : 2;
      nextionSetPic("p[" + String(page) + "].Icon_" + pre, picId);
      nextionSetText("p[" + String(page) + "].T_min_" + pre, String(tmin, 0));
      nextionSetText("p[" + String(page) + "].T_max_" + pre, String(tmax, 0));
      if (j <= 3) {
        nextionSetPic("p[2].Icon_" + pre, picId);
        nextionSetText("p[2].T_min_" + pre, String(tmin, 0));
        nextionSetText("p[2].T_max_" + pre, String(tmax, 0));
      }
    }
  }

  // --------- Prévisions Horaires (Les 8 prochaines heures) ---------
  // Retrouver l'index de l'heure actuelle dans le tableau hourly
  struct tm ti;
  if (getLocalTime(&ti)) {
    // Array open-meteo: hourly[1] est dans 1h, hourly[2] dans 2h, etc..
    // (Open-Meteo renvoie les 168 heures de la semaine, on se base sur ti.tm_hour)
    int startIndex = ti.tm_hour + 1; 

    String tFields[] = {"t9", "t12", "t14", "t51", "t60", "t66", "t71", "t76"};
    for (int h = 0; h < 8; h++) {
      int idx = startIndex + h;
      float temp = doc["hourly"]["temperature_2m"][idx];
      float pluvio = doc["hourly"]["precipitation"][idx];
      int wmoW = doc["hourly"]["weather_code"][idx];
      int h_disp = (ti.tm_hour + 1 + h) % 24;
      
      String strHeure = String(h_disp) + ":00";
      if(strHeure.length() == 4) strHeure = "0" + strHeure;

      nextionSetPic("p[3].icon_h" + String(h), weatherCodeToPic(wmoW, true));
      nextionSetText("p[3].T_h" + String(h), String(temp, 0));
      nextionSetText("p[3].pl_t" + String(h), String(pluvio, 1));
      nextionSetText("p[3]." + tFields[h], strHeure);
    }
  }
}

// ==============================================================================
// 4. GESTION DE L'HEURE (NTP)
// ==============================================================================

void updateTimeOnNextion() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  
  char timeHour[3]; strftime(timeHour, 3, "%H", &timeinfo);
  char timeMin[3];  strftime(timeMin, 3, "%M", &timeinfo);
  
  const char* days[] = {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"};
  const char* months[] = {"Janv", "Fevr", "Mars", "Avri", "Mai", "Juin", "Juil", "Août", "Sept", "Octo", "Nove", "Dece"};

  char fullDate[32];
  sprintf(fullDate, "%s %d %s", days[timeinfo.tm_wday], timeinfo.tm_mday, months[timeinfo.tm_mon]);

  nextionSetText("p[0].heure", String(timeHour));
  nextionSetText("p[0].minute", String(timeMin));
  nextionSetText("p[0].date", String(fullDate));
  nextionSetText("tit_0", String(days[timeinfo.tm_wday]));
}

// ==============================================================================
// 5. INITIALISATION (SETUP) ET BOUCLE (LOOP)
// ==============================================================================

void setup() {
  Serial.begin(115200);
  
  // Init Port Nextion
  NEXTION_SERIAL.begin(115200, SERIAL_8N1, NEXTION_RX, NEXTION_TX);

  // Connexion WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au réseau WiFi.");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());

  // Init Heure
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Premier appel immédiat
  fetchWeatherData();
}

void loop() {
  unsigned long currentMillis = millis();

  // Mise à jour météo toutes les 15 minutes
  if (currentMillis - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL || lastWeatherUpdate == 0) {
    lastWeatherUpdate = currentMillis;
    fetchWeatherData();
  }

  // Mise à jour de l'heure sur l'écran toutes les secondes
  static unsigned long lastTick = 0;
  if(currentMillis - lastTick >= 1000) {
    lastTick = currentMillis;
    updateTimeOnNextion();
  }
}
