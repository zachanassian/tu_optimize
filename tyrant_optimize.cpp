// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------
//#define NDEBUG
#define BOOST_THREAD_USE_LIB
#include <cassert>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
#include <array>
#include <deque>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <tuple>
#include <boost/range/join.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/pool/pool.hpp>
#include <boost/optional.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/math/distributions/binomial.hpp>
#include <boost/filesystem.hpp>
#include "card.h"
#include "cards.h"
#include "deck.h"
#include "tyrant.h"
#include "xml.h"
//#include "timer.hpp"

using namespace std::placeholders;
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
// Pool-based indexed storage.
//---------------------- Pool-based indexed storage ----------------------------
template<typename T>
class Storage
{
public:
    typedef typename std::vector<T*>::size_type size_type;
    typedef T value_type;
    Storage(size_type size) :
        m_pool(sizeof(T))
    {
        m_indirect.reserve(size);
    }

    inline T& operator[](size_type i)
    {
        return(*m_indirect[i]);
    }

    inline T& add_back()
    {
        m_indirect.emplace_back((T*) m_pool.malloc());
        return(*m_indirect.back());
    }

    template<typename Pred>
    void remove(Pred p)
    {
        size_type head(0);
        for(size_type current(0); current < m_indirect.size(); ++current)
        {
            if(p((*this)[current]))
            {
                m_pool.free(m_indirect[current]);
            }
            else
            {
                if(current != head)
                {
                    m_indirect[head] = m_indirect[current];
                }
                ++head;
            }
        }
        m_indirect.erase(m_indirect.begin() + head, m_indirect.end());
    }

    void reset()
    {
        for(auto index: m_indirect)
        {
            m_pool.free(index);
        }
        m_indirect.clear();
    }

    inline size_type size() const
    {
        return(m_indirect.size());
    }

    std::vector<T*> m_indirect;
    boost::pool<> m_pool;
};
//--------------------- $10 data model: card properties, etc -------------------
Cards globalCards;
//------------------------------------------------------------------------------
struct CardStatus
{
    const Card* m_card;
    unsigned m_index;
    unsigned m_player;
    unsigned m_augmented;
    unsigned m_berserk;
    bool blitz;
    bool m_chaos;
    unsigned m_delay;
    bool m_diseased;
    unsigned m_enfeebled;
    Faction m_faction;
    bool m_frozen;
    unsigned m_hp;
    bool m_immobilized;
    bool m_infused;
    std::vector<SkillSpec> infused_skills;
    bool m_jammed;
    unsigned m_poisoned;
    unsigned m_protected;
    unsigned m_rallied;
    unsigned m_weakened;

    CardStatus() {}

    CardStatus(const Card* card) :
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
        m_weakened(0)
    {
    }

    inline void set(const Card* card)
    {
        this->set(*card);
    }

    inline void set(const Card& card)
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
};
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
    }
    desc += "[" + status->m_card->m_name + "]";
    return(desc);
}
//------------------------------------------------------------------------------
void print_deck(DeckIface& deck)
{
    std::cout << "Deck:" << std::endl;
    if(deck.commander)
    {
        std::cout << deck.commander->m_name << "\n";
    }
    else
    {
        std::cout << "No commander\n";
    }
    for(const Card* card: deck.cards)
    {
        std::cout << "  " << card->m_name << "\n" << std::flush;
    }
}
//------------------------------------------------------------------------------
// Represents a particular draw from a deck.
// Persistent object: call reset to get a new draw.
class Hand
{
public:

    Hand(DeckIface* deck_) :
        deck(deck_),
        assaults(15),
        structures(15)
    {
    }

    void reset(std::mt19937& re)
    {
        assaults.reset();
        structures.reset();
        commander = CardStatus(deck->get_commander());
        deck->shuffle(re);
    }

    DeckIface* deck;
    CardStatus commander;
    Storage<CardStatus> assaults;
    Storage<CardStatus> structures;
};
//---------------------- $40 Game rules implementation -------------------------
// Everything about how a battle plays out, except the following:
// the implementation of the attack by an assault card is in the next section;
// the implementation of the active skills is in the section after that.
// struct Field is the data model of a battle:
// an attacker and a defender deck, list of assaults and structures, etc.
unsigned turn_limit{50};
class Field
{
public:
    bool end;
    std::mt19937& re;
    const Cards& cards;
    // players[0]: the attacker, players[1]: the defender
    std::array<Hand*, 2> players;
    unsigned tapi; // current turn's active player index
    unsigned tipi; // and inactive
    Hand* tap;
    Hand* tip;
    std::array<CardStatus*, 256> selection_array;
    unsigned turn;
    gamemode_t gamemode;
    // With the introduction of on death skills, a single skill can trigger arbitrary many skills.
    // They are stored in this, and cleared after all have been performed.
    std::deque<std::tuple<CardStatus*, SkillSpec> > skill_queue;
    std::vector<CardStatus*> killed_with_on_death;
    std::vector<CardStatus*> killed_with_regen;
    enum phase
    {
        playcard_phase,
        commander_phase,
        structures_phase,
        assaults_phase
    };
    // the current phase of the turn: starts with playcard_phase, then commander_phase, structures_phase, and assaults_phase
    phase current_phase;
    // the index of the card being evaluated in the current phase.
    // Meaningless in playcard_phase,
    // otherwise is the index of the current card in players->structures or players->assaults
    unsigned current_ci;

    Field(std::mt19937& re_, const Cards& cards_, Hand& hand1, Hand& hand2, gamemode_t _gamemode) :
        end{false},
        re(re_),
        cards(cards_),
        players{{&hand1, &hand2}},
        turn(1),
        gamemode(_gamemode)
    {
    }

    inline unsigned rand(unsigned x, unsigned y)
    {
        return(std::uniform_int_distribution<unsigned>(x, y)(re));
    }

    inline unsigned flip()
    {
        return(this->rand(0,1));
    }

