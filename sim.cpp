#include "sim.h"

#include <boost/range/adaptors.hpp>
#include <random>
#include <string>
#include <sstream>
#include <vector>

#include "card.h"
#include "cards.h"
#include "deck.h"

//---------------------- $00 general stuff -------------------------------------
template<typename T>
std::string to_string(T val)
{
    std::stringstream s;
    s << val;
    return s.str();
}
//---------------------- Debugging stuff ---------------------------------------
bool debug_print(false);
bool debug_line(false);
#ifndef NDEBUG
#define _DEBUG_MSG(format, args...)                                     \
    {                                                                   \
        if(debug_print)                                                 \
        {                                                               \
            char* format2 = new char[strlen(format) + (8 + 1)*sizeof(char)]; \
            if(debug_line) { strcpy(format2, "%i - "); }                \
            else { strcpy(format2, ""); }                               \
            strcat(format2, format);                                    \
            if(debug_line) { printf(format2, __LINE__ , ##args); }      \
            else { printf(format2, ##args); }                           \
            delete[] format2;                                           \
            std::cout << std::flush;                                    \
        }                                                               \
    }
#else
#define _DEBUG_MSG(format, args...)
#endif
//------------------------------------------------------------------------------
CardStatus::CardStatus(const Card* card) :
    m_card(card),
    m_index(0),
    m_player(0),
    m_augmented(0),
    m_berserk(0),
    blitz(false),
    m_chaos(false),
    m_delay(card->m_delay),
    m_diseased(false),
    m_enfeebled(0),
    m_faction(card->m_faction),
    m_frozen(false),
    m_hp(card->m_health),
    m_immobilized(false),
    m_infused(false),
    m_jammed(false),
    m_poisoned(0),
    m_protected(0),
    m_rallied(0),
    m_weakened(0),
    m_temporary_split(false)
{
}
//------------------------------------------------------------------------------
inline void CardStatus::set(const Card* card)
{
    this->set(*card);
}
//------------------------------------------------------------------------------
inline void CardStatus::set(const Card& card)
{
    m_card = &card;
    m_index = 0;
    m_player = 0;
    m_augmented = 0;
    m_berserk = 0;
    blitz = false;
    m_chaos = false;
    m_delay = card.m_delay;
    m_diseased = false;
    m_enfeebled = 0;
    m_faction = card.m_faction;
    m_frozen = false;
    m_hp = card.m_health;
    m_immobilized = false;
    infused_skills.clear();
    m_infused = false;
    m_jammed = false;
    m_poisoned = 0;
    m_protected = 0;
    m_rallied = 0;
    m_weakened = 0;
}
//------------------------------------------------------------------------------
std::string skill_description(const SkillSpec& s)
{
    return(skill_names[std::get<0>(s)] +
           (std::get<2>(s) == allfactions ? "" : std::string(" ") + faction_names[std::get<2>(s)]) +
           (std::get<1>(s) == 0 ? "" : std::string(" ") + to_string(std::get<1>(s))));
}
//------------------------------------------------------------------------------
std::string status_description(CardStatus* status)
{
    assert(status);
    std::string desc;
    switch(status->m_card->m_type)
    {
    case CardType::commander: desc = "Commander "; break;
    case CardType::action: desc = "Action "; break;
    case CardType::assault: desc = "A " + to_string(status->m_index) + " "; break;
    case CardType::structure: desc = "S " + to_string(status->m_index) + " "; break;
    case CardType::num_cardtypes: assert(false); break;
    }
    desc += "[" + status->m_card->m_name + "]";
    return(desc);
}
//------------------------------------------------------------------------------
void Hand::reset(std::mt19937& re)
{
    assaults.reset();
    structures.reset();
    commander = CardStatus(deck->get_commander());
    deck->shuffle(re);
}
//---------------------- $40 Game rules implementation -------------------------
// Everything about how a battle plays out, except the following:
// the implementation of the attack by an assault card is in the next section;
// the implementation of the active skills is in the section after that.
unsigned turn_limit{50};
//------------------------------------------------------------------------------
inline unsigned opponent(unsigned player)
{
    return((player + 1) % 2);
}
//------------------------------------------------------------------------------
void prepend_on_death(Field* fd)
{
    for(auto status: boost::adaptors::reverse(fd->killed_with_on_death))
    {
        for(auto& skill: boost::adaptors::reverse(status->m_card->m_skills_died))
        {
            _DEBUG_MSG("On death skill pushed in front %s %u %s\n", skill_names[std::get<0>(skill)].c_str(), std::get<1>(skill), faction_names[std::get<2>(skill)].c_str());
            fd->skill_queue.emplace_front(status, skill);
        }
    }
    fd->killed_with_on_death.clear();
}
//------------------------------------------------------------------------------
void(*skill_table[num_skills])(Field*, CardStatus* src_status, const SkillSpec&);
void resolve_skill(Field* fd)
{
    while(!fd->skill_queue.empty())
    {
        auto skill_instance(fd->skill_queue.front());
        auto& status(std::get<0>(skill_instance));
        auto& skill(std::get<1>(skill_instance));
        fd->skill_queue.pop_front();
        skill_table[std::get<0>(skill)](fd, status, skill);
    }
}
//------------------------------------------------------------------------------
void attack_phase(Field* fd);
SkillSpec augmented_skill(CardStatus* status, const SkillSpec& s)
{
    SkillSpec augmented_s = s;
    if(std::get<0>(s) != augment && std::get<0>(s) != augment_all && std::get<0>(s) != summon)
    {
        std::get<1>(augmented_s) += status->m_augmented;
    }
    return(augmented_s);
}
SkillSpec fusioned_skill(const SkillSpec& s)
{
    SkillSpec fusioned_s = s;
    std::get<1>(fusioned_s) *= 2;
    return(fusioned_s);
}
void evaluate_skills(Field* fd, CardStatus* status, const std::vector<SkillSpec>& skills)
{
    assert(status);
    assert(fd->skill_queue.size() == 0);
    for(auto& skill: skills)
    {
        // Assumptions for fusion: Only active player can use it, and it is incompatible with augment.
        // This is fine for now since it only exists on the three Blightbloom structures (can't augment structures)
        _DEBUG_MSG("Evaluating %s skill %s\n", status_description(status).c_str(), skill_description(skill).c_str());
        bool fusion_active = status->m_card->m_fusion && status->m_player == fd->tapi && fd->fusion_count >= 3;
        fd->skill_queue.emplace_back(status, fusion_active ? fusioned_skill(skill) : (status->m_augmented == 0 ? skill : augmented_skill(status, skill)));
        resolve_skill(fd);
    }
}
struct PlayCard
{
    const Card* card;
    Field* fd;
    CardStatus* status;
    Storage<CardStatus>* storage;

    PlayCard(const Card* card_, Field* fd_) :
        card{card_},
        fd{fd_},
        status{nullptr},
        storage{nullptr}
    {}

    template <enum CardType::CardType type>
    bool op()
    {
        setStorage<type>();
        placeCard<type>();
        onPlaySkills<type>();
        blitz<type>();
        fieldEffects<type>();
        return(true);
    }

    // action
    template <enum CardType::CardType>
    void setStorage()
    {
    }

    // assault + structure
    template <enum CardType::CardType type>
    void placeCard()
    {
        status = &storage->add_back();
        status->set(card);
        status->m_index = storage->size() - 1;
        status->m_player = fd->tapi;
        if(fd->turn == 1 && fd->gamemode == tournament && status->m_delay > 0)
        {
            ++status->m_delay;
        }
        placeDebugMsg<type>();
    }

    // assault + structure
    template <enum CardType::CardType type>
    void placeDebugMsg()
    {
        _DEBUG_MSG("Placed [%s] as %s %d\n", card->m_name.c_str(), cardtype_names[type].c_str(), storage->size() - 1);
    }

    // all except assault: noop
    template <enum CardType::CardType>
    void blitz()
    {
    }

    // all except assault: noop
    template <enum CardType::CardType>
    void fieldEffects()
    {
    }

    // assault + structure
    template <enum CardType::CardType>
    void onPlaySkills()
    {
        for(auto& skill: card->m_skills_played)
        {
            fd->skill_queue.emplace_back(status, skill);
            resolve_skill(fd);
        }
    }
};
// assault
template <>
void PlayCard::setStorage<CardType::assault>()
{
    storage = &fd->tap->assaults;
}
// structure
template <>
void PlayCard::setStorage<CardType::structure>()
{
    storage = &fd->tap->structures;
}
// action
template <>
void PlayCard::placeCard<CardType::action>()
{
}
// assault
template <>
void PlayCard::blitz<CardType::assault>()
{
    if(card->m_blitz && fd->tip->assaults.size() > status->m_index && fd->tip->assaults[status->m_index].m_hp > 0 && fd->tip->assaults[status->m_index].m_delay == 0)
    {
        status->blitz = true;
    }
}
// assault
template <>
void PlayCard::fieldEffects<CardType::assault>()
{
    if(fd->effect == Effect::toxic)
    {
        status->m_poisoned = 1;
    }
    else if(fd->effect == Effect::decay)
    {
        status->m_poisoned = 1;
        status->m_diseased = true;
    }
}
// action
template <>
void PlayCard::onPlaySkills<CardType::action>()
{
    for(auto& skill: card->m_skills)
    {
        fd->skill_queue.emplace_back(nullptr, skill);
        resolve_skill(fd);
        // Special case: enemy commander killed by a shock action card
        if(fd->tip->commander.m_hp == 0)
        {
            _DEBUG_MSG("turn's defender dead.\n");
            fd->end = true;
            break;
        }
    }
    // Special case: recharge ability
    if(card->m_recharge && fd->flip())
    {
        fd->tap->deck->place_at_bottom(card);
    }
}
//------------------------------------------------------------------------------
void turn_start_phase(Field* fd);
void prepend_on_death(Field* fd);
void summon_card(Field* fd, unsigned player, const Card* summoned);
// return value : 0 -> attacker wins, 1 -> defender wins
unsigned play(Field* fd)
{
    fd->players[0]->commander.m_player = 0;
    fd->players[1]->commander.m_player = 1;
    fd->tapi = fd->gamemode == surge ? 1 : 0;
    fd->tipi = opponent(fd->tapi);
    fd->tap = fd->players[fd->tapi];
    fd->tip = fd->players[fd->tipi];
    fd->fusion_count = 0;
    fd->end = false;
    // Shuffle deck
    while(fd->turn < turn_limit && !fd->end)
    {
        fd->current_phase = Field::playcard_phase;
        // Initialize stuff, remove dead cards
        _DEBUG_MSG("##### TURN %u #####\n", fd->turn);
        turn_start_phase(fd);
        // Special case: refresh on commander
        if(fd->tip->commander.m_card->m_refresh && fd->tip->commander.m_hp > 0)
        {
            fd->tip->commander.m_hp = fd->tip->commander.m_card->m_health;
        }

        if(fd->effect == Effect::clone_project ||
           (fd->effect == Effect::clone_experiment && (fd->turn == 9 || fd->turn == 10)))
        {
            std::vector<SkillSpec> skills;
            skills.emplace_back(temporary_split, 0, allfactions);
            // The skill doesn't actually come from the commander,
            // but we need to provide some source and it seemed most reasonable.
            evaluate_skills(fd, &fd->tap->commander, skills);
        }

        // Play a card
        const Card* played_card(fd->tap->deck->next());
        if(played_card)
        {
            switch(played_card->m_type)
            {
            case CardType::action:
                // end: handles commander death by shock
                PlayCard(played_card, fd).op<CardType::action>();
                break;
            case CardType::assault:
                PlayCard(played_card, fd).op<CardType::assault>();
                break;
            case CardType::structure:
                PlayCard(played_card, fd).op<CardType::structure>();
                break;
            case CardType::commander:
            case CardType::num_cardtypes:
                assert(false);
                break;
            }
        }
        // Evaluate commander
        fd->current_phase = Field::commander_phase;
        evaluate_skills(fd, &fd->tap->commander, fd->tap->commander.m_card->m_skills);

        if (fd->effect == Effect::genesis)
        {
            unsigned index(fd->rand(0, fd->cards.player_assaults.size() - 1));
            Card* summonee(fd->cards.player_assaults[index]);
            summon_card(fd, fd->tapi, summonee);
        }

        // Evaluate structures
        fd->current_phase = Field::structures_phase;
        for(fd->current_ci = 0; !fd->end && fd->current_ci < fd->tap->structures.size(); ++fd->current_ci)
        {
            CardStatus& current_status(fd->tap->structures[fd->current_ci]);
            if(current_status.m_delay == 0)
            {
                evaluate_skills(fd, &current_status, current_status.m_card->m_skills);
            }
        }
        // Evaluate assaults
        fd->current_phase = Field::assaults_phase;
        for(fd->current_ci = 0; !fd->end && fd->current_ci < fd->tap->assaults.size(); ++fd->current_ci)
        {
            // ca: current assault
            CardStatus& current_status(fd->tap->assaults[fd->current_ci]);
            if((current_status.m_delay > 0 && !current_status.blitz) || current_status.m_hp == 0 || current_status.m_jammed || current_status.m_frozen)
            {
                //_DEBUG_MSG("! Assault %u (%s) hp: %u, jammed %u\n", card_index, current_status.m_card->m_name.c_str(), current_status.m_hp, current_status.m_jammed);
            }
            else
            {
                // Special case: check for split (tartarus swarm raid, or clone battlefield effects)
                if((current_status.m_temporary_split || current_status.m_card->m_split) && fd->tap->assaults.size() + fd->tap->structures.size() < 100)
                {
                    CardStatus& status_split(fd->tap->assaults.add_back());
                    status_split.set(current_status.m_card);
                    status_split.m_index = fd->tap->assaults.size() - 1;
                    status_split.m_player = fd->tapi;
                    _DEBUG_MSG("Split assault %d (%s)\n", fd->tap->assaults.size() - 1, current_status.m_card->m_name.c_str());
                    for(auto& skill: status_split.m_card->m_skills_played)
                    {
                        fd->skill_queue.emplace_back(&status_split, skill);
                        resolve_skill(fd);
                    }
                    // TODO: Determine whether we need to check for Blitz for the newly-Split unit
                }
                // Evaluate skills
                // Special case: Gore Typhon's infuse
                evaluate_skills(fd, &current_status, current_status.m_infused ? current_status.infused_skills : current_status.m_card->m_skills);
                // Attack
                if(!current_status.m_immobilized && current_status.m_hp > 0)
                {
                    attack_phase(fd);
                }
            }
        }
        std::swap(fd->tapi, fd->tipi);
        std::swap(fd->tap, fd->tip);
        ++fd->turn;
    }
    // defender wins
    if(fd->players[0]->commander.m_hp == 0) { _DEBUG_MSG("Defender wins.\n"); return(1); }
    // attacker wins
    if(fd->players[1]->commander.m_hp == 0) { _DEBUG_MSG("Attacker wins.\n"); return(0); }
    if(fd->turn >= turn_limit) { return(1); }

    // Huh? How did we get here?
    assert(false);
    return 0;
}
//------------------------------------------------------------------------------
// All the stuff that happens at the beginning of a turn, before a card is played
inline unsigned safe_minus(unsigned x, unsigned y)
{
    return(x - std::min(x, y));
}
// returns true iff the card died.
bool remove_hp(Field* fd, CardStatus& status, unsigned dmg)
{
    assert(status.m_hp > 0);
    status.m_hp = safe_minus(status.m_hp, dmg);
    const bool just_died(status.m_hp == 0);
    if(just_died)
    {
        _DEBUG_MSG("Card %u (%s) dead\n", status.m_index, status.m_card->m_name.c_str());
        if(status.m_card->m_skills_died.size() > 0)
        {
            fd->killed_with_on_death.push_back(&status);
        }
        if(status.m_card->m_regenerate)
        {
            fd->killed_with_regen.push_back(&status);
        }
    }
    return(just_died);
}
inline bool is_it_dead(CardStatus& c)
{
    if(c.m_hp == 0) // yes it is
    {
        _DEBUG_MSG("Dead: %s\n", status_description(&c).c_str());
        return(true);
    }
    else { return(false); } // nope still kickin'
}
inline void remove_dead(Storage<CardStatus>& storage)
{
    storage.remove(is_it_dead);
}
inline void add_hp(Field* field, CardStatus* target, unsigned v)
{
    unsigned old_hp = target->m_hp;
    target->m_hp = std::min(target->m_hp + v, target->m_card->m_health);
    if(field->effect == Effect::invigorate && target->m_card->m_type == CardType::assault)
    {
        unsigned healed = target->m_hp - old_hp;
        target->m_berserk += healed;
    }
}
void check_regeneration(Field* fd)
{
    for(unsigned i(0); i < fd->killed_with_regen.size(); ++i)
    {
        CardStatus& status = *fd->killed_with_regen[i];
        if(status.m_hp == 0 && status.m_card->m_regenerate > 0 && !status.m_diseased)
        {
            if (fd->flip())
            {
                add_hp(fd, &status, status.m_card->m_regenerate);
            }
        }
        if(status.m_hp > 0)
        {
            _DEBUG_MSG("Card %s regenerated, hp 0 -> %u\n", status.m_card->m_name.c_str(), status.m_hp);
        }

    }
    fd->killed_with_regen.clear();
}
void turn_start_phase(Field* fd)
{
    remove_dead(fd->tap->assaults);
    remove_dead(fd->tap->structures);
    remove_dead(fd->tip->assaults);
    remove_dead(fd->tip->structures);
    fd->fusion_count = 0;
    // Active player's assault cards:
    // update index
    // remove enfeeble, protect; apply poison damage, reduce delay
    {
        auto& assaults(fd->tap->assaults);
        for(unsigned index(0), end(assaults.size());
            index < end;
            ++index)
        {
            CardStatus& status(assaults[index]);
            status.m_index = index;
            status.m_enfeebled = 0;
            status.m_protected = 0;
            remove_hp(fd, status, status.m_poisoned);
            if(status.m_delay > 0 && !status.m_frozen) { --status.m_delay; }
            if(status.m_card->m_fusion && status.m_delay == 0) { ++fd->fusion_count; }
        }
    }
    // Active player's structure cards:
    // update index
    // reduce delay
    {
        auto& structures(fd->tap->structures);
        for(unsigned index(0), end(structures.size());
            index < end;
            ++index)
        {
            CardStatus& status(structures[index]);
            status.m_index = index;
            if(status.m_delay > 0) { --status.m_delay; }
            if(status.m_card->m_fusion && status.m_delay == 0) { ++fd->fusion_count; }
        }
    }
    // Defending player's assault cards:
    // update index
    // remove augment, chaos, freeze, immobilize, jam, rally, weaken, apply refresh
    // remove temp split
    {
        auto& assaults(fd->tip->assaults);
        for(unsigned index(0), end(assaults.size());
            index < end;
            ++index)
        {
            CardStatus& status(assaults[index]);
            status.m_index = index;
            status.m_augmented = 0;
            status.blitz = false;
            status.m_chaos = false;
            status.m_frozen = false;
            status.m_immobilized = false;
            status.m_jammed = false;
            status.m_rallied = 0;
            status.m_weakened = 0;
            status.m_temporary_split = false;
            if(status.m_card->m_refresh && !status.m_diseased)
            {
#ifndef NDEBUG
                if(status.m_hp < status.m_card->m_health)
                {
                    _DEBUG_MSG("%u %s refreshed. hp %u -> %u.\n", index, status_description(&status).c_str(), status.m_hp, status.m_card->m_health);
                }
#endif
                add_hp(fd, &status, status.m_card->m_health);
            }
        }
    }
    // Defending player's structure cards:
    // update index
    // apply refresh
    {
        auto& structures(fd->tip->structures);
        for(unsigned index(0), end(structures.size());
            index < end;
            ++index)
        {
            CardStatus& status(structures[index]);
            status.m_index = index;
            if(status.m_card->m_refresh && status.m_hp < status.m_card->m_health)
            {
#ifndef NDEBUG
                _DEBUG_MSG("%s refreshed. hp %u -> %u.\n", index, status_description(&status).c_str(), status.m_hp, status.m_card->m_health);
#endif
                add_hp(fd, &status, status.m_card->m_health);
            }
        }
    }
    // Perform on death skills (from cards killed by poison damage)
    prepend_on_death(fd);
    resolve_skill(fd);
    // Regen from poison
    check_regeneration(fd);
}
//---------------------- $50 attack by assault card implementation -------------
inline void apply_poison(CardStatus* target, unsigned v)
{
    target->m_poisoned = std::max(target->m_poisoned, v);
}
inline int attack_power(CardStatus* att)
{
    return(safe_minus(att->m_card->m_attack + att->m_berserk + att->m_rallied, att->m_weakened));
}
// Counter damage dealt to the attacker (att) by defender (def)
// pre-condition: only valid if m_card->m_counter > 0
inline unsigned counter_damage(CardStatus* att, CardStatus* def)
{
    assert(att->m_card->m_type == CardType::assault);
    assert(def->m_card->m_type != CardType::action);
    return(safe_minus(def->m_card->m_counter + att->m_enfeebled, att->m_protected));
}
inline CardStatus* select_first_enemy_wall(Field* fd)
{
    for(unsigned i(0); i < fd->tip->structures.size(); ++i)
    {
        CardStatus& card(fd->tip->structures[i]);
        if(card.m_card->m_wall && card.m_hp > 0) { return(&card); }
    }
    return(nullptr);
}

inline unsigned valor_damage(Field* fd, CardStatus& status)
{
    if(status.m_card->m_valor > 0)
    {
        unsigned count_ta(0);
        unsigned count_td(0);
        for(unsigned i(0); i < fd->tap->assaults.size(); ++i)
        {
            if(fd->tap->assaults[i].m_hp > 0) { ++count_ta; }
        }
        for(unsigned i(0); i < fd->tip->assaults.size(); ++i)
        {
            if(fd->tip->assaults[i].m_hp > 0) { ++count_td; }
        }
        if(count_ta < count_td) { return(status.m_card->m_valor); }
    }
    return(0);
}

inline unsigned attack_damage_against_non_assault(Field* fd, CardStatus& att_status)
{
    const Card& att_card(*att_status.m_card);
    assert(att_card.m_type == CardType::assault);
    // pre modifier damage
    unsigned damage(attack_power(&att_status));
    //
    if(damage > 0)
    {
        damage += valor_damage(fd, att_status);
    }
    return(damage);
}

inline unsigned attack_damage_against_assault(Field* fd, CardStatus& att_status, CardStatus& def_status)
{
    const Card& att_card(*att_status.m_card);
    const Card& def_card(*def_status.m_card);
    assert(att_card.m_type == CardType::assault);
    assert(def_card.m_type == CardType::assault);
    // pre modifier damage
    unsigned damage(attack_power(&att_status));
    //
    if(damage > 0)
    {
        damage = safe_minus(
            damage // pre-modifier damage
            + valor_damage(fd, att_status) // valor
            + def_status.m_enfeebled // enfeeble
            + (def_card.m_flying ? att_card.m_antiair : 0) // anti-air
            + (att_card.m_burst > 0 ? (def_status.m_hp == def_card.m_health ? att_card.m_burst : 0) : 0) // burst
            // armor + protect + pierce
            , safe_minus(def_card.m_armored + def_status.m_protected, att_card.m_pierce));
    }
    return(damage);
}

inline bool alive_assault(Storage<CardStatus>& assaults, unsigned index)
{
    return(index >= 0 && assaults.size() > index && assaults[index].m_hp > 0);
}

void remove_commander_hp(Field* fd, CardStatus& status, unsigned dmg)
{
    assert(status.m_hp > 0);
    assert(status.m_card->m_type == CardType::commander);
    status.m_hp = safe_minus(status.m_hp, dmg);
    if(status.m_hp == 0) { fd->end = true; }
}
//------------------------------------------------------------------------------
// implementation of one attack by an assault card, against either an enemy
// assault card, the first enemy wall, or the enemy commander.
struct PerformAttack
{
    Field* fd;
    CardStatus* att_status;
    CardStatus* def_status;
    unsigned att_dmg;
    bool killed_by_attack;

    PerformAttack(Field* fd_, CardStatus* att_status_, CardStatus* def_status_) :
        fd(fd_), att_status(att_status_), def_status(def_status_), att_dmg(0), killed_by_attack(false)
    {}

    template<enum CardType::CardType cardtype>
    void op()
    {
        if(attack_power(att_status) > 0)
        {
            const bool fly_check(!def_status->m_card->m_flying || att_status->m_card->m_flying || att_status->m_card->m_antiair > 0 ||
                                 (fd->effect != Effect::high_skies && fd->flip()));
            if(fly_check) // unnecessary check for structures, commander -> fix later ?
            {
                // Evaluation order:
                // assaults only: fly check
                // assaults only: immobilize
                // deal damage
                // assaults only: (siphon, poison, disease)
                // oa: poison, disease, assaults only: berserk, skills
                // counter, berserk
                // assaults only: (crush, leech if still alive)
                // check regeneration
                att_dmg = calculate_attack_damage<cardtype>();

                // If Impenetrable, force attack damage against walls to be 0,
                // but still activate Counter!
                if(fd->effect == Effect::impenetrable && def_status->m_card->m_wall)
                {
                    att_dmg = 0;
                }

                if(att_dmg > 0)
                {
                    immobilize<cardtype>();
                    attack_damage<cardtype>();
                    siphon_poison_disease<cardtype>();
                }
                oa<cardtype>();
                if(att_dmg > 0)
                {
                    if(att_status->m_hp > 0)
                    {
                        counter<cardtype>();
                        berserk<cardtype>();
                    }
                    crush_leech<cardtype>();
                }

                // If Impenetrable, force attack damage against walls to be 0,
                // but still activate Counter!
                if(fd->effect == Effect::impenetrable && def_status->m_card->m_wall && att_status->m_hp > 0)
                {
                    counter<cardtype>();
                }

                prepend_on_death(fd);
                resolve_skill(fd);
                check_regeneration(fd);
            }
        }
    }

    template<enum CardType::CardType>
    unsigned calculate_attack_damage()
    {
        return(attack_damage_against_non_assault(fd, *att_status));
    }

    template<enum CardType::CardType>
    void immobilize() {}

    template<enum CardType::CardType>
    void attack_damage()
    {
        remove_hp(fd, *def_status, att_dmg);
        killed_by_attack = def_status->m_hp == 0;
        _DEBUG_MSG("%s attack damage %u\n", status_description(att_status).c_str(), att_dmg);
    }

    template<enum CardType::CardType>
    void siphon_poison_disease() {}

    template<enum CardType::CardType cardtype>
    void oa()
    {
        if(def_status->m_card->m_poison_oa > 0)
        {
            apply_poison(att_status, def_status->m_card->m_poison_oa);
        }
        if(def_status->m_card->m_disease_oa)
        {
            att_status->m_diseased = true;
        }
        oa_berserk<cardtype>();
        for(auto& oa_skill: def_status->m_card->m_skills_attacked)
        {
            fd->skill_queue.emplace_back(def_status, oa_skill);
            resolve_skill(fd);
        }
    }

    template<enum CardType::CardType>
    void oa_berserk() {}

    template<enum CardType::CardType>
    void counter()
    {
        if(def_status->m_card->m_counter > 0)
        {
            unsigned counter_dmg(counter_damage(att_status, def_status));
            remove_hp(fd, *att_status, counter_dmg);
            _DEBUG_MSG("%s counter %u by %s\n", status_description(att_status).c_str(), counter_dmg, status_description(def_status).c_str());
        }
    }

    template<enum CardType::CardType>
    void berserk()
    {
        att_status->m_berserk += att_status->m_card->m_berserk;
    }

    template<enum CardType::CardType>
    void crush_leech() {}
};

template<>
unsigned PerformAttack::calculate_attack_damage<CardType::assault>()
{
    return(attack_damage_against_assault(fd, *att_status, *def_status));
}

template<>
void PerformAttack::immobilize<CardType::assault>()
{
    if(att_status->m_card->m_immobilize && def_status->m_delay <= 1 && !def_status->m_jammed && !def_status->m_frozen)
    {
        def_status->m_immobilized |= fd->flip();
    }
}

template<>
void PerformAttack::attack_damage<CardType::commander>()
{
    remove_commander_hp(fd, *def_status, att_dmg);
    _DEBUG_MSG("%s attack damage %u to commander; commander hp %u\n", status_description(att_status).c_str(), att_dmg, fd->tip->commander.m_hp);
}

template<>
void PerformAttack::siphon_poison_disease<CardType::assault>()
{
    if(att_status->m_card->m_siphon > 0)
    {
        add_hp(fd, &fd->tap->commander, std::min(att_dmg, att_status->m_card->m_siphon));
        _DEBUG_MSG(" \033[1;32m%s siphon %u; hp %u\033[0m\n", status_description(att_status).c_str(), std::min(att_dmg, att_status->m_card->m_siphon), fd->tap->commander.m_hp);
    }
    if(att_status->m_card->m_poison > 0)
    {
        apply_poison(def_status, att_status->m_card->m_poison);
    }
    if(att_status->m_card->m_disease)
    {
        def_status->m_diseased = true;
    }
}

template<>
void PerformAttack::oa_berserk<CardType::assault>() { def_status->m_berserk += def_status->m_card->m_berserk_oa; }

template<>
void PerformAttack::crush_leech<CardType::assault>()
{
    if(att_status->m_card->m_crush > 0 && killed_by_attack)
    {
        CardStatus* def_status{select_first_enemy_wall(fd)}; // defending wall
        if (def_status != nullptr)
        {
            remove_hp(fd, *def_status, att_status->m_card->m_crush);
            _DEBUG_MSG("%s crush %u; wall %s hp %u\n", status_description(att_status).c_str(), att_status->m_card->m_crush, status_description(def_status).c_str(), def_status->m_hp);
        }
        else
        {
            remove_commander_hp(fd, fd->tip->commander, att_status->m_card->m_crush);
            _DEBUG_MSG("%s crush %u; commander hp %u\n", status_description(att_status).c_str(), att_status->m_card->m_crush, fd->tip->commander.m_hp);
        }
    }
    if(att_status->m_card->m_leech > 0 && att_status->m_hp > 0 && !att_status->m_diseased)
    {
        add_hp(fd, att_status, std::min(att_dmg, att_status->m_card->m_leech));
        _DEBUG_MSG("%s leech %u; hp: %u.\n", status_description(att_status).c_str(), std::min(att_dmg, att_status->m_card->m_leech), att_status->m_hp);
    }
}

// General attack phase by the currently evaluated assault, taking into accounts exotic stuff such as flurry,swipe,etc.
void attack_phase(Field* fd)
{
    CardStatus* att_status(&fd->tap->assaults[fd->current_ci]); // attacking card
    Storage<CardStatus>& def_assaults(fd->tip->assaults);
    unsigned num_attacks(att_status->m_card->m_flurry > 0 && fd->flip() ? att_status->m_card->m_flurry + 1 : 1);
    for(unsigned attack_index(0); attack_index < num_attacks && !att_status->m_jammed && !att_status->m_frozen && att_status->m_hp > 0 && fd->tip->commander.m_hp > 0; ++attack_index)
    {
        // 3 possibilities:
        // - 1. attack against the assault in front
        // - 2. swipe attack the assault in front and adjacent assaults if any
        // - 3. attack against the commander or walls (if there is no assault or if the attacker has the fear attribute)
        // Check if attack mode is 1. or 2. (there is a living assault card in front, and no fear)
        if(alive_assault(def_assaults, fd->current_ci) && !att_status->m_card->m_fear)
        {
            // attack mode 1.
            if(!att_status->m_card->m_swipe)
            {
                PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci]}.op<CardType::assault>();
            }
            // attack mode 2.
            else
            {
                // attack the card on the left
                if(alive_assault(def_assaults, fd->current_ci - 1))
                {
                    PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci-1]}.op<CardType::assault>();
                }
                // stille alive? attack the card in front
                if(fd->tip->commander.m_hp > 0 && att_status->m_hp > 0 && alive_assault(def_assaults, fd->current_ci))
                {
                    PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci]}.op<CardType::assault>();
                }
                // still alive? attack the card on the right
                if(fd->tip->commander.m_hp > 0 && att_status->m_hp > 0 && alive_assault(def_assaults, fd->current_ci + 1))
                {
                    PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci+1]}.op<CardType::assault>();
                }
            }
        }
        // attack mode 3.
        else
        {
            CardStatus* def_status{select_first_enemy_wall(fd)}; // defending wall
            if(def_status != nullptr)
            {
                PerformAttack{fd, att_status, def_status}.op<CardType::structure>();
            }
            else
            {
                PerformAttack{fd, att_status, &fd->tip->commander}.op<CardType::commander>();
            }
        }
    }
}

