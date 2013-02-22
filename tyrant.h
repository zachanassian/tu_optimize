#ifndef TYRANT_H_INCLUDED
#define TYRANT_H_INCLUDED

#include <string>
#include <set>
#include <tuple>

enum Faction
{
    allfactions,
    bloodthirsty,
    imperial,
    raider,
    righteous,
    xeno,
    num_factions
};
extern const std::string faction_names[num_factions];

enum Skill
{
    // Activation (including Destroyed):
    augment, backfire, chaos, cleanse, enfeeble, freeze, heal, infuse, jam,
    mimic, protect, rally, recharge, repair, rush, shock, siege, strike, summon, supply,
    temporary_split, // not actually a skill; handles Clone Project/Experiment
    trigger_regen, // not actually a skill; handles regeneration after strike/siege
    weaken, 
    // Combat-Modifier:
    antiair, burst, fear, flurry, pierce, swipe, valor,
    // Damage-Dependant:
    berserk, crush, disease, immobilize, leech, poison, siphon,
    // Defensive:
    armored, counter, emulate, evade, flying, intercept, payback, refresh, regenerate, tribute, wall,
    // Triggered:
    blitz, legion,
    // Static, ignored:
    /* blizzard, fusion, mist, */
    // Misc:
    attack,
    num_skills};
extern std::string skill_names[num_skills];
extern std::set<Skill> helpful_skills;

namespace CardType {
enum CardType {
    commander,
    assault,
    structure,
    action,
    num_cardtypes
};
}

extern std::string cardtype_names[CardType::num_cardtypes];

extern std::string rarity_names[5];

namespace DeckType {
enum DeckType {
    deck,
    mission,
    raid,
    quest,
    custom_deck,
    num_decktypes
};
}

extern std::string decktype_names[DeckType::num_decktypes];

enum Effect {
    none,
    time_surge,
    copycat,
    quicksilver,
    decay,
    high_skies,
    impenetrable,
    invigorate,
    clone_project,
    friendly_fire,
    genesis,
    artillery_fire,
    photon_shield,
    decrepit,
    forcefield,
    chilling_touch,
    clone_experiment,
    toxic,
    haunt,
    num_effects
};

extern std::string effect_names[Effect::num_effects];

enum gamemode_t
{
    fight,
    surge,
    tournament
};

struct true_ {};

struct false_ {};

template<unsigned>
struct skillTriggersRegen { typedef false_ T; };

template<>
struct skillTriggersRegen<strike> { typedef true_ T; };

template<>
struct skillTriggersRegen<siege> { typedef true_ T; };

enum SkillSourceType
{
    source_hostile,
    source_allied,
    source_global_hostile,
    source_global_allied,
    source_chaos
};

typedef std::tuple<Skill, unsigned, Faction, bool /* all */> SkillSpec;

#endif
