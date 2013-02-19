#ifndef DECK_H_INCLUDED
#define DECK_H_INCLUDED

#include <deque>
#include <list>
#include <map>
#include <random>
#include <vector>
#include "tyrant.h"

class Card;
class Cards;

std::string deck_hash(const Card* commander, const std::vector<const Card*>& cards);

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

    std::deque<const Card*> shuffled_cards;
    // card id -> card order
    std::map<unsigned, std::list<unsigned> > order;
    std::vector<std::pair<unsigned, std::vector<const Card*> > > raid_cards;

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
        commander(nullptr)
    {
    }

    ~Deck() {}

    void set(
        const Card* commander_,
        const std::vector<const Card*>& cards_,
        std::vector<std::pair<unsigned, std::vector<const Card*> > > raid_cards_ =
        std::vector<std::pair<unsigned, std::vector<const Card*> > >())
    {
        commander = commander_;
//        cards = cards_;
//        raid_cards = raid_cards_;
        cards = std::vector<const Card*>(std::begin(cards_), std::end(cards_));
        raid_cards = std::vector<std::pair<unsigned, std::vector<const Card*> > >(raid_cards_);
    }

    void set(const Cards& all_cards, const std::vector<std::string>& names);
    void set(const Cards& all_cards, const std::vector<unsigned>& ids);

    Deck* clone() const;
    std::string short_description() const;
    std::string long_description() const;
    const Card* get_commander();
    const Card* next();
    void shuffle(std::mt19937& re);
    void place_at_bottom(const Card* card);
};

// + also the custom decks
struct Decks
{
    std::map<std::string, Deck*> custom_decks;
    std::list<Deck> mission_decks;
    std::map<unsigned, Deck*> mission_decks_by_id;
    std::map<std::string, Deck*> mission_decks_by_name;
    std::list<Deck> raid_decks;
    std::map<unsigned, Deck*> raid_decks_by_id;
    std::map<std::string, Deck*> raid_decks_by_name;
    std::list<Deck> quest_decks;
    std::map<unsigned, Deck*> quest_decks_by_id;
    std::map<std::string, Deck*> quest_decks_by_name;

    ~Decks()
    {
        for(auto& obj: custom_decks)
        {
            delete(obj.second);
        }
    }
};

#endif
