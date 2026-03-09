/*
  Meteo35 - Version V4 (Standalone / Autonome Météo-France)
  Compatible : ESP32 dans l'IDE Arduino.
  
  Description : Se connecte directement au Wi-Fi, récupère l'heure 
  via NTP, lance des requêtes HTTPS vers l'API OFFICIELLE de Météo-France 
  (en utilisant le Token de sécurité public de leur application mobile) 
  et pilote un écran Nextion.

  Cette version restaure les alertes vigilances Françaises et la pluie à 5 minutes 
  tout en se passant de Home Assistant.
  
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
const String DEPARTEMENT = "40"; // Code département pour les vigilances

// Api Météo France Mobile
const String MF_TOKEN = "__Wj7dVSTjV9YGu1guveLyDq0g7S7TfTjaHBTPTpO0kj8__";
const String urlForecast = "https://webservice.meteofrance.com/v1/forecast?lat=" + LATITUDE + "&lon=" + LONGITUDE + "&lang=fr";
const String urlRain     = "https://webservice.meteofrance.com/rain?lat=" + LATITUDE + "&lon=" + LONGITUDE + "&lang=fr";
const String urlWarning  = "https://webservice.meteofrance.com/v3/warning/currentphenomenons?domain=" + DEPARTEMENT + "&depth=0";

// Fréquence de rafraîchissement météo en millisecondes (ex: 15 minutes)
const unsigned long WEATHER_UPDATE_INTERVAL = 15 * 60 * 1000;
unsigned long lastWeatherUpdate = 0;

// Fréquence de rafraîchissement de la Pluie (ex: 5 minutes)
const unsigned long RAIN_UPDATE_INTERVAL = 5 * 60 * 1000;
unsigned long lastRainUpdate = 0;

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

void nextionSendCmd(String cmd) {
  NEXTION_SERIAL.print(cmd);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

void nextionSetText(String component, String text) {
  nextionSendCmd(component + ".txt=\"" + text + "\"");
}

void nextionSetVal(String component, int val) {
  nextionSendCmd(component + ".val=" + String(val));
}

void nextionSetPic(String component, int picId) {
  nextionSendCmd(component + ".pic=" + String(picId));
}

// Convertit l'icone texte de Météo France (ex: "p1j", "p3n") vers l'ID d'image Nextion (0 à 32)
// Basé sur le mapping historique de Home Assistant (Météo France -> Nextion)
int mfIconToPic(String iconCode, bool isMainDay = false) {
  // Codes icones Météo France les plus courants
  // p1 : ensoillé, p2/p3 : nuageux, p4... : couvert, brouillard, p10...: précipitations, p20: neige, p30: orages
  int mfNum = 0;
  sscanf(iconCode.c_str(), "p%dj", &mfNum); // essaye format jour
  if(mfNum == 0) sscanf(iconCode.c_str(), "p%dn", &mfNum); // format nuit
  if(mfNum == 0) sscanf(iconCode.c_str(), "p%d", &mfNum); // format sans jour/nuit suffixé

  bool isNight = iconCode.endsWith("n");

  if(isMainDay) {
    // Mode Icone du jour principal
    switch (mfNum) {
      case 1: return isNight ? 10 : 7; // Clair/Sunny
      case 2: case 3: return 5; // partlycloudy
      case 4: case 5: return 4; // cloudy
      case 6: case 7: case 8: case 9: return 8; // fog
      case 10: case 11: case 12: case 13: case 29: case 34: case 42: return 11; // rainy
      case 14: case 15: case 16: case 17: case 44: case 48: case 49: return 12; // pouring
      case 18: case 19: case 20: case 21: case 22: case 23: case 45: case 46: case 47: return 14; // snowy
      case 24: case 25: case 26: case 27: case 28: case 35: case 36: case 37: case 38: return 16; // lightning
      case 31: case 32: case 33: case 39: case 40: case 41: case 50: return 9; // lightning-rainy
      default: return 2; // Inconnu
    }
  } else {
    // Mode Icones horaires / miniatures / jours suivants
    switch (mfNum) {
      case 1: return isNight ? 25 : 22; // Clair/Sunny
      case 2: case 3: return 20; // partlycloudy
      case 4: case 5: return 19; // cloudy
      case 6: case 7: case 8: case 9: return 23; // fog
      case 10: case 11: case 12: case 13: case 29: case 34: case 42: return 26; // rainy
      case 14: case 15: case 16: case 17: case 44: case 48: case 49: return 27; // pouring
      case 18: case 19: case 20: case 21: case 22: case 23: case 45: case 46: case 47: return 29; // snowy
      case 24: case 25: case 26: case 27: case 28: case 35: case 36: case 37: case 38: return 31; // lightning
      case 31: case 32: case 33: case 39: case 40: case 41: case 50: return 24; // lightning-rainy
      default: return 3; // Inconnu
    }
  }
}

// Convertit le code couleur Météo France (1=Vert, 2=Jaune, 3=Orange, 4=Rouge) à un pic Id
int mfColorToPic(int mfColorId, int idVert) {
  if (mfColorId == 2) return idVert + 1; // Jaune
  if (mfColorId == 3) return idVert + 2; // Orange
  if (mfColorId == 4) return idVert + 3; // Rouge
  return idVert; // Vert par défaut
}

// ==============================================================================
// 3. RECUPERATION ET TRAITEMENT DES DONNEES METEO FRANCE
// ==============================================================================

String fetchUrl(String url) {
  HTTPClient http;
  http.begin(url);
  http.addHeader("Authorization", "Bearer " + MF_TOKEN);
  int httpCode = http.GET();
  String payload = "";
  if (httpCode == 200) {
    payload = http.getString();
  } else {
    Serial.printf("Erreur HTTP %d sur URL: %s\n", httpCode, url.c_str());
  }
  http.end();
  return payload;
}

void fetchAllWeatherData() {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.println("Recherche Météo Générale (V1)...");

  // 1. Prévisions Jours / Heures (Forecast)
  String payloadForecast = fetchUrl(urlForecast);
  if (payloadForecast.length() > 0) {
    // Utiliser ArduinoJson avec filtrage pour ne pas saturer la RAM !
    DynamicJsonDocument filter(512);
    filter["current"]["T"]["value"] = true;
    filter["current"]["humidity"] = true;
    filter["current"]["weather"]["icon"] = true;
    
    filter["probability_forecast"][0]["rain"]["snow"] = true; // Pour UV etc on prend global
    filter["daily_forecast"][0]["T"]["min"] = true;
    filter["daily_forecast"][0]["T"]["max"] = true;
    filter["daily_forecast"][0]["weather12H"]["icon"] = true;
    filter["daily_forecast"][0]["uv"] = true;

    // Récupère les 8 prochaines heures et 9 prochains jours.
    DynamicJsonDocument doc(12288);
    DeserializationError error = deserializeJson(doc, payloadForecast, DeserializationOption::Filter(filter));
    
    if (!error) {
       updateForecastNextion(doc);
    } else {
       // Si OOM (Plus de mémoire), réessayer sans filtre mais avec un doc plus grand, 
       // ou filtrer plus agressivement. Ici on simplifie:
       DynamicJsonDocument doc2(32768);
       deserializeJson(doc2, payloadForecast);
       updateForecastNextion(doc2);
    }
  }

  // 2. Alertes (Warning)
  String payloadWarning = fetchUrl(urlWarning);
  if (payloadWarning.length() > 0) {
    DynamicJsonDocument docW(2048);
    if (!deserializeJson(docW, payloadWarning)) {
      updateWarningsNextion(docW);
    }
  }

  // 3. Radar Pluie dans l'heure (Rain)
  fetchRainRadar();
}

void fetchRainRadar() {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.println("Recherche Radar Pluie 5min...");

  String payloadRain = fetchUrl(urlRain);
  if (payloadRain.length() > 0) {
    DynamicJsonDocument docR(2048);
    if (!deserializeJson(docR, payloadRain)) {
      updateRainNextion(docR);
    }
  }
}

// ---------------- Traitement JSON vers Nextion ----------------

void updateForecastNextion(DynamicJsonDocument& doc) {
  // --------- Actuel ---------
  if(doc["current"].containsKey("T")) {
    nextionSetVal("p[0].temp_1", (int)doc["current"]["T"]["value"]);
    nextionSetVal("p[0].hum_1", doc["current"]["humidity"]);
    String iconStr = doc["current"]["weather"]["icon"];
    nextionSetPic("p[0].icon", mfIconToPic(iconStr, true));
  }

  // --------- Proba Pluie et UV ---------
  // Météo France peut délivrer proba pluie ou on le prend sur probability_forecast
  int uv = doc["daily_forecast"][0]["uv"];
  nextionSetPic("p[0].uv", (uv < 5) ? 85 : (uv < 7) ? 84 : (uv < 9) ? 83 : (uv < 11) ? 82 : 81);

  // Météo France a des probas de gel/neige/pluie dans probability_forecast
  // (Par simplification, ici on récupère celles du jour ou on fixe par défaut si absentes)
  JsonArray probas = doc["probability_forecast"];
  if(probas.size() > 0) {
      int pRain = probas[0]["rain"]["3h"]; // Exemple 3h proba pluie
      int pSnow = probas[0]["snow"]["3h"];
      int pFreez= probas[0]["freezing"];
      
      nextionSetPic("p[0].alerte_rain", (pRain < 9) ? 80 : (pRain < 40) ? 79 : (pRain < 70) ? 78 : 77);
      nextionSetPic("p[0].neige", (pSnow < 5) ? 87 : (pSnow < 50) ? 88 : (pSnow < 90) ? 89 : (pSnow <= 100) ? 90 : 95);
      nextionSetPic("p[0].gel",   (pFreez < 5) ? 91 : (pFreez < 50) ? 92 : (pFreez < 90) ? 93 : (pFreez <= 100) ? 94 : 95);
  }

  // --------- Jours (Daily) ---------
  JsonArray daily = doc["daily_forecast"];
  for (int j = 0; j < 9 && j < daily.size(); j++) {
    float tmin = daily[j]["T"]["min"];
    float tmax = daily[j]["T"]["max"];
    String iconStr = daily[j]["weather12H"]["icon"];
    int picId = mfIconToPic(iconStr, false);

    if (j == 0) {
      nextionSetPic("p[0].icon_jour", mfIconToPic(iconStr, true));
      nextionSetText("p[0].T_min_jour", String(tmin, 0));
      nextionSetText("p[0].T_max_jour", String(tmax, 0));
    } 
    else if (j == 1) {
      nextionSetPic("p[0].icon_dem", picId);   nextionSetPic("p[2].icon_dem", picId);
      nextionSetText("p[0].T_min_dem", String(tmin, 0));  nextionSetText("p[2].T_min_dem", String(tmin, 0));
      nextionSetText("p[0].T_max_dem", String(tmax, 0));  nextionSetText("p[2].T_max_dem", String(tmax, 0));
    } 
    else {
      String pre = String(j + 1) + "j";
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

  // --------- Heures (Hourly) ---------
  JsonArray hourly = doc["forecast"];
  String tFields[] = {"t9", "t12", "t14", "t51", "t60", "t66", "t71", "t76"};
  // Météo France donne les heures (par heure). On saute index 0 (heure actuelle) -> on prend index 1 à 8.
  for (int h = 0; h < 8 && ((h + 1) < hourly.size()); h++) {
    int idx = h + 1;
    long timestamp = hourly[idx]["dt"];
    float temp = hourly[idx]["T"]["value"];
    String iconStr = hourly[idx]["weather"]["icon"];
    float pluvio = hourly[idx]["rain"].containsKey("1h") ? (float)hourly[idx]["rain"]["1h"] : 0.0;

    // Convertir Timestamp local ESP en Date
    time_t rawtime = (time_t)timestamp;
    struct tm * ti = localtime(&rawtime);
    String strHeure = String(ti->tm_hour) + ":00";
    if(strHeure.length() == 4) strHeure = "0" + strHeure;

    nextionSetPic("p[3].icon_h" + String(h), mfIconToPic(iconStr, false));
    nextionSetText("p[3].T_h" + String(h), String(temp, 0));
    nextionSetText("p[3].pl_t" + String(h), String(pluvio, 1));
    nextionSetText("p[3]." + tFields[h], strHeure);
  }
}

void updateWarningsNextion(DynamicJsonDocument& doc) {
  // Couleur globale (Vert:35, Jaune:36, Orange:37, Rouge:38)
  int globColor = doc["color_max"]; // API Météo France Warning V3: color_max = 1 (Vert), 2 (Jaune)...
  int globPic = 34; // Défaut gris
  if(globColor == 1) globPic = 35;
  if(globColor == 2) globPic = 36;
  if(globColor == 3) globPic = 37;
  if(globColor == 4) globPic = 38;
  nextionSetVal("p[0].alert_coul", globPic);

  // Valeurs par défaut Vertical (Vert)
  int v_vent = 47, v_pluie = 71, v_orage = 59, v_inondation = 55, v_neige = 67, v_canicule = 51, v_vague = 63;

  JsonArray phenomenons = doc["phenomenons_max_colors"];
  for(JsonObject phenomenon : phenomenons) {
    int idType = phenomenon["phenomenon_id"];
    int color  = phenomenon["phenomenon_max_color_id"];
    
    // Mappage IDs Officiels Météo France
    // 1: Vent, 2: Pluie/Inondations, 3: Orages, 4: Inondations, 6: Neige/Verglas, 7: Canicule, 8: Grand Froid, 9: Avalanches, 10: Vagues/Submersions
    if(idType == 1) v_vent = mfColorToPic(color, 47);
    if(idType == 2) v_pluie = mfColorToPic(color, 71);
    if(idType == 3) v_orage = mfColorToPic(color, 59);
    if(idType == 4) v_inondation = mfColorToPic(color, 55);
    if(idType == 6) v_neige = mfColorToPic(color, 67);
    if(idType == 7 || idType == 8) v_canicule = mfColorToPic(color, 51); // Canicule ou Froid (Bouton partagé ?)
    if(idType == 10) v_vague = mfColorToPic(color, 63);
  }

  nextionSetPic("p[0].alert_vent", v_vent);
  nextionSetPic("p[0].alert_pluie_in", v_pluie);
  nextionSetPic("p[0].alert_orages", v_orage);
  nextionSetPic("p[0].alert_inondat", v_inondation);
  nextionSetPic("p[0].alert_neig_ver", v_neige);
  nextionSetPic("p[0].alert_froid", v_canicule);
  nextionSetPic("p[0].alert_vagues", v_vague);
}

void updateRainNextion(DynamicJsonDocument& doc) {
  // Parsing Radar 5 min de MF (v1/vision/rain ou /rain) -> Array 'forecast'
  if(!doc.containsKey("forecast")) return;

  JsonArray forecasts = doc["forecast"];
  
  // Météo France livre le texte desc : 'Temps sec', 'Pluie faible', 'Pluie modérée', 'Pluie forte'.
  int minutesProchaine = 0;
  bool aTrouvePluie = false;

  for (int i = 0; i < 12 && i < forecasts.size(); i++) {
    int mfColor = forecasts[i]["color"]; // 1=sec, 2=faible, 3=moderee, 4=forte
    String desc = forecasts[i]["desc"];

    int pic = 42; // Inconnu
    if (mfColor == 1 || desc == "Temps sec") pic = 42; // En theorie 42 c'est temps sec chez toi
    else if (mfColor == 2 || desc == "Pluie faible") pic = 43;
    else if (mfColor == 3 || desc == "Pluie modérée") pic = 44;
    else if (mfColor == 4 || desc == "Pluie forte") pic = 45;
    
    // Si pluie trouvée, calculer la différence en minutes pour la Pluie dans le texte Nextion
    if(mfColor > 1 && !aTrouvePluie) {
        long dtNow = time(NULL);
        long dtRain = forecasts[i]["dt"];
        minutesProchaine = max(0, (int)((dtRain - dtNow) / 60));
        aTrouvePluie = true;
    }

    nextionSetPic("p[0].mn_" + String(i * 5), pic);
  }

  nextionSetText("p[0].proc_plu", String(minutesProchaine));
}

// ==============================================================================
// 4. GESTION DE L'HEURE (NTP)
// ==============================================================================

void updateTimeOnNextion() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
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
  NEXTION_SERIAL.begin(115200, SERIAL_8N1, NEXTION_RX, NEXTION_TX);

  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" OK");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  fetchAllWeatherData(); // Initial
}

void loop() {
  unsigned long currentMillis = millis();

  // Mise à jour Pluie toutes les 5 mins
  if (currentMillis - lastRainUpdate >= RAIN_UPDATE_INTERVAL) {
    lastRainUpdate = currentMillis;
    fetchRainRadar();
  }

  // Mise à jour Générale Météo toutes les 15 mins
  if (currentMillis - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
    lastWeatherUpdate = currentMillis;
    fetchAllWeatherData();
  }

  // Heure (Toutes les sec)
  static unsigned long lastTick = 0;
  if(currentMillis - lastTick >= 1000) {
    lastTick = currentMillis;
    updateTimeOnNextion();
  }
}