//---------------------- $65 active skills implementation ----------------------
unsigned strike_damage(CardStatus* target, unsigned v)
{
    return(safe_minus(v + target->m_enfeebled, target->m_protected));
}

template<
    bool C
    , typename T1
    , typename T2
    >
struct if_
{
    typedef T1 type;
};

template<
    typename T1
    , typename T2
    >
struct if_<false,T1,T2>
{
    typedef T2 type;
};

template<unsigned skill_id>
inline bool skill_predicate(CardStatus* c)
{ assert(false); return(false); }

template<>
inline bool skill_predicate<augment>(CardStatus* c)
{
    if(c->m_hp > 0 && (c->m_delay == 0 || c->blitz) && !c->m_jammed && !c->m_frozen)
    {
        for(auto& s: c->m_card->m_skills)
        {
            // Any quantifiable skill except augment
            if(std::get<1>(s) > 0 && std::get<0>(s) != augment && std::get<0>(s) != augment_all && std::get<0>(s) != summon) { return(true); }
        }
    }
    return(false);
}

template<>
inline bool skill_predicate<chaos>(CardStatus* c)
{ return(c->m_delay <= 1 && c->m_hp > 0 && !c->m_chaos && !c->m_jammed && !c->m_frozen); }

template<>
inline bool skill_predicate<cleanse>(CardStatus* c)
{
    return(c->m_hp > 0 && (
               c->m_chaos ||
               c->m_diseased ||
               c->m_enfeebled > 0 ||
               (c->m_frozen && c->m_delay == 0) ||
               c->m_jammed ||
               c->m_poisoned
               ));
}

