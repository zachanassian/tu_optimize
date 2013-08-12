#ifndef DECK_H_INCLUDED
#define DECK_H_INCLUDED

#include <deque>
#include <list>
#include <map>
#include <random>
#include <vector>
#include "tyrant.h"
#include "card.h"

class Cards;

std::string deck_hash(const Card* commander, std::vector<const Card*> cards, bool is_ordered);

//---------------------- $30 Deck: a commander + a sequence of cards -----------
// Can be shuffled.
// Implementations: random player and raid decks, ordered player decks.
//------------------------------------------------------------------------------
namespace DeckStrategy
{
enum DeckStrategy
{
    random,
    ordered,
    exact_ordered,
    num_deckstrategies
};
}
//------------------------------------------------------------------------------
// No support for ordered raid decks
struct Deck
{
    DeckType::DeckType decktype;
    unsigned id;
    std::string name;
    DeckStrategy::DeckStrategy strategy;
    Effect effect; // for quests

    const Card* commander;
    std::vector<const Card*> cards;

    std::map<signed, char> card_marks;  // <positions of card, prefix mark>: -1 indicating the commander. E.g, used as a mark to be kept in attacking deck when optimizing.
    std::deque<const Card*> shuffled_cards;
    // card id -> card order
    std::map<unsigned, std::list<unsigned>> order;
    std::vector<std::pair<unsigned, std::vector<const Card*>>> raid_cards;
    std::vector<const Card*> reward_cards;
    unsigned mission_req;

    std::string deck_string;
    std::vector<unsigned> given_hand;

    Deck(
        DeckType::DeckType decktype_ = DeckType::deck,
        unsigned id_ = 0,
        std::string name_ = "",
        DeckStrategy::DeckStrategy strategy_ = DeckStrategy::random) :
        decktype(decktype_),
        id(id_),
        name(name_),
        strategy(strategy_),
        effect(Effect::none),
        commander(nullptr),
        mission_req(0)
    {
    }

    ~Deck() {}

    void set(
        const Card* commander_,
        const std::vector<const Card*>& cards_,
        std::vector<std::pair<unsigned, std::vector<const Card*>>> raid_cards_ = {},
        std::vector<const Card*> reward_cards_ = {},
        unsigned mission_req_ = 0)
    {
        commander = commander_;
        cards = std::vector<const Card*>(std::begin(cards_), std::end(cards_));
        raid_cards = std::vector<std::pair<unsigned, std::vector<const Card*>>>(raid_cards_);
        reward_cards = std::vector<const Card*>(reward_cards_);
        mission_req = mission_req_;
    }

    void set(const Cards& all_cards, const std::vector<unsigned>& ids, const std::map<signed, char> marks = {});
    void set(const Cards& all_cards, const std::string& deck_string_);
    void resolve(const Cards& all_cards);
    void set_given_hand(const Cards& all_cards, const std::string& deck_string_);

    template<class Container>
    Container card_ids() const
    {
        Container results;
        results.insert(results.end(), commander->m_id);
        for(auto card: cards)
        {
            results.insert(results.end(), card->m_id);
        }
        return(results);
    }

    Deck* clone() const;
    std::string short_description() const;
    std::string long_description(const Cards& all_cards) const;
    const Card* get_commander();
    const Card* next();
    void shuffle(std::mt19937& re);
    void place_at_bottom(const Card* card);
};

// + also the custom decks
struct Decks
{
    std::list<Deck> decks;
    std::map<std::pair<DeckType::DeckType, unsigned>, Deck*> by_type_id;
    std::map<std::string, Deck*> by_name;
    std::map<std::string, std::string> custom_decks;
};

#endif
