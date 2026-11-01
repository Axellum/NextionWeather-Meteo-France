# Vidéo : script et plans

Brouillon pour une vidéo courte (~2 min) qui présente le Nextion météo et le compare aux autres écrans. Axel tourne et monte ; ce fichier sert de liste de plans.

## Plans à tourner

| # | Plan | Durée | Image | Ce qu'on dit (voix ou texte à l'écran) |
|---|---|---|---|---|
| 1 | Les trois écrans sur le bureau, plan large, lent travelling | 6 s | 7B à gauche, Nextion devant, Tab5 au fond | « Trois écrans pour Home Assistant, du plus simple au plus complet. » |
| 2 | Gros plan du Nextion 3,5" | 8 s | l'accueil : heure, temps actuel, 4 jours | « Le plus simple : un Nextion de 3,5 pouces, piloté par un petit ESP32. » |
| 3 | Doigt qui balaie / touche : pages 9 jours et heures | 10 s | pages 2 et 3 | « Prévisions sur 9 jours, et heure par heure. » |
| 4 | Gros plan des pictos de vigilance et de la barre de pluie | 6 s | page 0, bas de l'écran | « La vigilance du département et la pluie dans l'heure, de Météo-France. » |
| 5 | Le XIAO et ses 4 fils, à la main | 6 s | dos de l'écran | « Quatre fils : TX, RX, 5 V, masse. » |
| 6 | Capture d'écran : import du blueprint dans HA | 10 s | écran du PC | « Dans Home Assistant, un blueprint : on choisit l'écran et la ville, rien à recompiler. » |
| 7 | Redémarrage : on débranche le XIAO, on rebranche, chrono à l'écran | 12 s | Nextion vide puis rempli | « Au redémarrage, l'écran est rempli en six secondes environ. » |
| 8 | Plan sur la 7B, puis sur le Tab5 | 12 s | tableau de bord complet | « À côté, les écrans ESP32-P4 du projet Tab5 : un vrai tableau de bord tactile. » |
| 9 | Tableau comparatif en surimpression | 15 s | tableau de `comparatif-ecrans.md` | taille, prix, installation, démarrage |
| 10 | Retour plan large | 5 s | les écrans | « Le code est sur GitHub, partagé au cas où. » |

## Conseils de tournage

- Écrans à la même luminosité, pièce sans soleil direct (les dalles résistives du Nextion reflètent).
- Téléphone sur trépied, mise au point verrouillée sur l'écran filmé, exposition baissée d'un cran pour que les écrans ne brûlent pas.
- Plan 7 : filmer en continu, du débranchage jusqu'à l'écran rempli, pour garder le vrai chrono.

## À vérifier avant de publier

- Le prix du Tab5 (non vérifié le 01/11/2026).
