#ifndef DECK_H_INCLUDED
#define DECK_H_INCLUDED

#include <boost/utility.hpp> // because of 1.51 bug. missing include in range/any_range.hpp ?
#include <boost/range/algorithm_ext/insert.hpp>
#include <boost/range/any_range.hpp>
#include <deque>
#include <list>
#include <vector>

#include "cards.h"

class Card;

template<class RandomAccessIterator, class UniformRandomNumberGenerator>
void partial_shuffle(RandomAccessIterator first, RandomAccessIterator middle,
                     RandomAccessIterator last,
                     UniformRandomNumberGenerator&& g)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type diff_t;
    typedef typename std::make_unsigned<diff_t>::type udiff_t;
    typedef typename std::uniform_int_distribution<udiff_t> distr_t;
    typedef typename distr_t::param_type param_t;

    distr_t D;
    diff_t m = middle - first;
    diff_t n = last - first;
    for (diff_t i = 0; i < m; ++i)
    {
        std::swap(first[i], first[D(g, param_t(i, n-1))]);
    }
}

namespace boost
{
namespace range
{
template<typename Range, typename UniformRandomNumberGenerator>
void shuffle(Range& range, UniformRandomNumberGenerator&& rand)
{
    std::shuffle(boost::begin(range), boost::end(range), rand);
}
} // namespace range
using range::shuffle;
} // namespace boost

namespace range = boost::range;
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
              boost::any_range<const Card*, boost::forward_traversal_tag, const Card*, std::ptrdiff_t> cards_) :
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

    DeckRandom(const Cards& all_cards, const std::vector<std::string>& names)
    {
        for(auto name: names)
        {
            auto card_it(all_cards.player_cards_by_name.find(name));
            if(card_it == all_cards.player_cards_by_name.end())
            {
                throw std::runtime_error("While constructing a deck: the card " + name + " was not found.");
            }
            else
            {
                const Card* card{card_it->second};
                if(card->m_type == CardType::commander)
                {
                    if(commander == nullptr)
                    {
                        commander = card;
                    }
                    else
                    {
                        throw std::runtime_error("While constructing a deck: two commanders detected (" + name + " and " + commander->m_name + ")");
                    }
                }
                else
                {
                    cards.emplace_back(card);
                }
            }
        }
        if(commander == nullptr)
        {
            throw std::runtime_error("While constructing a deck: no commander found");
        }
    }

    DeckRandom(const Cards& all_cards, const std::vector<unsigned>& ids)
    {
        for(auto id: ids)
        {
            const Card* card{all_cards.by_id(id)};
            if(card->m_type == CardType::commander)
            {
                if(commander == nullptr)
                {
                    commander = card;
                }
                else
                {
                    throw std::runtime_error("While constructing a deck: two commanders detected (" + card->m_name + " and " + commander->m_name + ")");
                }
            }
            else
            {
                cards.emplace_back(card);
            }
        }
        if(commander == nullptr)
        {
            throw std::runtime_error("While constructing a deck: no commander found");
        }
    }

    ~DeckRandom() {}

    virtual DeckIface* clone() const
    {
        return(new DeckRandom(*this));
    }

    const Card* get_commander()
    {
        return(commander);
    }

    const Card* next()
    {
        if(!shuffled_cards.empty())
        {
            const Card* card = shuffled_cards.front();
            shuffled_cards.pop_front();
            return(card);
        }
        else
        {
            return(nullptr);
        }
    }

    void shuffle(std::mt19937& re)
    {
        shuffled_cards.clear();
        boost::insert(shuffled_cards, shuffled_cards.end(), cards);
        for(auto& card_pool: raid_cards)
        {
            assert(card_pool.first <= card_pool.second.size());
            partial_shuffle(card_pool.second.begin(), card_pool.second.begin() + card_pool.first, card_pool.second.end(), re);
            shuffled_cards.insert(shuffled_cards.end(), card_pool.second.begin(), card_pool.second.begin() + card_pool.first);
        }
        boost::shuffle(shuffled_cards, re);
    }

    void place_at_bottom(const Card* card)
    {
        shuffled_cards.push_back(card);
    }
};
//------------------------------------------------------------------------------
// No support for ordered raid decks
struct DeckOrdered : DeckIface
{
    std::deque<const Card*> shuffled_cards;
    // card id -> card order
    std::map<unsigned, std::list<unsigned> > order;

    DeckOrdered(const Card* commander_, boost::any_range<const Card*, boost::forward_traversal_tag, const Card*, std::ptrdiff_t> cards_) :
        DeckIface(commander_, cards_),
        shuffled_cards(cards.begin(), cards.end())
    {
    }

    DeckOrdered(const DeckIface& other) :
        DeckIface(other)
    {
    }

    ~DeckOrdered() {}

    virtual DeckOrdered* clone() const
    {
        return(new DeckOrdered(*this));
    }

    const Card* get_commander() { return(commander); }

    const Card* next()
    {
        if(shuffled_cards.empty())
        {
            return(nullptr);
        }
        else
        {
            auto cardIter = std::min_element(shuffled_cards.begin(), shuffled_cards.begin() + std::min<unsigned>(3u, shuffled_cards.size()), [this](const Card* card1, const Card* card2) -> bool
                                             {
                                                 auto card1_order = order.find(card1->m_id);
                                                 if(!card1_order->second.empty())
                                                 {
                                                     auto card2_order = order.find(card2->m_id);
                                                     if(!card1_order->second.empty())
                                                     {
                                                         return(*card1_order->second.begin() < *card2_order->second.begin());
                                                     }
                                                     else
                                                     {
                                                         return(true);
                                                     }
                                                 }
                                                 else
                                                 {
                                                     return(false);
                                                 }
                                             });
            auto card = *cardIter;
            shuffled_cards.erase(cardIter);
            auto card_order = order.find(card->m_id);
            if(!card_order->second.empty())
            {
                card_order->second.erase(card_order->second.begin());
            }
            return(card);
        }
    }

    void shuffle(std::mt19937& re)
    {
        unsigned i = 0;
        order.clear();
        for(auto card: cards)
        {
            order[card->m_id].push_back(i);
            ++i;
        }
        shuffled_cards.clear();
        range::insert(shuffled_cards, shuffled_cards.end(), cards);
        std::shuffle(shuffled_cards.begin(), shuffled_cards.end(), re);
    }

    void place_at_bottom(const Card* card)
    {
        shuffled_cards.push_back(card);
    }
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

    ~Decks()
    {
        for(auto& obj: custom_decks)
        {
            delete(obj.second);
        }
    }
};

#endif