template<>
inline bool skill_predicate<enfeeble>(CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<freeze>(CardStatus* c)
{ return(c->m_hp > 0 && !c->m_jammed && !c->m_frozen); }

template<>
inline bool skill_predicate<heal>(CardStatus* c)
{ return(c->m_hp > 0 && c->m_hp < c->m_card->m_health && !c->m_diseased); }

template<>
inline bool skill_predicate<infuse>(CardStatus* c)
{ return(c->m_faction != bloodthirsty); }

template<>
inline bool skill_predicate<jam>(CardStatus* c)
{ return(c->m_delay <= 1 && c->m_hp > 0 && !c->m_jammed && !c->m_frozen); }

template<>
inline bool skill_predicate<mimic>(CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<protect>(CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<rally>(CardStatus* c)
{ return((c->m_delay == 0 || c->blitz) && c->m_hp > 0 && !c->m_jammed && !c->m_frozen && !c->m_immobilized); }

template<>
inline bool skill_predicate<repair>(CardStatus* c)
{ return(c->m_hp > 0 && c->m_hp < c->m_card->m_health); }

template<>
inline bool skill_predicate<rush>(CardStatus* c)
{ return(c->m_delay > 0); }

template<>
inline bool skill_predicate<siege>(CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<strike>(CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<supply>(CardStatus* c)
{ return(c->m_hp > 0 && c->m_hp < c->m_card->m_health && !c->m_diseased); }

template<>
inline bool skill_predicate<temporary_split>(CardStatus* c)
// It is unnecessary to check for Blitz, since temporary_split status is
// awarded before a card is played.
{ return(c->m_delay == 0 && c->m_hp > 0); }

template<>
inline bool skill_predicate<weaken>(CardStatus* c)
{ return(c->m_delay <= 1 && c->m_hp > 0 && attack_power(c) > 0 && !c->m_jammed && !c->m_frozen && !c->m_immobilized); }

template<unsigned skill_id>
inline void perform_skill(Field* fd, CardStatus* c, unsigned v)
{ assert(false); }

template<>
inline void perform_skill<augment>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_augmented += v;
}

template<>
inline void perform_skill<backfire>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_hp = safe_minus(c->m_hp, v);
    if(c->m_hp == 0)
    {
        fd->end = true;
    }
}

template<>
inline void perform_skill<chaos>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_chaos = true;
}

template<>
inline void perform_skill<cleanse>(Field* fd, CardStatus* c, unsigned v)
{
    if(fd->effect == Effect::decay)
    {
        return;
    }
    c->m_chaos = false;
    c->m_diseased = false;
    c->m_enfeebled = 0;
    c->m_frozen = false;
    c->m_immobilized = false;
    c->m_jammed = false;
    c->m_poisoned = 0;
}

template<>
inline void perform_skill<enfeeble>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_enfeebled += v;
}

template<>
inline void perform_skill<freeze>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_frozen = true;
}

