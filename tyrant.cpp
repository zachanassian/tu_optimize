#include "tyrant.h"

#include <string>

const std::string faction_names[Faction::num_factions] =
{ "", "bloodthirsty", "imperial", "raider", "righteous", "xeno" };

std::string skill_names[Skill::num_skills] =
{
    // Attack:
    "0",
    // Activation (Including Destroyed):
    "Augment", "Backfire", "Chaos", "Cleanse", "Enfeeble",
    "Freeze", "Heal", "Infuse", "Jam",
    "Mimic", "Protect", "Rally", "Recharge", "Repair", "Rush", "Shock",
    "Siege", "Strike", "Summon", "Supply",
    "temporary_split",
    "trigger_regen",
    "Weaken",
    // Combat-Modifier:
    "AntiAir", "Burst", "Fear", "Flurry", "Pierce", "Swipe", "Valor",
    // Damage-Dependant:
    "Berserk", "Crush", "Disease", "Immobilize", "Leech", "Poison", "Siphon",
    // Defensive:
    "Armored", "Counter", "Emulate", "Evade", "Flying", "Intercept", "Payback", "Refresh", "Regenerate", "Stun", "Tribute", "Wall",
    // Triggered:
    "Blitz", "Legion",
    // Static (Ignored):
    /* "Blizzard", "Fusion", "Mist", */
};

std::set<Skill> helpful_skills{
    augment, cleanse, heal, protect, rally, repair, rush, supply,
};

std::string skill_activation_modifier_names[num_skill_activation_modifiers] = {"", "Play", "Attacked", "Kill", "Death", };

std::string cardtype_names[CardType::num_cardtypes]{"Commander", "Assault", "Structure", "Action", };

std::string rarity_names[5]{"", "common", "uncommon", "rare", "legendary", };

std::string decktype_names[DeckType::num_decktypes]{"Deck", "Mission", "Raid", "Quest", "Custom Deck", };

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

std::string achievement_misc_req_names[AchievementMiscReq::num_achievement_misc_reqs] = {
    "Kill units with skill: flying",
    "Skill activated: (any)",
    "Turns",
    "Damage",
    "Total damage to the enemy Commander"
};
