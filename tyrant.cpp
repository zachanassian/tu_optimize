#include "tyrant.h"

#include <string>

const std::string faction_names[Faction::num_factions] =
{ "", "bloodthirsty", "imperial", "raider", "righteous", "xeno" };

std::string skill_names[Skill::num_skills] =
{
    // Activation (including Destroyed):
    "augment", "backfire", "chaos", "cleanse", "enfeeble",
    "freeze", "heal", "infuse", "jam",
    "mimic", "protect", "rally", "recharge", "repair", "rush", "shock",
    "siege", "strike", "summon", "supply",
    "temporary_split",
    "trigger_regen",
    "weaken",
    // Combat-Modifier:
    "antiair", "burst", "fear", "flurry", "pierce", "swipe", "valor",
    // Damage-Dependant:
    "berserk", "crush", "disease", "immobilize", "leech", "poison", "siphon",
    // Defensive:
    "armored", "counter", "emulate", "evade", "flying", "intercept", "payback", "refresh", "regenerate", "tribute", "wall",
    // Triggered:
    "blitz",
    // Static (ignored):
    /* "blizzard", "fusion", "mist", */
    // Misc:
    "0",
};

std::set<Skill> helpful_skills{
    augment, cleanse, heal, protect, rally, repair, rush, supply,
};

std::string cardtype_names[CardType::num_cardtypes]{"Commander", "Assault", "Structure", "Action", };

std::string rarity_names[5]{"", "common", "uncommon", "rare", "legendary", };

std::string effect_names[Effect::num_effects] = {
    "None",
    "Time Surge",
    "Copycat",
    "Quicksilver",
    "Decay",
    "High Skies",
    "Impenetrable",
    "Invigorate",
    "Clone Project",
    "Friendly Fire",
    "Genesis",
    "Artillery Fire",
    "Photon Shield",
    "Decrepit",
    "Forcefield",
    "Chilling Touch",
    "Clone Experiment",
    "Toxic",
    "Haunt",
};