template<>
inline void perform_skill<heal>(Field* fd, CardStatus* c, unsigned v)
{
    add_hp(fd, c, v);
}

template<>
inline void perform_skill<infuse>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_faction = bloodthirsty;
    c->m_infused = true;
    c->infused_skills.clear();
    for(auto& skill: c->m_card->m_skills)
    {
        c->infused_skills.emplace_back(std::get<0>(skill), std::get<1>(skill), std::get<2>(skill) == allfactions ? allfactions : bloodthirsty);
    }
}

template<>
inline void perform_skill<jam>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_jammed = fd->flip();
}

template<>
inline void perform_skill<protect>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_protected += v;
}

template<>
inline void perform_skill<rally>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_rallied += v;
}

template<>
inline void perform_skill<repair>(Field* fd, CardStatus* c, unsigned v)
{
    add_hp(fd, c, v);
}

template<>
inline void perform_skill<rush>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_delay = safe_minus(c->m_delay, v);
}

template<>
inline void perform_skill<shock>(Field* fd, CardStatus* c, unsigned v)
{
    _DEBUG_MSG(" \033[1;31mshock %u. hp %u -> %u.\033[0m", v, c->m_hp, safe_minus(c->m_hp, v));
    c->m_hp = safe_minus(c->m_hp, v);
}

