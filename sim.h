#ifndef SIM_H_INCLUDED
#define SIM_H_INCLUDED

#include <boost/pool/pool.hpp>
#include <string>
#include <array>
#include <deque>
#include <tuple>
#include <vector>

#include "tyrant.h"

class Card;
class Cards;
class Deck;
class Field;
class Achievement;

extern unsigned debug_print;
extern unsigned debug_cached;
extern bool debug_line;
extern std::string debug_str;
extern unsigned turn_limit;

inline unsigned safe_minus(unsigned x, unsigned y)
{
    return(x - std::min(x, y));
}

//---------------------- Represent Simulation Results ----------------------------
template<typename result_type>
struct Results
{
    result_type wins;
    result_type draws;
    result_type losses;
    result_type points;
    result_type sq_points;
    template<typename other_result_type>
    Results& operator+=(const Results<other_result_type>& other)
    {
        wins += other.wins;
        draws += other.draws;
        losses += other.losses;
        points += other.points;
        sq_points += other.points * other.points;
        return *this;
    }
};

void fill_skill_table();
Results<uint64_t> play(Field* fd);
void modify_cards(Cards& cards, enum Effect effect);
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
//------------------------------------------------------------------------------
enum class CardStep
{
    none,
    attacking,
    attacked,
};
//------------------------------------------------------------------------------
struct CardStatus
{
    const Card* m_card;
    unsigned m_index;
    unsigned m_player;
    unsigned m_augmented;
    unsigned m_berserk;
    bool m_blitzing;
    bool m_chaosed;
    unsigned m_delay;
    bool m_diseased;
    unsigned m_enfeebled;
    Faction m_faction;
    bool m_frozen;
    unsigned m_hp;
    bool m_immobilized;
    bool m_infused;
    bool m_jammed;
    bool m_phased;
    unsigned m_poisoned;
    unsigned m_protected;
    unsigned m_rallied;
    unsigned m_stunned;
    bool m_sundered;
    unsigned m_weakened;
    bool m_temporary_split;
    bool m_is_summoned; // is this card summoned (or split)?
    CardStep m_step;

    CardStatus() {}
    CardStatus(const Card* card);

    void set(const Card* card);
    void set(const Card& card);
    std::string description();
};
//------------------------------------------------------------------------------
// Represents a particular draw from a deck.
// Persistent object: call reset to get a new draw.
class Hand
{
public:

    Hand(Deck* deck_) :
        deck(deck_),
        assaults(15),
        structures(15)
    {
    }

    void reset(std::mt19937& re);

    Deck* deck;
    CardStatus commander;
    Storage<CardStatus> assaults;
    Storage<CardStatus> structures;
};
//------------------------------------------------------------------------------
// struct Field is the data model of a battle:
// an attacker and a defender deck, list of assaults and structures, etc.
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
    std::vector<CardStatus*> selection_array;
    unsigned turn;
    gamemode_t gamemode;
    OptimizationMode optimization_mode;
    const Effect effect;
    const Achievement& achievement;
    // With the introduction of on death skills, a single skill can trigger arbitrary many skills.
    // They are stored in this, and cleared after all have been performed.
    std::deque<std::tuple<CardStatus*, SkillSpec>> skill_queue;
    std::vector<CardStatus*> killed_with_on_death;
    std::vector<CardStatus*> killed_with_regen;
    enum phase
    {
        playcard_phase,
        legion_phase,
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
    unsigned last_decision_turn;
//    unsigned points_since_last_decision;

    unsigned fusion_count;
    std::vector<unsigned> achievement_counter;

    Field(std::mt19937& re_, const Cards& cards_, Hand& hand1, Hand& hand2, gamemode_t gamemode_, OptimizationMode optimization_mode_, Effect effect_, const Achievement& achievement_) :
        end{false},
        re(re_),
        cards(cards_),
        players{{&hand1, &hand2}},
        turn(1),
        gamemode(gamemode_),
        optimization_mode(optimization_mode_),
        effect(effect_),
        achievement(achievement_)
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
    inline T random_in_vector(const std::vector<T>& v)
    {
        assert(v.size() > 0);
        return(v[this->rand(0, v.size() - 1)]);
    }

    template <typename CardsIter, typename Functor>
    inline unsigned make_selection_array(CardsIter first, CardsIter last, Functor f);
    inline void print_selection_array();

    template <class T>
    inline void set_counter(T& container, unsigned key, unsigned value)
    {
        auto x = container.find(key);
        if(x != container.end())
        {
            achievement_counter[x->second] = value;
        }
    }

    template <class T>
    inline void inc_counter(T& container, unsigned key, unsigned value = 1)
    {
        auto x = container.find(key);
        if(x != container.end())
        {
            achievement_counter[x->second] += value;
#if 0
            if(achievement.req_counter[x->second].predict_monoinc(achievement_counter[x->second]) < 0)
            {
                end = true;
            }
#endif
        }
    }

    template <class T>
    inline void update_max_counter(T& container, unsigned key, unsigned value)
    {
        auto x = container.find(key);
        if(x != container.end() && achievement_counter[x->second] < value)
        {
            achievement_counter[x->second] = value;
#if 0
            if(achievement.req_counter[x->second].predict_monoinc(achievement_counter[x->second]) < 0)
            {
                end = true;
            }
#endif
        }
    }
};

#endif
