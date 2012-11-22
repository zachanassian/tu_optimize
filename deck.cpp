#include "deck.h"

#include <boost/range/algorithm_ext/insert.hpp>
#include <stdexcept>

#include "card.h"

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

DeckRandom::DeckRandom(const Cards& all_cards, const std::vector<std::string>& names)
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

DeckRandom::DeckRandom(const Cards& all_cards, const std::vector<unsigned>& ids)
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

DeckIface* DeckRandom::clone() const
{
    return(new DeckRandom(*this));
}

const Card* DeckRandom::get_commander()
{
    return(commander);
}

const Card* DeckRandom::next()
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

void DeckRandom::shuffle(std::mt19937& re)
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

void DeckRandom::place_at_bottom(const Card* card)
{
    shuffled_cards.push_back(card);
}

DeckOrdered* DeckOrdered::clone() const
{
    return(new DeckOrdered(*this));
}


const Card* DeckOrdered::get_commander()
{
    return(commander);
}

const Card* DeckOrdered::next()
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

void DeckOrdered::shuffle(std::mt19937& re)
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

void DeckOrdered::place_at_bottom(const Card* card)
{
    shuffled_cards.push_back(card);
}
