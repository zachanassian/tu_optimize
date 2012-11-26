#ifndef DECK_H_INCLUDED
#define DECK_H_INCLUDED

#include <deque>
#include <list>
#include <map>
#include <random>
#include <vector>

class Card;
class Cards;

//---------------------- $30 Deck: a commander + a sequence of cards -----------
// Can be shuffled.
// Implementations: random player and raid decks, ordered player decks.
//------------------------------------------------------------------------------
struct DeckIface
{
    const Card* commander;
    std::vector<const Card*> cards;

    DeckIface() :
        commander{nullptr}
    {}

    DeckIface(const Card* commander_,
              std::vector<const Card*> cards_) :
        commander(commander_),
        cards(std::begin(cards_), std::end(cards_))
    {}
    ;
    virtual ~DeckIface() {};
    virtual DeckIface* clone() const = 0;
    virtual const Card* get_commander() = 0;
    virtual const Card* next() = 0;
    virtual void shuffle(std::mt19937& re) = 0;
    // Special case for recharge (behemoth raid's ability).
    virtual void place_at_bottom(const Card*) = 0;
};
//------------------------------------------------------------------------------
struct DeckRandom : DeckIface
{
    std::vector<std::pair<unsigned, std::vector<const Card*> > > raid_cards;
    std::deque<const Card*> shuffled_cards;

    DeckRandom(
        const Card* commander_,
        const std::vector<const Card*>& cards_,
        std::vector<std::pair<unsigned, std::vector<const Card*> > > raid_cards_ =
        std::vector<std::pair<unsigned, std::vector<const Card*> > >()) :
        DeckIface(commander_, cards_),
        raid_cards(raid_cards_)
    {
    }

    DeckRandom(const DeckIface& other) :
        DeckIface(other)
    {
    }

    DeckRandom(const Cards& all_cards, const std::vector<std::string>& names);
    DeckRandom(const Cards& all_cards, const std::vector<unsigned>& ids);

    ~DeckRandom() {}

    virtual DeckIface* clone() const;
    const Card* get_commander();
    const Card* next();
    void shuffle(std::mt19937& re);
    void place_at_bottom(const Card* card);
};
//------------------------------------------------------------------------------
// No support for ordered raid decks
struct DeckOrdered : DeckIface
{
    std::deque<const Card*> shuffled_cards;
    // card id -> card order
    std::map<unsigned, std::list<unsigned> > order;

    DeckOrdered(const Card* commander_, std::vector<const Card*> cards_) :
        DeckIface(commander_, cards_),
        shuffled_cards(cards.begin(), cards.end())
    {
    }

    DeckOrdered(const DeckIface& other) :
        DeckIface(other)
    {
    }

    ~DeckOrdered() {}

    virtual DeckOrdered* clone() const;
    const Card* get_commander();
    const Card* next();
    void shuffle(std::mt19937& re);
    void place_at_bottom(const Card* card);
};

// + also the custom decks
struct Decks
{
    std::map<std::string, DeckIface*> custom_decks;
    std::list<DeckRandom> mission_decks;
    std::map<unsigned, DeckRandom*> mission_decks_by_id;
    std::map<std::string, DeckRandom*> mission_decks_by_name;
    std::list<DeckRandom> raid_decks;
    std::map<unsigned, DeckRandom*> raid_decks_by_id;
    std::map<std::string, DeckRandom*> raid_decks_by_name;
    std::list<DeckRandom> quest_decks;
    std::map<unsigned, DeckRandom*> quest_decks_by_id;
    std::map<std::string, DeckRandom*> quest_decks_by_name;
    std::map<unsigned, unsigned> quest_effects_by_id;
    std::map<std::string, unsigned> quest_effects_by_name;

    ~Decks()
    {
        for(auto& obj: custom_decks)
        {
            delete(obj.second);
        }
    }
};

#endif