    template <typename T>
    inline T& random_in_vector(std::vector<T>& v)
    {
        assert(v.size() > 0);
        return(v[this->rand(0, v.size() - 1)]);
    }
};
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
void evaluate_skills(Field* fd, CardStatus* status, const std::vector<SkillSpec>& skills)
{
    assert(status);
    assert(fd->skill_queue.size() == 0);
    for(auto& skill: skills)
    {
        _DEBUG_MSG("Evaluating %s skill %s\n", status_description(status).c_str(), skill_description(skill).c_str());
        fd->skill_queue.emplace_back(status, status->m_augmented == 0 ? skill : augmented_skill(status, skill));
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
// return value : 0 -> attacker wins, 1 -> defender wins
unsigned play(Field* fd)
{
    fd->players[0]->commander.m_player = 0;
    fd->players[1]->commander.m_player = 1;
    fd->tapi = fd->gamemode == surge ? 1 : 0;
    fd->tipi = opponent(fd->tapi);
    fd->tap = fd->players[fd->tapi];
    fd->tip = fd->players[fd->tipi];
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
            }
        }
        // Evaluate commander
        fd->current_phase = Field::commander_phase;
        evaluate_skills(fd, &fd->tap->commander, fd->tap->commander.m_card->m_skills);
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
                // Special case: check for split (tartarus swarm raid)
                if(current_status.m_card->m_split && fd->tap->assaults.size() + fd->tap->structures.size() < 100)
                {
                    CardStatus& status_split(fd->tap->assaults.add_back());
                    status_split.set(current_status.m_card);
                    _DEBUG_MSG("Split assault %d (%s)\n", fd->tap->assaults.size() - 1, current_status.m_card->m_name.c_str());
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
void check_regeneration(Field* fd)
{
    for(unsigned i(0); i < fd->killed_with_regen.size(); ++i)
    {
        CardStatus& status = *fd->killed_with_regen[i];
        if(status.m_hp == 0 && status.m_card->m_regenerate > 0 && !status.m_diseased)
        {
            status.m_hp = fd->flip() ? status.m_card->m_regenerate : 0;
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
        }
    }
    // Defending player's assault cards:
    // update index
    // remove augment, chaos, freeze, immobilize, jam, rally, weaken, apply refresh
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
            if(status.m_card->m_refresh && !status.m_diseased)
            {
#ifndef NDEBUG
                if(status.m_hp < status.m_card->m_health)
                {
                    _DEBUG_MSG("%u %s refreshed. hp %u -> %u.\n", index, status_description(&status).c_str(), status.m_hp, status.m_card->m_health);
                }
#endif
                status.m_hp = status.m_card->m_health;
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
                status.m_hp = status.m_card->m_health;
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
inline void add_hp(CardStatus* target, unsigned v)
{
    target->m_hp = std::min(target->m_hp + v, target->m_card->m_health);
}
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
            const bool fly_check(!def_status->m_card->m_flying || att_status->m_card->m_flying || att_status->m_card->m_antiair > 0 || fd->flip());
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
                        counter_berserk<cardtype>();
                    }
                    crush_leech<cardtype>();
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
    void counter_berserk()
    {
        if(def_status->m_card->m_counter > 0)
        {
            unsigned counter_dmg(counter_damage(att_status, def_status));
            remove_hp(fd, *att_status, counter_dmg);
            _DEBUG_MSG("%s counter %u by %s\n", status_description(att_status).c_str(), counter_dmg, status_description(def_status).c_str());
        }
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
    if(att_status->m_card->m_immobilize)
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
        add_hp(&fd->tap->commander, std::min(att_dmg, att_status->m_card->m_siphon));
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
        add_hp(att_status, std::min(att_dmg, att_status->m_card->m_leech));
        _DEBUG_MSG("%s leech %u; hp: %u.\n", status_description(att_status).c_str(), std::min(att_dmg, att_status->m_card->m_leech), att_status->m_hp);
    }
}

// General attack phase by the currently evaluated assault, taking into accounts exotic stuff such as flurry,swipe,etc.
void attack_phase(Field* fd)
{
    CardStatus* att_status(&fd->tap->assaults[fd->current_ci]); // attacking card
    Storage<CardStatus>& def_assaults(fd->tip->assaults);
    unsigned num_attacks(att_status->m_card->m_flurry > 0 && fd->flip() ? att_status->m_card->m_flurry + 1 : 1);
    for(unsigned attack_index(0); attack_index < num_attacks && att_status->m_hp > 0 && fd->tip->commander.m_hp > 0; ++attack_index)
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
{ return(c->m_delay <= 1 && c->m_hp > 0 && !c->m_chaos); }

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
{ return(c->m_hp > 0 && !c->m_frozen); }

template<>
inline bool skill_predicate<heal>(CardStatus* c)
{ return(c->m_hp > 0 && c->m_hp < c->m_card->m_health && !c->m_diseased); }

template<>
inline bool skill_predicate<infuse>(CardStatus* c)
{ return(c->m_faction != bloodthirsty); }

template<>
inline bool skill_predicate<jam>(CardStatus* c)
{ return(c->m_delay <= 1 && c->m_hp > 0 && !c->m_jammed); }

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
inline void perform_skill<chaos>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_chaos = true;
}

template<>
inline void perform_skill<cleanse>(Field* fd, CardStatus* c, unsigned v)
{
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
    add_hp(c, v);
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
    add_hp(c, v);
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

template<> std::vector<CardStatus*>& skill_targets<rush>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<strike>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<supply>(Field* fd, CardStatus* src_status)
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
        if(!c->m_card->m_evade || fd->flip())
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
        if(!c->m_card->m_evade || fd->flip())
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
    CardStatus* c(get_target_hostile_fast<mimic>(fd, src_status, s));
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

//---------------------- $70 More xml parsing: missions and raids --------------
template<typename Iterator, typename Functor> Iterator advance_until(Iterator it, Iterator it_end, Functor f)
{
    while(it != it_end)
    {
        if(f(*it))
        {
            break;
        }
        ++it;
    }
    return(it);
}

// take care that "it" is 1 past current.
template<typename Iterator, typename Functor> Iterator recede_until(Iterator it, Iterator it_beg, Functor f)
{
    if(it == it_beg) { return(it_beg); }
    --it;
    do
    {
        if(f(*it))
        {
            return(++it);
        }
        --it;
    } while(it != it_beg);
    return(it_beg);
}

template<typename Iterator, typename Functor, typename Token> Iterator read_token(Iterator it, Iterator it_end, Functor f, boost::optional<Token>& token)
{
    Iterator token_start = advance_until(it, it_end, [](const char& c){return(c != ' ');});
    Iterator token_end_after_spaces = advance_until(token_start, it_end, f);
    if(token_start != token_end_after_spaces)
    {
        Iterator token_end = recede_until(token_end_after_spaces, token_start, [](const char& c){return(c != ' ');});
        token = boost::lexical_cast<Token>(std::string{token_start, token_end});
        return(token_end_after_spaces);
    }
    return(token_end_after_spaces);
}

// Error codes:
// 2 -> file not readable
// 3 -> error while parsing file
unsigned read_custom_decks(Cards& cards, std::string filename, std::map<std::string, DeckIface*>& custom_decks)
{
    std::ifstream decks_file(filename.c_str());
    if(!decks_file.is_open())
    {
        std::cerr << "File " << filename << " could not be opened\n";
        return(2);
    }
    unsigned num_line(0);
    decks_file.exceptions(std::ifstream::badbit);
    try
    {
        while(decks_file && !decks_file.eof())
        {
            std::vector<unsigned> card_ids;
            std::string deck_string;
            getline(decks_file, deck_string);
            ++num_line;
            if(deck_string.size() > 0)
            {
                if(strncmp(deck_string.c_str(), "//", 2) == 0)
                {
                    continue;
                }
                boost::tokenizer<boost::char_delimiters_separator<char> > deck_tokens{deck_string, boost::char_delimiters_separator<char>{false, ":,", ""}};
                auto token_iter = deck_tokens.begin();
                boost::optional<std::string> deck_name;
                if(token_iter != deck_tokens.end())
                {
                    read_token(token_iter->begin(), token_iter->end(), [](char c){return(false);}, deck_name);
                    if(!deck_name || (*deck_name).size() == 0)
                    {
                        std::cerr << "Error in file " << filename << " at line " << num_line << ", could not read the deck name.\n";
                        continue;
                    }
                }
                else
                {
                    std::cerr << "Error in file " << filename << " at line " << num_line << ", could not read the deck name.\n";
                    continue;
                }
                ++token_iter;
                for(; token_iter != deck_tokens.end(); ++token_iter)
                {
                    std::string card_spec(*token_iter);
                    try
                    {
                        auto card_spec_iter = card_spec.begin();
                        boost::optional<std::string> card_name;
                        card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c=='[' || c=='#' || c=='\r');}, card_name);
                        if(!card_name)
                        {
                            std::cerr << "Error in file " << filename << " at line " << num_line << " while parsing card " << card_spec << " in deck " << deck_name << "\n";
                            break;
                        }
                        else
                        {
                            boost::optional<unsigned> card_id;
                            if(*card_spec_iter == '[')
                            {
                                ++card_spec_iter;
                                card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c==']');}, card_id);
                                card_spec_iter = advance_until(card_spec_iter, card_spec.end(), [](char c){return(c!=' ');});
                            }
                            boost::optional<unsigned> card_num;
                            if(*card_spec_iter == '#')
                            {
                                ++card_spec_iter;
                                card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c < '0' || c > '9');}, card_num);
                            }
                            unsigned resolved_id{card_id ? *card_id : 0};
                            if(resolved_id == 0)
                            {
                                auto card_it = cards.player_cards_by_name.find(*card_name);
                                if(card_it != cards.player_cards_by_name.end())
                                {
                                    resolved_id = card_it->second->m_id;
                                }
                                else
                                {
                                    std::cerr << "Error in file " << filename << " at line " << num_line << " while parsing card " << card_spec << " in deck " << *deck_name << ": card not found\n";
                                    break;
                                }
                            }
                            for(unsigned i(0); i < (card_num ? *card_num : 1); ++i)
                            {
                                card_ids.push_back(resolved_id);
                            }
                        }
                    }
                    catch(boost::bad_lexical_cast e)
                    {
                        std::cerr << "Error in file " << filename << " at line " << num_line << " while parsing card " << card_spec << " in deck " << deck_name << "\n";
                    }
                }
                if(deck_name)
                {
                    custom_decks.insert({*deck_name, new DeckRandom{cards, card_ids}});
                }
            }
        }
    }
    catch (std::ifstream::failure e)
    {
        std::cerr << "Exception while parsing the file " << filename << " (badbit is set).\n";
        e.what();
        return(3);
    }
}
//------------------------------------------------------------------------------
void load_decks(Decks& decks, Cards& cards)
{
    if(boost::filesystem::exists("Custom.txt"))
    {
        try
        {
            read_custom_decks(cards, std::string{"Custom.txt"}, decks.custom_decks);
        }
        catch(const std::runtime_error& e)
        {
            std::cout << "Exception while loading custom decks: " << e.what() << "\n";
        }
    }
}
//------------------------------------------------------------------------------
DeckIface* find_deck(const Decks& decks, std::string name)
{
    auto it1 = decks.mission_decks_by_name.find(name);
    if(it1 != decks.mission_decks_by_name.end())
    {
        return(it1->second);
    }
    auto it2 = decks.raid_decks_by_name.find(name);
    if(it2 != decks.raid_decks_by_name.end())
    {
        return(it2->second);
    }
    auto it3 = decks.custom_decks.find(name);
    if(it3 != decks.custom_decks.end())
    {
        return(it3->second);
    }
    return(nullptr);
}
//---------------------- $80 deck optimization ---------------------------------
//------------------------------------------------------------------------------
// Owned cards
//------------------------------------------------------------------------------
std::map<unsigned, unsigned> owned_cards;
bool use_owned_cards{false};
void read_owned_cards(Cards& cards)
{
    std::ifstream owned_file{"ownedcards.txt"};
    std::string owned_str{(std::istreambuf_iterator<char>(owned_file)), std::istreambuf_iterator<char>()};
    boost::tokenizer<boost::char_delimiters_separator<char> > tok{owned_str, boost::char_delimiters_separator<char>{false, "()\n", ""}};
    for(boost::tokenizer<boost::char_delimiters_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg)
    {
        std::string name{*beg};
        ++beg;
        assert(beg != tok.end());
        unsigned num{atoi((*beg).c_str())};
        auto card_itr = cards.player_cards_by_name.find(name);
        if(card_itr == cards.player_cards_by_name.end())
        {
            std::cerr << "Error in file ownedcards.txt, the card \"" << name << "\" does not seem to be a valid card.\n";
        }
        else
        {
            owned_cards[card_itr->second->m_id] = num;
        }
    }
}