template<>
inline void perform_skill<siege>(Field* fd, CardStatus* c, unsigned v)
{
    _DEBUG_MSG(" hp %u -> %u.", c->m_hp, safe_minus(c->m_hp, v));
    remove_hp(fd, *c, v);
}

template<>
inline void perform_skill<strike>(Field* fd, CardStatus* c, unsigned v)
{
    remove_hp(fd, *c, strike_damage(c, v));
}

template<>
inline void perform_skill<supply>(Field* fd, CardStatus* c, unsigned v)
{
    add_hp(fd, c, v);
}

template<>
inline void perform_skill<temporary_split>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_temporary_split = true;
}

template<>
inline void perform_skill<weaken>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_weakened += v;
}

template<unsigned skill_id>
inline unsigned select_fast(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    unsigned array_head{0};
    if(std::get<2>(s) == allfactions)
    {
        for(auto card: cards)
        {
            if(skill_predicate<skill_id>(card))
            {
                fd->selection_array[array_head] = card;
                ++array_head;
            }
        }
    }
    else
    {
        for(auto card: cards)
        {
            if(card->m_faction == std::get<2>(s) &&
               skill_predicate<skill_id>(card))
            {
                fd->selection_array[array_head] = card;
                ++array_head;
            }
        }
    }
    return(array_head);
}

