"""Vérifie le blueprint et sa cohérence avec le firmware ESPHome.

- le blueprint se lit (avec les balises !input de Home Assistant) ;
- chaque !input utilisé est déclaré, et chaque entrée déclarée est utilisée ;
- chaque action esphome.{{ nom }}_<action> appelée existe dans le firmware,
  avec exactement les mêmes variables (le contrat entre les deux fichiers).

Lancer : python tools/verifier_blueprint.py (code 1 si un problème est trouvé).
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

import yaml

RACINE = Path(__file__).resolve().parent.parent
BLUEPRINT = RACINE / "blueprints/automation/nextion_meteo/nextion_meteo.yaml"
FIRMWARE = RACINE / "esphome/nextion-meteo.yaml"


class Entree(str):
    """Valeur d'une balise !input."""


class Secret(str):
    """Valeur d'une balise !secret."""


class Chargeur(yaml.SafeLoader):
    pass


def _entree(chargeur: yaml.SafeLoader, noeud: yaml.Node) -> Entree:
    assert isinstance(noeud, yaml.ScalarNode)
    return Entree(chargeur.construct_scalar(noeud))


def _secret(chargeur: yaml.SafeLoader, noeud: yaml.Node) -> Secret:
    assert isinstance(noeud, yaml.ScalarNode)
    return Secret(chargeur.construct_scalar(noeud))


Chargeur.add_constructor("!input", _entree)
Chargeur.add_constructor("!secret", _secret)


def charger(chemin: Path):
    return yaml.load(chemin.read_text(encoding="utf-8"), Loader=Chargeur)


def entrees_declarees(bloc: dict) -> set[str]:
    noms = set()
    for nom, contenu in bloc.items():
        if isinstance(contenu, dict) and "input" in contenu:  # section
            noms |= entrees_declarees(contenu["input"])
        else:
            noms.add(nom)
    return noms


def parcourir(valeur):
    if isinstance(valeur, dict):
        for k, v in valeur.items():
            yield k, v
            yield from parcourir(v)
    elif isinstance(valeur, list):
        for v in valeur:
            yield None, v
            yield from parcourir(v)


def main() -> int:
    erreurs: list[str] = []
    bp = charger(BLUEPRINT)
    fw = charger(FIRMWARE)

    declarees = entrees_declarees(bp["blueprint"]["input"])
    utilisees = {v for _, v in parcourir(bp) if isinstance(v, Entree)}
    for nom in sorted(utilisees - declarees):
        erreurs.append(f"!input {nom} utilisé mais pas déclaré")
    for nom in sorted(declarees - utilisees):
        erreurs.append(f"entrée {nom} déclarée mais jamais utilisée")

    actions_fw = {
        a["action"]: set((a.get("variables") or {}).keys()) for a in fw["api"]["actions"]
    }
    appels = [
        v for _, v in parcourir(bp) if isinstance(v, dict) and isinstance(v.get("action"), str)
    ]
    vues = set()
    for appel in appels:
        m = re.fullmatch(r"esphome\.\{\{ nom \}\}_(\w+)", appel["action"])
        if not m:
            continue
        nom = m.group(1)
        vues.add(nom)
        if nom not in actions_fw:
            erreurs.append(f"le blueprint appelle {nom}, absente du firmware")
            continue
        envoyees = set((appel.get("data") or {}).keys())
        if envoyees != actions_fw[nom]:
            erreurs.append(
                f"{nom} : le blueprint envoie {sorted(envoyees)}, "
                f"le firmware attend {sorted(actions_fw[nom])}"
            )
    for nom in sorted(set(actions_fw) - vues):
        erreurs.append(f"action {nom} du firmware jamais appelée par le blueprint")

    for e in erreurs:
        print(f"ERREUR : {e}")
    if not erreurs:
        print(f"OK : {len(declarees)} entrées, {len(vues)} actions cohérentes avec le firmware")
    return 1 if erreurs else 0


if __name__ == "__main__":
    sys.exit(main())