// No raid rewards from 500 and 1k honor for ancient raids
// No very hard to get rewards (level >= 150, faction >= 13)
// No WB
// No UB
bool cheap_1(const Card* card)
{
    if(card->m_set == 2 || card->m_set == 3 || card->m_set == 4 || card->m_set == 6 || card->m_set == 5001 || card->m_set == 9000) { return(false); }
    // Ancient raids rewards
    // pantheon
    if(card->m_id == 567 || card->m_id == 566) { return(false); }
    // sentinel
    if(card->m_id == 572 || card->m_id == 564) { return(false); }
    // lithid
    if(card->m_id == 570 || card->m_id == 571) { return(false); }
    // hydra
    if(card->m_id == 565 || card->m_id == 568) { return(false); }
    // Arachnous
    if(card->m_id == 432) { return(false); }
    // Shrouded defiler
    if(card->m_id == 434) { return(false); }
    // Emergency fire
    if(card->m_id == 3021) { return(false); }
    // Turbo commando
    if(card->m_id == 428) { return(false); }
    // Solar powerhouse
    if(card->m_id == 530) { return(false); }
    // Utopia Beacon
    if(card->m_id == 469) { return(false); }
    return(true);
}

// Top commanders
std::set<unsigned> top_commanders{
    // common commanders:
    1105, // Opak
        1121, // Daizon
        // uncommon commanders:
        1031, // Dalia
        1017, // Duncan
        1120, // Emanuel
        1102, // Korvald
        // rare commanders:
        1021, // Atlas
        1153, // Daedalus
        1045, // Dracorex
        1099, // Empress
        1116, // Gaia
        1182, // Gialdrea
        1050, // IC
        1184, // Kleave
        1004, // LoT
        1123, // LtW
        1171, // Nexor
        1104, // Stavros
        1152, // Svetlana
        1141, // Tabitha
        1172, // Teiffa
        // halcyon + terminus:
        1203, // Halcyon the Corrupt shitty artwork
        1204, // Halcyon the Corrupt LE
        1200, // Corra
        1049, // Lord Halcyon
        1198, // Virulentus
        1199, // Lord Silus
        };
