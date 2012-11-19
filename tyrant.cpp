#include "tyrant.h"

#include <string>

const std::string faction_names[num_factions] =
{ "", "bloodthirsty", "imperial", "raider", "righteous", "xeno" };

std::string skill_names[num_skills] =
{"augment", "augment_all", "chaos", "chaos_all", "cleanse", "cleanse_all", "enfeeble", "enfeeble_all",
 "freeze", "freeze_all", "heal", "heal_all", "infuse", "jam", "jam_all",
 "mimic", "protect", "protect_all", "rally", "rally_all", "rush", "shock",
 "siege", "siege_all", "strike", "strike_all", "summon", "supply",
 "trigger_regen",
 "weaken", "weaken_all"};

std::string cardtype_names[CardType::num_cardtypes]{"action", "assault", "commander", "structure"};
