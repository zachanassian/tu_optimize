#include "tyrant.h"

#include <string>

bool verbose(true);
const std::string faction_names[Faction::num_factions] =
{ "", "bloodthirsty", "imperial", "raider", "righteous", "xeno", "progenitor" };

std::string skill_names[Skill::num_skills] =
{
    // Attack:
    "0",
    // Activation (Including Destroyed):
    "Augment", "Backfire", "Chaos", "Cleanse", "Enfeeble",
    "Enhance Armored", "Enhance Berserk", "Enhance Corrosive", "Enhance Counter", "Enhance Enfeeble", "Enhance Evade",
    "Enhance Leech", "Enhance Heal", "Enhance Poison", "Enhance Rally", "Enhance Strike",
    "Freeze", "Heal", "Infuse", "Jam",
    "Mimic", "Overload", "Protect", "Rally", "Recharge", "Repair", "Rush", "Shock",
    "Siege", "Split", "Strike", "Summon", "Supply",
    "trigger_regen",
    "Weaken",
    // Combat-Modifier:
    "AntiAir", "Burst", "Fear", "Flurry", "Pierce", "Swipe", "Valor",
    // Damage-Dependant:
    "Berserk", "Corrosive", "Crush", "Disease", "Immobilize", "Inhibit", "Leech", "Phase", "Poison", "Siphon", "Sunder",
    // Defensive:
    "Armored", "Counter", "Emulate", "Evade", "Flying", "Intercept", "Payback", "Refresh", "Regenerate", "Stun", "Tribute", "Wall",
    // Triggered:
    "Blitz", "Legion",
    // Static (Ignored):
    /* "Blizzard", "Fusion", "Mist", */
    // Placeholder for new gained skill from battleground effect:
    "<Error>"
};

std::set<Skill> helpful_skills{
    augment, cleanse, heal, protect, rally, repair, rush, supply,
};

std::string skill_activation_modifier_names[SkillMod::num_skill_activation_modifiers] = {"", " on Play", " on Attacked", " on Kill", " on Death", };

std::string cardtype_names[CardType::num_cardtypes]{"Commander", "Assault", "Structure", "Action", };

std::string rarity_names[5]{"", "common", "uncommon", "rare", "legendary", };

std::string decktype_names[DeckType::num_decktypes]{"Deck", "Mission", "Raid", "Quest", "Custom Deck", };

std::string effect_names[Effect::num_effects] = {
    "None",
    "Armor 1",
    "Armor 2",
    "Armor 3",
    "Berserk 1",
    "Berserk 2",
    "Berserk 3",
    "Corrosive 1",
    "Corrosive 2",
    "Corrosive 3",
    "Counter 1",
    "Counter 2",
    "Counter 3",
    "Enfeeble 1",
    "Enfeeble 2",
    "Enfeeble 3",
    "Evade 1",
    "Evade 2",
    "Evade 3",
    "Heal 1",
    "Heal 2",
    "Heal 3",
    "Leech 1",
    "Leech 2",
    "Leech 3",
    "Poison 1",
    "Poison 2",
    "Poison 3",
    "Rally 1",
    "Rally 2",
    "Rally 3",
    "Strike 1",
    "Strike 2",
    "Strike 3",
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
    "Artillery Strike",
    "Photon Shield",
    "Decrepit",
    "Forcefield",
    "Chilling Touch",
    "Clone Experiment",
    "Toxic",
    "Haunt",
    "United Front",
    "Harsh Conditions",
};

std::string achievement_misc_req_names[AchievementMiscReq::num_achievement_misc_reqs] = {
    "Kill units with skill: flying",
    "Skill activated: (any)",
    "Turns",
    "Damage",
    "Total damage to the enemy Commander"
};