//------------------------------------------------------------------------------
bool suitable_non_commander(DeckIface& deck, unsigned slot, const Card* card)
{
    assert(card->m_type != CardType::commander);
    if(use_owned_cards)
    {
        if(owned_cards.find(card->m_id) == owned_cards.end()) { return(false); }
        else
        {
            unsigned num_in_deck{0};
            for(unsigned i(0); i < deck.cards.size(); ++i)
            {
                if(i != slot && deck.cards[i]->m_id == card->m_id)
                {
                    ++num_in_deck;
                }
            }
            if(owned_cards.find(card->m_id)->second <= num_in_deck) { return(false); }
        }
    }
    if(card->m_rarity == 4) // legendary - 1 per deck
    {
        for(unsigned i(0); i < deck.cards.size(); ++i)
        {
            if(i != slot && deck.cards[i]->m_rarity == 4)
            {
                return(false);
            }
        }
    }
    if(card->m_unique) // unique - 1 card with same id per deck
    {
        for(unsigned i(0); i < deck.cards.size(); ++i)
        {
            if(i != slot && deck.cards[i]->m_id == card->m_id)
            {
                return(false);
            }
        }
    }
    return(true);
}

bool keep_commander{false};
bool suitable_commander(const Card* card)
{
    assert(card->m_type == CardType::commander);
    if(keep_commander) { return(false); }
    if(use_owned_cards)
    {
        auto owned_iter = owned_cards.find(card->m_id);
        if(owned_iter == owned_cards.end()) { return(false); }
        else
        {
            if(owned_iter->second <= 0) { return(false); }
        }
    }
    if(top_commanders.find(card->m_id) == top_commanders.end()) { return(false); }
    return(true);
}
//------------------------------------------------------------------------------
double compute_efficiency(const std::pair<std::vector<unsigned> , unsigned>& results)
{
    if(results.second == 0) { return(0.); }
    double sum{0.};
    for(unsigned index(0); index < results.first.size(); ++index)
    {
        sum += (double)results.second / results.first[index];
    }
    return(results.first.size() / sum);
}
//------------------------------------------------------------------------------
bool use_efficiency{false};
double compute_score(const std::pair<std::vector<unsigned> , unsigned>& results, std::vector<double>& factors)
{
    double score{0.};
    if(use_efficiency)
    {
        score = compute_efficiency(results);
    }
    else
    {
        if(results.second == 0) { score = 0.; }
        double sum{0.};
        for(unsigned index(0); index < results.first.size(); ++index)
        {
            sum += results.first[index] * factors[index];
        }
        score = sum / std::accumulate(factors.begin(), factors.end(), 0.) / (double)results.second;
    }
    return(score);
}
//------------------------------------------------------------------------------
volatile unsigned thread_num_iterations{0}; // written by threads
std::vector<unsigned> thread_score; // written by threads
volatile unsigned thread_total{0}; // written by threads
volatile double thread_prev_score{0.0};
volatile bool thread_compare{false};
volatile bool thread_compare_stop{false}; // written by threads
volatile bool destroy_threads;
//------------------------------------------------------------------------------
// Per thread data.
// seed should be unique for each thread.
// d1 and d2 are intended to point to read-only process-wide data.
struct SimulationData
{
    std::mt19937 re;
    const Cards& cards;
    const Decks& decks;
    std::shared_ptr<DeckIface> att_deck;
    Hand att_hand;
    std::vector<std::shared_ptr<DeckIface> > def_decks;
    std::vector<Hand*> def_hands;
    std::vector<double> factors;
    gamemode_t gamemode;

    SimulationData(unsigned seed, const Cards& cards_, const Decks& decks_, unsigned num_def_decks_, std::vector<double> factors_, gamemode_t gamemode_) :
        re(seed),
        cards(cards_),
        decks(decks_),
        att_deck(),
        att_hand(nullptr),
        def_decks(num_def_decks_),
        factors(factors_),
        gamemode(gamemode_)
    {
        for(auto def_deck: def_decks)
        {
            def_hands.emplace_back(new Hand(nullptr));
        }
    }

    ~SimulationData()
    {
        for(auto hand: def_hands) { delete(hand); }
    }

    void set_decks(const DeckIface* const att_deck_, std::vector<DeckIface*> const & def_decks_)
    {
        att_deck.reset(att_deck_->clone());
        att_hand.deck = att_deck.get();
        for(unsigned i(0); i < def_decks_.size(); ++i)
        {
            def_decks[i].reset(def_decks_[i]->clone());
            def_hands[i]->deck = def_decks[i].get();
        }
    }

    inline std::vector<unsigned> evaluate()
    {
        std::vector<unsigned> res;
        for(Hand* def_hand: def_hands)
        {
            att_hand.reset(re);
            def_hand->reset(re);
            Field fd(re, cards, att_hand, *def_hand, gamemode);
            unsigned result(play(&fd));
            res.emplace_back(result);
        }
        return(res);
    }
};
//------------------------------------------------------------------------------
class Process;
void thread_evaluate(boost::barrier& main_barrier,
                     boost::mutex& shared_mutex,
                     SimulationData& sim,
                     const Process& p);
//------------------------------------------------------------------------------
class Process
{
public:
    unsigned num_threads;
    std::vector<boost::thread*> threads;
    std::vector<SimulationData*> threads_data;
    boost::barrier main_barrier;
    boost::mutex shared_mutex;
    const Cards& cards;
    const Decks& decks;
    DeckIface* att_deck;
    const std::vector<DeckIface*> def_decks;
    std::vector<double> factors;
    gamemode_t gamemode;

    Process(unsigned _num_threads, const Cards& cards_, const Decks& decks_, DeckIface* att_deck_, std::vector<DeckIface*> _def_decks, std::vector<double> _factors, gamemode_t _gamemode) :
        num_threads(_num_threads),
        cards(cards_),
        decks(decks_),
        att_deck(att_deck_),
        def_decks(_def_decks),
        factors(_factors),
        main_barrier(num_threads+1),
        gamemode(_gamemode)
    {
        destroy_threads = false;
        unsigned seed(time(0));
        for(unsigned i(0); i < num_threads; ++i)
        {
            threads_data.push_back(new SimulationData(seed + i, cards, decks, def_decks.size(), factors, gamemode));
            threads.push_back(new boost::thread(thread_evaluate, std::ref(main_barrier), std::ref(shared_mutex), std::ref(*threads_data.back()), std::ref(*this)));
        }
    }

