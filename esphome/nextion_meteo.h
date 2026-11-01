// Nextion Météo — correspondance entre les conditions météo de Home Assistant
// (états de l'entité weather.* : sunny, rainy, …) et les numéros d'image du
// fichier nextion/NextionWeather 3.5.HMI. Une seule table pour l'icône actuelle
// et une pour les prévisions : les deux jeux d'images n'ont pas les mêmes numéros.
#pragma once

#include <cstring>
#include <string>

namespace nextion_meteo {

struct Correspondance {
  const char *condition;
  int actuelle;   // grande icône de la page 0
  int prevision;  // icônes des prévisions (jours et heures)
};

static const Correspondance TABLE[] = {
    {"sunny", 7, 22},          {"Clear", 7, 22},           {"clear-night", 10, 25},
    {"partlycloudy", 5, 20},   {"cloudy", 4, 19},          {"fog", 8, 23},
    {"rainy", 11, 26},         {"pouring", 12, 27},        {"lightning", 16, 31},
    {"lightning-rainy", 9, 24}, {"hail", 13, 28},          {"snowy", 14, 29},
    {"snowy-rainy", 15, 30},   {"windy", 17, 32},
};

inline const Correspondance *chercher(const std::string &condition) {
  for (const auto &c : TABLE) {
    if (condition == c.condition) return &c;
  }
  return nullptr;
}

}  // namespace nextion_meteo

// Image de la grande icône (2 = inconnue)
inline int icone_actuelle(const std::string &condition) {
  const auto *c = nextion_meteo::chercher(condition);
  return c ? c->actuelle : 2;
}

// Image des icônes de prévision (3 = inconnue)
inline int icone_prevision(const std::string &condition) {
  const auto *c = nextion_meteo::chercher(condition);
  return c ? c->prevision : 3;
}
