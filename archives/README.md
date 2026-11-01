# Archives

Les anciennes architectures du projet, gardées pour mémoire. Elles ne sont plus
maintenues : pour une nouvelle installation, suivre la version « Service Push »
du [README principal](../README.md) (dossiers `esphome/` et `blueprints/`).

| Dossier | Ce que c'était |
|---|---|
| `V1_HA_ESPHome/` | V1 : capteurs template dans Home Assistant (`configuration.yaml`, `meteo_nextion.yaml`) lus par l'ESP32 un par un. Lente au démarrage et lourde côté HA. |
| `V1_template_sensors/` | Les capteurs template de la V1. |
| `V3_Arduino_OpenMeteo/` | V3 : ESP32 autonome sous Arduino IDE, prévisions Open-Meteo, sans Home Assistant. |
| `V4_Arduino_MeteoFrance/` | V4 : ESP32 autonome sous Arduino IDE, API de l'application Météo-France. Fragile : dépend d'une clé que Météo-France peut changer. |
| `esphome_ancien/` | Les premiers YAML ESPHome (ESP32 AZ-Delivery, écrans 2.8" et 3.5"), sur les broches GPIO1/GPIO3. |
| `NextionWeather 10.1 essai.HMI` | Un essai de mise en page pour un écran 10.1", jamais terminé. |

L'historique complet reste dans git (`git log -- <chemin>`).