    ~Process()
    {
        destroy_threads = true;
        main_barrier.wait();
        for(auto thread: threads) { thread->join(); }
        for(auto data: threads_data) { delete(data); }
    }

    std::pair<std::vector<unsigned> , unsigned> evaluate(unsigned num_iterations)
    {
        thread_num_iterations = num_iterations;
        thread_score = std::vector<unsigned>(def_decks.size(), 0u);
        thread_total = 0;
        thread_compare = false;
        // unlock all the threads
        main_barrier.wait();
        // wait for the threads
        main_barrier.wait();
        return(std::make_pair(thread_score, thread_total));
    }

    std::pair<std::vector<unsigned> , unsigned> compare(unsigned num_iterations, double prev_score)
    {
        thread_num_iterations = num_iterations;
        thread_score = std::vector<unsigned>(def_decks.size(), 0u);
        thread_total = 0;
        thread_prev_score = prev_score;
        thread_compare = true;
        thread_compare_stop = false;
        // unlock all the threads
        main_barrier.wait();
        // wait for the threads
        main_barrier.wait();
        return(std::make_pair(thread_score, thread_total));
    }
};
//------------------------------------------------------------------------------
void thread_evaluate(boost::barrier& main_barrier,
                     boost::mutex& shared_mutex,
                     SimulationData& sim,
                     const Process& p)
{
    while(true)
    {
        main_barrier.wait();
        sim.set_decks(p.att_deck, p.def_decks);
        if(destroy_threads) { return; }
        while(true)
        {
            shared_mutex.lock(); //<<<<
            if(thread_num_iterations == 0 || (thread_compare && thread_compare_stop)) //!
            {
                shared_mutex.unlock(); //>>>>
                main_barrier.wait();
                break;
            }
            else
            {
                --thread_num_iterations; //!
                shared_mutex.unlock(); //>>>>
                std::vector<unsigned> result{sim.evaluate()};
                shared_mutex.lock(); //<<<<
                std::vector<unsigned> thread_score_local(thread_score.size(), 0); //!
                for(unsigned index(0); index < result.size(); ++index)
                {
                    thread_score[index] += result[index] == 0 ? 1 : 0; //!
                    thread_score_local[index] = thread_score[index]; // !
                }
                ++thread_total; //!
                unsigned thread_total_local{thread_total}; //!
                shared_mutex.unlock(); //>>>>
                if(thread_compare && thread_total_local >= 1 && thread_total_local % 100 == 0)
                {
                    unsigned score_accum = 0;
                    // Multiple defense decks case: scaling by factors and approximation of a "discrete" number of events.
                    if(result.size() > 1)
                    {
                        double score_accum_d = 0.0;
                        for(unsigned i = 0; i < thread_score_local.size(); ++i)
                        {
                            score_accum_d += thread_score_local[i] * sim.factors[i];
                        }
                        score_accum_d /= std::accumulate(sim.factors.begin(), sim.factors.end(), .0d);
                        score_accum = score_accum_d;
                    }
                    else
                    {
                        score_accum = thread_score_local[0];
                    }
                    if(boost::math::binomial_distribution<>::find_upper_bound_on_p(thread_total_local, score_accum, 0.01) < thread_prev_score)
                    {
                        shared_mutex.lock(); //<<<<
                        //std::cout << thread_total_local << "\n";
                        thread_compare_stop = true; //!
                        shared_mutex.unlock(); //>>>>
                    }
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void print_score_info(const std::pair<std::vector<unsigned> , unsigned>& results, std::vector<double>& factors)
{
    std::cout << "win%: " << compute_score(results, factors) * 100.0 << " (";
    for(auto val: results.first)
    {
        std::cout << val << " ";
    }
    std::cout << "out of " << results.second << ")\n" << std::flush;
}
//------------------------------------------------------------------------------
void hill_climbing(unsigned num_iterations, DeckIface* d1, Process& proc)
{
    auto results = proc.evaluate(num_iterations);
    print_score_info(results, proc.factors);
    double current_score = compute_score(results, proc.factors);
    double best_score = current_score;
    // Non-commander cards
    auto non_commander_cards = boost::join(boost::join(proc.cards.player_assaults, proc.cards.player_structures), proc.cards.player_actions);
    const Card* best_commander = d1->commander;
    std::vector<const Card*> best_cards = d1->cards;
    bool deck_has_been_improved = true;
    bool eval_commander = true;
    while(deck_has_been_improved && best_score < 1.0)
    {
        deck_has_been_improved = false;
        for(unsigned slot_i(0); slot_i < d1->cards.size(); ++slot_i)
        {
            if(eval_commander && !keep_commander)
            {
                for(const Card* commander_candidate: proc.cards.player_commanders)
                {
                    // Various checks to check if the card is accepted
                    assert(commander_candidate->m_type == CardType::commander);
                    if(commander_candidate == best_commander) { continue; }
                    if(!suitable_commander(commander_candidate)) { continue; }
                    // Place it in the deck
                    d1->commander = commander_candidate;
                    // Evaluate new deck
                    auto compare_results = proc.compare(num_iterations, best_score);
                    current_score = compute_score(compare_results, proc.factors);
                    // Is it better ?
                    if(current_score > best_score)
                    {
                        // Then update best score/commander, print stuff
                        best_score = current_score;
                        best_commander = commander_candidate;
                        deck_has_been_improved = true;
                        std::cout << "Deck improved: commander -> " << commander_candidate->m_name << ": ";
                        print_score_info(compare_results, proc.factors);
                    }
                }
                // Now that all commanders are evaluated, take the best one
                d1->commander = best_commander;
                eval_commander = false;
            }
            for(const Card* card_candidate: non_commander_cards)
            {
                // Various checks to check if the card is accepted
                assert(card_candidate->m_type != CardType::commander);
                if(card_candidate == best_cards[slot_i]) { continue; }
                if(!suitable_non_commander(*d1, slot_i, card_candidate)) { continue; }
                // Place it in the deck
                d1->cards[slot_i] = card_candidate;
                // Evaluate new deck
                auto compare_results = proc.compare(num_iterations, best_score);
                current_score = compute_score(compare_results, proc.factors);
                // Is it better ?
                if(current_score > best_score)
                {
                    // Then update best score/slot, print stuff
                    best_score = current_score;
                    best_cards[slot_i] = card_candidate;
                    eval_commander = true;
                    deck_has_been_improved = true;
                    std::cout << "Deck improved: slot " << slot_i << " -> " << card_candidate->m_name << ": ";
                    print_score_info(compare_results, proc.factors);
                }
            }
            // Now that all cards are evaluated, take the best one
            d1->cards[slot_i] = best_cards[slot_i];
        }
    }
    std::cout << "Best deck: " << best_score * 100.0 << "%\n";
    std::cout << best_commander->m_name;
    for(const Card* card: best_cards)
    {
        std::cout << ", " << card->m_name;
    }
    std::cout << "\n";
}
//------------------------------------------------------------------------------
void hill_climbing_ordered(unsigned num_iterations, DeckOrdered* d1, Process& proc)
{
    auto results = proc.evaluate(num_iterations);
    print_score_info(results, proc.factors);
    double current_score = compute_score(results, proc.factors);
    double best_score = current_score;
    // Non-commander cards
    auto non_commander_cards = boost::join(boost::join(proc.cards.player_assaults, proc.cards.player_structures), proc.cards.player_actions);
    const Card* best_commander = d1->commander;
    std::vector<const Card*> best_cards = d1->cards;
    bool deck_has_been_improved = true;
    bool eval_commander = true;
    while(deck_has_been_improved && best_score < 1.0)
    {
        deck_has_been_improved = false;
        std::set<unsigned> remaining_cards;
        for(unsigned i = 0; i < best_cards.size(); ++i)
        {
            remaining_cards.insert(i);
        }
        while(!remaining_cards.empty())
        {
            unsigned current_slot(*remaining_cards.begin());
            remaining_cards.erase(remaining_cards.begin());
            if(eval_commander && !keep_commander)
            {
                for(const Card* commander_candidate: proc.cards.player_commanders)
                {
                    if(best_score == 1.0) { break; }
                    // Various checks to check if the card is accepted
                    assert(commander_candidate->m_type == CardType::commander);
                    if(commander_candidate == best_commander) { continue; }
                    if(!suitable_commander(commander_candidate)) { continue; }
                    // Place it in the deck
                    d1->commander = commander_candidate;
                    // Evaluate new deck
                    auto compare_results = proc.compare(num_iterations, best_score);
                    current_score = compute_score(compare_results, proc.factors);
                    // Is it better ?
                    if(current_score > best_score)
                    {
                        // Then update best score/commander, print stuff
                        best_score = current_score;
                        best_commander = commander_candidate;
                        deck_has_been_improved = true;
                        std::cout << "Deck improved: commander -> " << commander_candidate->m_name << ": ";
                        print_score_info(compare_results, proc.factors);
                    }
                }
                // Now that all commanders are evaluated, take the best one
                d1->commander = best_commander;
                eval_commander = false;
            }
            for(const Card* card_candidate: non_commander_cards)
            {
                if(best_score == 1.0) { break; }
                // Various checks to check if the card is accepted
                assert(card_candidate->m_type != CardType::commander);
                for(unsigned slot_i(0); slot_i < d1->cards.size(); ++slot_i)
                {
                    // Various checks to check if the card is accepted
                    if(card_candidate == best_cards[slot_i]) { continue; }
                    if(!suitable_non_commander(*d1, current_slot, card_candidate)) { continue; }
                    // Place it in the deck
                    d1->cards.erase(d1->cards.begin() + current_slot);
                    d1->cards.insert(d1->cards.begin() + slot_i, card_candidate);
                    // Evaluate new deck
                    auto compare_results = proc.compare(num_iterations, best_score);
                    current_score = compute_score(compare_results, proc.factors);
                    // Is it better ?
                    if(current_score > best_score)
                    {
                        // Then update best score/slot, print stuff
                        std::cout << "Deck improved: " << current_slot << " " << best_cards[current_slot]->m_name << " -> " << slot_i << " " << card_candidate->m_name << ": ";
                        best_score = current_score;
                        best_cards.erase(best_cards.begin() + current_slot);
                        best_cards.insert(best_cards.begin() + slot_i, card_candidate);
                        eval_commander = true;
                        deck_has_been_improved = true;
                        print_score_info(compare_results, proc.factors);
                    }
                    d1->cards = best_cards;
                }
            }
            // Now that all cards are evaluated, take the best one
            // d1->cards[slot_i] = best_cards[slot_i];
        }
    }
    std::cout << "Best deck: " << best_score * 100.0 << "%\n";
    std::cout << best_commander->m_name;
    for(const Card* card: best_cards)
    {
        std::cout << ", " << card->m_name;
    }
    std::cout << "\n";
}
//------------------------------------------------------------------------------
// Implements iteration over all combination of k elements from n elements.
// parameter firstIndexLimit: this is a ugly hack used to implement the special condition that
// a deck could be expected to contain at least 1 assault card. Thus the first element
// will be chosen among the assault cards only, instead of all cards.
// It works on the condition that the assault cards are sorted first in the list of cards,
// thus have indices 0..firstIndexLimit-1.
class Combination
{
public:
    Combination(unsigned all_, unsigned choose_, unsigned firstIndexLimit_ = 0) :
        all(all_),
        choose(choose_),
        firstIndexLimit(firstIndexLimit_ == 0 ? all_ - choose_ : firstIndexLimit_),
        indices(choose_, 0),
        indicesLimits(choose_, 0)
    {
        assert(choose > 0);
        assert(choose <= all);
        assert(firstIndexLimit <= all);
        indicesLimits[0] = firstIndexLimit;
        for(unsigned i(1); i < choose; ++i)
        {
            indices[i] = i;
            indicesLimits[i] =  all - choose + i;
        }
    }

    const std::vector<unsigned>& getIndices()
    {
        return(indices);
    }

    bool next()
    {
        for(index = choose - 1; index >= 0; --index)
        {
            if(indices[index] < indicesLimits[index])
            {
                ++indices[index];
                for(nextIndex = index + 1; nextIndex < choose; nextIndex++)
                {
                    indices[nextIndex] = indices[index] - index + nextIndex;
                }
                return(false);
            }
        }
        return(true);
    }

private:
    unsigned all;
    unsigned choose;
    unsigned firstIndexLimit;
    std::vector<unsigned> indices;
    std::vector<unsigned> indicesLimits;
    int index;
    int nextIndex;
};
//------------------------------------------------------------------------------
static unsigned total_num_combinations_test(0);
inline void try_all_ratio_combinations(unsigned deck_size, unsigned var_k, unsigned num_iterations, const std::vector<unsigned>& card_indices, std::vector<const Card*>& cards, const Card* commander, Process& proc, double& best_score, boost::optional<DeckRandom>& best_deck)
{
    assert(card_indices.size() > 0);
    assert(card_indices.size() <= deck_size);
    unsigned num_cards_to_combine(deck_size);
    std::vector<const Card*> unique_cards;
    std::vector<const Card*> cards_to_combine;
    bool legendary_found(false);
    for(unsigned card_index: card_indices)
    {
        const Card* card(cards[card_index]);
        if(card->m_unique || card->m_rarity == 4)
        {
            if(card->m_rarity == 4)
            {
                if(legendary_found) { return; }
                legendary_found = true;
            }
            --num_cards_to_combine;
            unique_cards.push_back(card);
        }
        else
        {
            cards_to_combine.push_back(card);
        }
    }
    // all unique or legendaries, quit
    if(cards_to_combine.size() == 0) { return; }
    if(cards_to_combine.size() == 1)
    {
        std::vector<const Card*> deck_cards = unique_cards;
        std::vector<const Card*> combined_cards(num_cards_to_combine, cards_to_combine[0]);
        deck_cards.insert(deck_cards.end(), combined_cards.begin(), combined_cards.end());
        DeckRandom deck(commander, deck_cards);
        (*dynamic_cast<DeckRandom*>(proc.att_deck)) = deck;
        auto new_results = proc.compare(num_iterations, best_score);
        double new_score = compute_score(new_results, proc.factors);
        if(new_score > best_score)
        {
            best_score = new_score;
            best_deck = deck;
            print_score_info(new_results, proc.factors);
            print_deck(deck);
            std::cout << std::flush;
        }
        //++num;
        // num_cards = num_cards_to_combine ...
    }
    else
    {
        var_k = cards_to_combine.size() - 1;
        Combination cardAmounts(num_cards_to_combine-1, var_k);
        bool finished(false);
        while(!finished)
        {
            const std::vector<unsigned>& indices = cardAmounts.getIndices();
            std::vector<unsigned> num_cards(var_k+1, 0);
            num_cards[0] = indices[0] + 1;
            for(unsigned i(1); i < var_k; ++i)
            {
                num_cards[i] = indices[i] - indices[i-1];
            }
            num_cards[var_k] = num_cards_to_combine - (indices[var_k-1] + 1);
            std::vector<const Card*> deck_cards = unique_cards;
            //std::cout << "num cards: ";
            for(unsigned num_index(0); num_index < num_cards.size(); ++num_index)
            {
                //std::cout << num_cards[num_index] << " ";
                for(unsigned i(0); i < num_cards[num_index]; ++i) { deck_cards.push_back(cards[card_indices[num_index]]); }
            }
            //std::cout << "\n" << std::flush;
            //std::cout << std::flush;
            assert(deck_cards.size() == deck_size);
            DeckRandom deck(commander, deck_cards);
            *proc.att_deck = deck;
            auto new_results = proc.compare(num_iterations, best_score);
            double new_score = compute_score(new_results, proc.factors);
            if(new_score > best_score)
            {
                best_score = new_score;
                best_deck = deck;
                print_score_info(new_results, proc.factors);
                print_deck(deck);
                std::cout << std::flush;
            }
            ++total_num_combinations_test;
            finished = cardAmounts.next();
        }
    }
}
//------------------------------------------------------------------------------
void exhaustive_k(unsigned num_iterations, unsigned var_k, Process& proc)
{
    std::vector<const Card*> ass_structs;
    for(const Card* card: proc.cards.player_assaults)
    {
        if(card->m_rarity >= 3) { ass_structs.push_back(card); }
    }
    for(const Card* card: proc.cards.player_structures)
    {
        if(card->m_rarity >= 3) { ass_structs.push_back(card); }
    }
    //std::vector<Card*> ass_structs; = cards.player_assaults;
    //ass_structs.insert(ass_structs.end(), cards.player_structures.begin(), cards.player_structures.end());
    unsigned var_n = ass_structs.size();
    assert(var_k <= var_n);
    unsigned num(0);
    Combination cardIndices(var_n, var_k);
    const std::vector<unsigned>& indices = cardIndices.getIndices();
    bool finished(false);
    double best_score{0};
    boost::optional<DeckRandom> best_deck;
    unsigned num_cards = ((DeckRandom*)proc.att_deck)->cards.size();
    while(!finished)
    {
        if(keep_commander)
        {
            try_all_ratio_combinations(num_cards, var_k, num_iterations, indices, ass_structs, ((DeckRandom*)proc.att_deck)->commander, proc, best_score, best_deck);
        }
        else
        {
            // Iterate over all commanders
            for(unsigned commanderIndex(0); commanderIndex < proc.cards.player_commanders.size() && !finished; ++commanderIndex)
            {
                const Card* commander(proc.cards.player_commanders[commanderIndex]);
                if(!suitable_commander(commander)) { continue; }
                try_all_ratio_combinations(num_cards, var_k, num_iterations, indices, ass_structs, commander, proc, best_score, best_deck);
            }
        }
        finished = cardIndices.next();
    }
    std::cout << "done " << num << "\n";
}
//------------------------------------------------------------------------------
enum Operation {
    bruteforce,
    climb,
    fightonce
};
//------------------------------------------------------------------------------
// void print_raid_deck(DeckRandom& deck)
// {
//         std::cout << "--------------- Raid ---------------\n";
//         std::cout << "Commander:\n";
//         std::cout << "  " << deck.m_commander->m_name << "\n";
//         std::cout << "Always include:\n";
//         for(auto& card: deck.m_cards)
//         {
//             std::cout << "  " << card->m_name << "\n";
//         }
//         for(auto& pool: deck.m_raid_cards)
//         {
//             std::cout << pool.first << " from:\n";
//             for(auto& card: pool.second)
//             {
//                 std::cout << "  " << card->m_name << "\n";
//             }
//         }
// }
//------------------------------------------------------------------------------
void print_available_decks(const Decks& decks)
{
    std::cout << "Mission decks:\n";
    for(auto it: decks.mission_decks_by_name)
    {
        std::cout << "  " << it.first << "\n";
    }
    std::cout << "Raid decks:\n";
    for(auto it: decks.raid_decks_by_name)
    {
        std::cout << "  " << it.first << "\n";
    }
    std::cout << "Custom decks:\n";
    for(auto it: decks.custom_decks)
    {
        std::cout << "  " << it.first << "\n";
    }
}

std::vector<std::pair<std::string, double> > parse_deck_list(std::string list_string)
{
    std::vector<std::pair<std::string, double> > res;
    boost::tokenizer<boost::char_delimiters_separator<char> > list_tokens{list_string, boost::char_delimiters_separator<char>{false, ";", ""}};
    for(auto list_token = list_tokens.begin(); list_token != list_tokens.end(); ++list_token)
    {
        boost::tokenizer<boost::char_delimiters_separator<char> > deck_tokens{*list_token, boost::char_delimiters_separator<char>{false, ":", ""}};
        auto deck_token = deck_tokens.begin();
        res.push_back(std::make_pair(*deck_token, 1.0d));
        ++deck_token;
        if(deck_token != deck_tokens.end())
        {
            res.back().second = boost::lexical_cast<double>(*deck_token);
        }
    }
    return(res);
}

void usage(int argc, char** argv)
{
    std::cout << "usage: " << argv[0] << " <attack deck> <defense decks list> [optional flags] [brute <num1> <num2>] [climb <num>]\n";
    std::cout << "\n";
    std::cout << "<attack deck>: the deck name of a custom deck.\n";
    std::cout << "<defense decks list>: semicolon separated list of defense decks, syntax:\n";
    std::cout << "  deckname1[:factor1];deckname2[:factor2];...\n";
    std::cout << "  where deckname is the name of a mission, raid, or custom deck, and factor is optional. The default factor is 1.\n";
    std::cout << "  example: \'fear:0.2;slowroll:0.8\' means fear is the defense deck 20% of the time, while slowroll is the defense deck 80% of the time.\n";
    std::cout << "\n";
    std::cout << "Flags:\n";
    std::cout << "  -c: don't try to optimize the commander.\n";
    std::cout << "  -o: restrict hill climbing to the owned cards listed in \"ownedcards.txt\".\n";
    std::cout << "  -r: the attack deck is played in order instead of randomly (respects the 3 cards drawn limit).\n";
    std::cout << "  -s: use surge (default is fight).\n";
    std::cout << "  -t <num>: set the number of threads, default is 4.\n";
    std::cout << "  -turnlimit <num>: set the number of turns in a battle, default is 50 (can be used for speedy achievements).\n";
    std::cout << "Operations:\n";
    std::cout << "brute <num1> <num2>: find the best combination of <num1> different cards, using up to <num2> battles to evaluate a deck.\n";
    std::cout << "climb <num>: perform hill-climbing starting from the given attack deck, using up to <num> battles to evaluate a deck.\n";
}

int main(int argc, char** argv)
{
    if(argc == 1) { usage(argc, argv); return(0); }
    debug_print = getenv("DEBUG_PRINT");
    unsigned num_threads = (debug_print || getenv("DEBUG")) ? 1 : 4;
    gamemode_t gamemode = fight;
    bool ordered = false;
    Cards cards;
    read_cards(cards);
    read_owned_cards(cards);
    Decks decks;
    load_decks_xml(decks, cards);
    load_decks(decks, cards);

    skill_table[augment] = perform_targetted_allied_fast<augment>;
    skill_table[augment_all] = perform_global_allied_fast<augment>;
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
    skill_table[rush] = perform_targetted_allied_fast<rush>;
    skill_table[shock] = perform_shock;
    skill_table[siege] = perform_targetted_hostile_fast<siege>;
    skill_table[siege_all] = perform_global_hostile_fast<siege>;
    skill_table[supply] = perform_supply;
    skill_table[strike] = perform_targetted_hostile_fast<strike>;
    skill_table[strike_all] = perform_global_hostile_fast<strike>;
    skill_table[summon] = perform_summon;
    skill_table[trigger_regen] = perform_trigger_regen;
    skill_table[weaken] = perform_targetted_hostile_fast<weaken>;
    skill_table[weaken_all] = perform_global_hostile_fast<weaken>;

    if(argc <= 2)
    {
        print_available_decks(decks);
        return(4);
    }
    std::string att_deck_name{argv[1]};
    std::vector<DeckIface*> def_decks;
    std::vector<double> def_decks_factors;
    auto deck_list_parsed = parse_deck_list(argv[2]);
    for(auto deck_parsed: deck_list_parsed)
    {
        DeckIface* def_deck = find_deck(decks, deck_parsed.first);
        if(def_deck != nullptr)
        {
            def_decks.push_back(def_deck);
            def_decks_factors.push_back(deck_parsed.second);
        }
        else
        {
            std::cout << "The deck " << deck_parsed.first << " was not found. Available decks:\n";
            print_available_decks(decks);
            return(5);
        }
    }
    std::vector<std::tuple<unsigned, unsigned, Operation> > todo;
    for(unsigned argIndex(3); argIndex < argc; ++argIndex)
    {
        if(strcmp(argv[argIndex], "-c") == 0)
        {
            keep_commander = true;
        }
        else if(strcmp(argv[argIndex], "-o") == 0)
        {
            use_owned_cards = true;
        }
        else if(strcmp(argv[argIndex], "-r") == 0)
        {
            ordered = true;
        }
        else if(strcmp(argv[argIndex], "-s") == 0)
        {
            gamemode = surge;
        }
        else if(strcmp(argv[argIndex], "-t") == 0)
        {
            num_threads = atoi(argv[argIndex+1]);
            argIndex += 1;
        }
        else if(strcmp(argv[argIndex], "-turnlimit") == 0)
        {
            turn_limit = atoi(argv[argIndex+1]);
            argIndex += 1;
        }
        else if(strcmp(argv[argIndex], "brute") == 0)
        {
            todo.push_back(std::make_tuple((unsigned)atoi(argv[argIndex+1]), (unsigned)atoi(argv[argIndex+2]), bruteforce));
            argIndex += 2;
        }
        else if(strcmp(argv[argIndex], "climb") == 0)
        {
            todo.push_back(std::make_tuple((unsigned)atoi(argv[argIndex+1]), 0u, climb));
            argIndex += 1;
        }
        else if(strcmp(argv[argIndex], "debug") == 0)
        {
            debug_print = true;
            num_threads = 1;
            todo.push_back(std::make_tuple(0u, 0u, fightonce));
        }
    }

    unsigned attacker_wins(0);
    unsigned defender_wins(0);

    DeckIface* att_deck{nullptr};
    auto custom_deck_it = decks.custom_decks.find(att_deck_name);
    if(custom_deck_it != decks.custom_decks.end())
    {
        att_deck = custom_deck_it->second;
    }
    else
    {
        std::cout << "The deck " << att_deck_name << " was not found. Available decks:\n";
        std::cout << "Custom decks:\n";
        for(auto it: decks.custom_decks)
        {
            std::cout << "  " << it.first << "\n";
        }
        return(5);
    }
    print_deck(*att_deck);

    std::shared_ptr<DeckOrdered> att_deck_ordered;
    if(ordered)
    {
        att_deck_ordered = std::make_shared<DeckOrdered>(*att_deck);
    }

    Process p(num_threads, cards, decks, ordered ? att_deck_ordered.get() : att_deck, def_decks, def_decks_factors, gamemode);
    {
        //ScopeClock timer;
        for(auto op: todo)
        {
            switch(std::get<2>(op))
            {
            case bruteforce: {
                exhaustive_k(std::get<1>(op), std::get<0>(op), p);
                break;
            }
            case climb: {
                if(!ordered)
                {
                    hill_climbing(std::get<0>(op), att_deck, p);
                }
                else
                {
                    hill_climbing_ordered(std::get<0>(op), att_deck_ordered.get(), p);
                }
                break;
            }
            case fightonce: {
                p.evaluate(1);
                break;
            }
            }
        }
    }
    return(0);
}