template<unsigned skill_id>
inline unsigned select_rally_like(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    unsigned array_head{0};
    unsigned card_index(fd->current_phase == Field::assaults_phase ? fd->current_ci : 0);
    if(std::get<2>(s) == allfactions)
    {
        for(; card_index < cards.size(); ++card_index)
        {
            if(skill_predicate<skill_id>(cards[card_index]))
            {
                fd->selection_array[array_head] = cards[card_index];
                ++array_head;
            }
        }
    }
    else
    {
        for(; card_index < cards.size(); ++card_index)
        {
            if(cards[card_index]->m_faction == std::get<2>(s) &&
               skill_predicate<skill_id>(cards[card_index]))
            {
                fd->selection_array[array_head] = cards[card_index];
                ++array_head;
            }
        }
    }
    return(array_head);
}

template<>
inline unsigned select_fast<augment>(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    return(select_rally_like<augment>(fd, src_status, cards, s));
}

template<>
inline unsigned select_fast<rally>(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    return(select_rally_like<rally>(fd, src_status, cards, s));
}

template<>
inline unsigned select_fast<supply>(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    // mimiced supply by a structure, etc ?
    if(!(src_status && src_status->m_card->m_type == CardType::assault)) { return(0); }
    unsigned array_head{0};
    const unsigned min_index(src_status->m_index - (src_status->m_index == 0 ? 0 : 1));
    const unsigned max_index(src_status->m_index + (src_status->m_index == cards.size() - 1 ? 0 : 1));
    for(unsigned card_index(min_index); card_index <= max_index; ++card_index)
    {
        if(skill_predicate<supply>(cards[card_index]))
        {
            fd->selection_array[array_head] = cards[card_index];
            ++array_head;
        }
    }
    return(array_head);
}

inline unsigned select_infuse(Field* fd, const SkillSpec& s)
{
    unsigned array_head{0};
    // Select candidates among attacker's assaults
    for(auto card_status: fd->tap->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    // Select candidates among defender's assaults
    for(auto card_status: fd->tip->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    return(array_head);
}

inline std::vector<CardStatus*>& skill_targets_hostile_assault(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? (src_status->m_chaos ? src_status->m_player : opponent(src_status->m_player)) : fd->tipi]->assaults.m_indirect);
}

inline std::vector<CardStatus*>& skill_targets_allied_assault(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? src_status->m_player : fd->tapi]->assaults.m_indirect);
}

inline std::vector<CardStatus*>& skill_targets_hostile_structure(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? (src_status->m_chaos ? src_status->m_player : opponent(src_status->m_player)) : fd->tipi]->structures.m_indirect);
}

inline std::vector<CardStatus*>& skill_targets_allied_structure(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? src_status->m_player : fd->tapi]->structures.m_indirect);
}

template<unsigned skill>
std::vector<CardStatus*>& skill_targets(Field* fd, CardStatus* src_status)
{
    std::cout << "skill_targets: Error: no specialization for " << skill_names[skill] << "\n";
    assert(false);
}

template<> inline std::vector<CardStatus*>& skill_targets<augment>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<chaos>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<cleanse>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<enfeeble>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<freeze>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<heal>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<jam>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<mimic>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<protect>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<rally>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<repair>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_structure(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<rush>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<strike>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<supply>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<temporary_split>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<weaken>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<siege>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_structure(fd, src_status)); }

template<typename T>
void maybeTriggerRegen(Field* fd)
{
}

template<>
void maybeTriggerRegen<true_>(Field* fd)
{
    fd->skill_queue.emplace_front(nullptr, std::make_tuple(trigger_regen, 0, allfactions));
}

template<unsigned skill_id>
CardStatus* get_target_hostile_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    if(array_head > 0)
    {
        unsigned rand_index(fd->rand(0, array_head - 1));
        CardStatus* c(fd->selection_array[rand_index]);
        // intercept
        if(src_status && !src_status->m_chaos)
        {
            CardStatus* intercept_card(nullptr);
            if(rand_index > 0)
            {
                CardStatus* left_status(fd->selection_array[rand_index-1]);
                if(left_status->m_card->m_intercept && left_status->m_index == c->m_index-1)
                {
                    intercept_card = left_status;
                }
            }
            if(rand_index+1 < array_head && !intercept_card)
            {
                CardStatus* right_status(fd->selection_array[rand_index+1]);
                if(right_status->m_card->m_intercept && right_status->m_index == c->m_index+1)
                {
                    intercept_card = right_status;
                }
            }
            if(intercept_card) { c = intercept_card; }
        }
        return(c);
    }
    return(nullptr);
}

template<unsigned skill_id>
void perform_targetted_hostile_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    // null status = action card
    CardStatus* c(get_target_hostile_fast<skill_id>(fd, src_status, s));
    if(c)
    {
        // evade
        if(!c->m_card->m_evade || (src_status && src_status->m_chaos) || fd->flip())
        {
            _DEBUG_MSG("%s (%u) from %s on %s.\n", skill_names[skill_id].c_str(), std::get<1>(s), status_description(src_status).c_str(), status_description(c).c_str());
            // skill
            perform_skill<skill_id>(fd, c, std::get<1>(s));
            // payback
            if(c->m_card->m_payback &&
               src_status &&
               src_status->m_card->m_type == CardType::assault &&
               !src_status->m_chaos &&
               src_status->m_hp > 0 &&
               fd->flip())
            {
                // payback evade
                if(skill_predicate<skill_id>(src_status) &&
                   (!src_status->m_card->m_evade || fd->flip()))
                {
                    _DEBUG_MSG("Payback (%s %u) on (%s)\n", skill_names[skill_id].c_str(), std::get<1>(s), src_status->m_card->m_name.c_str());
                    // payback skill
                    perform_skill<skill_id>(fd, src_status, std::get<1>(s));
                }
            }
        }
    }
    maybeTriggerRegen<typename skillTriggersRegen<skill_id>::T>(fd);
    prepend_on_death(fd);
}

template<unsigned skill_id>
void perform_targetted_allied_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    if(array_head > 0)
    {
        CardStatus* c(fd->selection_array[fd->rand(0, array_head - 1)]);
        _DEBUG_MSG(" \033[1;34m%s: %s on %s\033[0m", status_description(src_status).c_str(), skill_description(s).c_str(), status_description(c).c_str());
        perform_skill<skill_id>(fd, c, std::get<1>(s));
        _DEBUG_MSG("\n");
        if(c->m_card->m_tribute &&
           src_status &&
           src_status->m_card->m_type == CardType::assault &&
           src_status != c &&
           src_status->m_hp > 0 &&
           fd->flip())
        {
            if(skill_predicate<skill_id>(src_status))
            {
                _DEBUG_MSG("Tribute (%s %u) on (%s)\n", skill_names[skill_id].c_str(), std::get<1>(s), src_status->m_card->m_name.c_str());
                perform_skill<skill_id>(fd, src_status, std::get<1>(s));
            }
        }
    }
}

template<unsigned skill_id>
void perform_global_hostile_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    unsigned payback_count(0);
    for(unsigned s_index(0); s_index < array_head; ++s_index)
    {
        CardStatus* c(fd->selection_array[s_index]);
        if(!c->m_card->m_evade || (src_status && src_status->m_chaos) || fd->flip())
        {
            _DEBUG_MSG("%s (%u) on (%s)\n", skill_names[skill_id].c_str(), std::get<1>(s), c->m_card->m_name.c_str());
            perform_skill<skill_id>(fd, c, std::get<1>(s));
            // payback
            if(c->m_card->m_payback &&
               src_status &&
               src_status->m_card->m_type == CardType::assault &&
               !src_status->m_chaos &&
               src_status->m_hp > 0 &&
               fd->flip())
            {
                ++payback_count;
            }
        }
    }
    for(unsigned i(0); i < payback_count && skill_predicate<skill_id>(src_status); ++i)
    {
        if((!src_status->m_card->m_evade || fd->flip()))
        {
            _DEBUG_MSG("Payback (%s %u) on (%s)\n", skill_names[skill_id].c_str(), std::get<1>(s), src_status->m_card->m_name.c_str());
            perform_skill<skill_id>(fd, src_status, std::get<1>(s));
        }
    }
    maybeTriggerRegen<typename skillTriggersRegen<skill_id>::T>(fd);
    prepend_on_death(fd);
}

template<unsigned skill_id>
void perform_global_allied_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    for(unsigned s_index(0); s_index < array_head; ++s_index)
    {
        CardStatus* c(fd->selection_array[s_index]);
        _DEBUG_MSG("%s (%u) on (%s)\n", skill_names[skill_id].c_str(), std::get<1>(s), c->m_card->m_name.c_str());
        perform_skill<skill_id>(fd, c, std::get<1>(s));
        if(c->m_card->m_tribute &&
           src_status &&
           src_status->m_card->m_type == CardType::assault &&
           src_status != c &&
           src_status->m_hp > 0 &&
           fd->flip())
        {
            if(skill_predicate<skill_id>(src_status))
            {
                _DEBUG_MSG("Tribute (%s %u) on (%s)\n", skill_names[skill_id].c_str(), std::get<1>(s), src_status->m_card->m_name.c_str());
                perform_skill<skill_id>(fd, src_status, std::get<1>(s));
            }
        }
    }
}

void perform_backfire(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    Hand* backfired_side = fd->players[src_status->m_player];
    _DEBUG_MSG("Performing backfire on (%s).", backfired_side->commander.m_card->m_name.c_str());
    perform_skill<backfire>(fd, &backfired_side->commander, std::get<1>(s));
    _DEBUG_MSG("\n");
}

void perform_infuse(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    unsigned array_head{0};
    // Select candidates among attacker's assaults
    for(auto card_status: fd->players[src_status->m_player]->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    // Select candidates among defender's assaults
    for(auto card_status: fd->players[opponent(src_status->m_player)]->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    if(array_head > 0)
    {
        CardStatus* c(fd->selection_array[fd->rand(0, array_head - 1)]);
        // check evade for enemy assaults only
        if(c->m_player == src_status->m_player || !c->m_card->m_evade || fd->flip())
        {
            _DEBUG_MSG("%s on (%s).", skill_names[infuse].c_str(), c->m_card->m_name.c_str());
            perform_skill<infuse>(fd, c, std::get<1>(s));
            _DEBUG_MSG("\n");
        }
    }
}

// a summoned card's on play skills seem to be evaluated before any other skills on the skill queue.
inline void prepend_skills(Field* fd, CardStatus* status)
{
    for(auto& skill: boost::adaptors::reverse(status->m_card->m_skills_played))
    {
        fd->skill_queue.emplace_front(status, skill);
    }
}
void summon_card(Field* fd, unsigned player, const Card* summoned)
{
    assert(summoned->m_type == CardType::assault || summoned->m_type == CardType::structure);
    Hand* hand{fd->players[player]};
    if(hand->assaults.size() + hand->structures.size() < 100)
    {
        Storage<CardStatus>* storage{summoned->m_type == CardType::assault ? &hand->assaults : &hand->structures};
        CardStatus& card_status(storage->add_back());
        card_status.set(summoned);
        card_status.m_index = storage->size() - 1;
        card_status.m_player = player;
        _DEBUG_MSG("Summoned [%s] as %s %d\n", summoned->m_name.c_str(), cardtype_names[summoned->m_type].c_str(), card_status.m_index);
        prepend_skills(fd, &card_status);
        if(card_status.m_card->m_blitz &&
           fd->players[opponent(player)]->assaults.size() > card_status.m_index &&
           fd->players[opponent(player)]->assaults[card_status.m_index].m_hp > 0 &&
           fd->players[opponent(player)]->assaults[card_status.m_index].m_delay == 0)
        {
            card_status.blitz = true;
        }

        if(fd->effect == Effect::toxic)
        {
            card_status.m_poisoned = 1;
        }
        else if(fd->effect == Effect::decay)
        {
            card_status.m_poisoned = 1;
            card_status.m_diseased = true;
        }
    }
}
void perform_summon(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    summon_card(fd, src_status ? src_status->m_player : fd->tapi, fd->cards.by_id(std::get<1>(s)));
}

void perform_trigger_regen(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    check_regeneration(fd);
}

void perform_shock(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    _DEBUG_MSG("Performing shock on (%s).", fd->tip->commander.m_card->m_name.c_str());
    perform_skill<shock>(fd, &fd->tip->commander, std::get<1>(s));
    _DEBUG_MSG("\n");
}

void perform_supply(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    perform_global_allied_fast<supply>(fd, src_status, s);
}

// Special rules for mimic :
// cannot mimic mimic,
// structures cannot mimic supply,
// and is not affected by payback.
void perform_mimic(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    // mimic cannot be triggered by anything. So it should be the only skill in the unresolved skill table.
    // so we can probably clear it safely. This is necessary, because mimic calls resolve_skill as well (infinite loop).
    fd->skill_queue.clear();
    CardStatus* c(nullptr);
    if(fd->effect == Effect::copycat)
    {
        std::vector<CardStatus*>& cards(skill_targets_allied_assault(fd, src_status));
        unsigned array_head{select_fast<mimic>(fd, src_status, cards, s)};
        if(array_head > 0)
        {
            c = fd->selection_array[fd->rand(0, array_head - 1)];
        }
    }
    else
    {
        c = get_target_hostile_fast<mimic>(fd, src_status, s);
        // evade check for mimic
        // individual skills are subject to evade checks too,
        // but resolve_skill will handle those.
        if(c && c->m_card->m_evade && (!src_status || !src_status->m_chaos) && fd->flip())
        {
            return;
        }
    }

    if(c)
    {
        _DEBUG_MSG("%s on (%s)\n", skill_names[std::get<0>(s)].c_str(), c->m_card->m_name.c_str());
        for(auto skill: c->m_card->m_skills)
        {
            if(src_status && src_status->m_card->m_type == CardType::assault && src_status->m_hp == 0)
            { break; }
            if(std::get<0>(skill) != mimic &&
               (std::get<0>(skill) != supply || (src_status && src_status->m_card->m_type == CardType::assault)))
            {
                SkillSpec mimic_s(std::get<0>(skill), std::get<1>(skill), allfactions);
                fd->skill_queue.emplace_back(src_status, src_status && src_status->m_augmented > 0 ? augmented_skill(src_status, mimic_s) : mimic_s);
                resolve_skill(fd);
                check_regeneration(fd);
            }
        }
    }
}
//------------------------------------------------------------------------------
void fill_skill_table()
{
    skill_table[augment] = perform_targetted_allied_fast<augment>;
    skill_table[augment_all] = perform_global_allied_fast<augment>;
    skill_table[backfire] = perform_backfire;
    skill_table[chaos] = perform_targetted_hostile_fast<chaos>;
    skill_table[chaos_all] = perform_global_hostile_fast<chaos>;
    skill_table[cleanse] = perform_targetted_allied_fast<cleanse>;
    skill_table[cleanse_all] = perform_global_allied_fast<cleanse>;
    skill_table[enfeeble] = perform_targetted_hostile_fast<enfeeble>;
    skill_table[enfeeble_all] = perform_global_hostile_fast<enfeeble>;
    skill_table[freeze] = perform_targetted_hostile_fast<freeze>;
    skill_table[freeze_all] = perform_global_hostile_fast<freeze>;
    skill_table[heal] = perform_targetted_allied_fast<heal>;
    skill_table[heal_all] = perform_global_allied_fast<heal>;
    skill_table[infuse] = perform_infuse;
    skill_table[jam] = perform_targetted_hostile_fast<jam>;
    skill_table[jam_all] = perform_global_hostile_fast<jam>;
    skill_table[mimic] = perform_mimic;
    skill_table[protect] = perform_targetted_allied_fast<protect>;
    skill_table[protect_all] = perform_global_allied_fast<protect>;
    skill_table[rally] = perform_targetted_allied_fast<rally>;
    skill_table[rally_all] = perform_global_allied_fast<rally>;
    skill_table[repair] = perform_targetted_allied_fast<repair>;
    skill_table[repair_all] = perform_global_allied_fast<repair>;
    skill_table[rush] = perform_targetted_allied_fast<rush>;
    skill_table[shock] = perform_shock;
    skill_table[siege] = perform_targetted_hostile_fast<siege>;
    skill_table[siege_all] = perform_global_hostile_fast<siege>;
    skill_table[supply] = perform_supply;
    skill_table[strike] = perform_targetted_hostile_fast<strike>;
    skill_table[strike_all] = perform_global_hostile_fast<strike>;
    skill_table[summon] = perform_summon;
    skill_table[temporary_split] = perform_targetted_allied_fast<temporary_split>;
    skill_table[trigger_regen] = perform_trigger_regen;
    skill_table[weaken] = perform_targetted_hostile_fast<weaken>;
    skill_table[weaken_all] = perform_global_hostile_fast<weaken>;
}
//------------------------------------------------------------------------------
// Utility functions for modify_cards.

// Adds the skill "<new_skill> <magnitude>" to all assaults,
// except those who have any instance of either <new_skill> or <conflict>.
inline void maybe_add_to_assaults(Cards& cards, ActiveSkill new_skill, unsigned magnitude, ActiveSkill conflict)
{
    for(Card* card: cards.cards)
    {
        if(card->m_type != CardType::assault)
        {
            continue;
        }

        bool conflict(false);
        for(auto& skill: card->m_skills)
        {
            if(std::get<0>(skill) == new_skill ||
               std::get<0>(skill) == conflict)
            {
                conflict = true;
            }
        }

        if(!conflict)
        {
            card->add_skill(new_skill, magnitude, allfactions);
        }
    }
}
// Adds the skill "<skill> <magnitude>" to all commanders.
inline void add_to_commanders(Cards& cards, ActiveSkill skill, unsigned magnitude)
{
    for(Card* card: cards.cards)
    {
        if(card->m_type != CardType::commander)
        {
            continue;
        }
        card->add_skill(skill, magnitude, allfactions);
    }
}

// Adds the skill "<new> <magnitude>" to all commanders.
// If the commander has an instance of either <old> or <new> in its skill list,
// the new skill replaces it.
// Otherwise, the new skill is added on to the end.
inline void replace_on_commanders(Cards& cards, ActiveSkill old_skill, ActiveSkill new_skill, unsigned magnitude)
{
    for(Card* card: cards.cards)
    {
        if(card->m_type != CardType::commander)
        {
            continue;
        }

        bool replaced(false);
        for(auto& skill: card->m_skills)
        {
            if(std::get<0>(skill) == old_skill ||
               std::get<0>(skill) == new_skill)
            {
                skill = std::make_tuple(new_skill, magnitude, allfactions);
                replaced = true;
            }
        }

        if(!replaced)
        {
            card->add_skill(new_skill, magnitude, allfactions);
        }
    }
}
//------------------------------------------------------------------------------
void modify_cards(Cards& cards, enum Effect effect)
{
    switch (effect)
    {
        case Effect::none:
            break;
        case Effect::time_surge:
            add_to_commanders(cards, rush, 1);
            break;
        case Effect::copycat:
            // Do nothing; this is implemented in perform_mimic
            break;
        case Effect::quicksilver:
            for(Card* card: cards.cards)
            {
                if(card->m_type == CardType::assault)
                {
                    card->m_evade = true;
                }
            }
            break;
        case Effect::decay:
            // Do nothing; this is implemented in PlayCard::fieldEffects,
            // summon_card, and perform_skill<cleanse>
            break;
        case Effect::high_skies:
            // Do nothing; this is implemented in PerformAttack
            break;
        case Effect::impenetrable:
            // Also implemented in PerformAttack
            for(Card* card: cards.cards)
            {
                if(card->m_type == CardType::structure)
                {
                    card->m_refresh = false;
                }
            }
            break;
        case Effect::invigorate:
            // Do nothing; this is implemented in add_hp
            break;
        case Effect::clone_project:
        case Effect::clone_experiment:
            // Do nothing; these are implemented in the temporary_split skill
            break;
        case Effect::friendly_fire:
            replace_on_commanders(cards, chaos, chaos_all, 0);
            maybe_add_to_assaults(cards, strike, 1, strike_all);
            break;
        case Effect::genesis:
            // Do nothing; this is implemented in play
            break;
        case Effect::decrepit:
            replace_on_commanders(cards, enfeeble, enfeeble_all, 1);
            break;
        case Effect::forcefield:
            replace_on_commanders(cards, protect, protect_all, 1);
            break;
        case Effect::chilling_touch:
            add_to_commanders(cards, freeze, 0);
            break;
        case Effect::toxic:
            // Do nothing; this is implemented in PlayCard::fieldEffects
            // and summon_card
            break;
        default:
            // TODO: throw something more useful
            throw effect;
            break;
    }
}
