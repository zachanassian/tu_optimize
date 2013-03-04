#include "deck.h"

#include <boost/range/algorithm_ext/insert.hpp>
#include <sstream>
#include <stdexcept>

#include "card.h"
#include "cards.h"

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

//------------------------------------------------------------------------------
std::string deck_hash(const Card* commander, const std::vector<const Card*>& cards)
{
    std::string base64= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::stringstream ios;
    ios << base64[commander->m_id / 64];
    ios << base64[commander->m_id % 64];
    unsigned last_id = 0;
    unsigned num_repeat = 0;
    for(const Card* card: cards)
    {
        unsigned card_id(card->m_id);
        if(card_id == last_id)
        {
            ++ num_repeat;
        }
        else
        {
            if(num_repeat > 1)
            {
                ios << base64[(num_repeat + 4000) / 64];
                ios << base64[(num_repeat + 4000) % 64];
            }
            last_id = card_id;
            num_repeat = 1;
            if(card_id > 4000)
            {
                ios << '-';
                card_id -= 4000;
            }
            ios << base64[card_id / 64];
            ios << base64[card_id % 64];
        }
    }
    if(num_repeat > 1)
    {
        ios << base64[(num_repeat + 4000) / 64];
        ios << base64[(num_repeat + 4000) % 64];
    }
    return ios.str();
}
namespace range = boost::range;

void Deck::set(const Cards& all_cards, const std::vector<std::string>& names)
{
    commander = nullptr;
    strategy = DeckStrategy::random;
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

void Deck::set(const Cards& all_cards, const std::vector<unsigned>& ids)
{
    commander = nullptr;
    strategy = DeckStrategy::random;
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

std::string Deck::short_description() const
{
    std::stringstream ios;
    ios << decktype_names[decktype];
    if(id > 0) { ios << " " << id; }
    if(!name.empty()) { ios << " \"" << name << "\""; }
    if(raid_cards.empty()) { ios << ": " << deck_hash(commander, cards); }
    return ios.str();
}

std::string Deck::long_description() const
{
    std::stringstream ios;
    ios << short_description() << std::endl;
    if(effect != Effect::none)
    {
        ios << "Effect: " << effect_names[effect] << "\n";
    }
    if(commander)
    {
        ios << commander->m_name << "\n";
    }
    else
    {
        ios << "No commander\n";
    }
    if(!cards.empty() && !raid_cards.empty())
    {
        ios << "Always include:\n";
    }
    for(const Card* card: cards)
    {
        ios << "  " << card->m_name << std::endl;
    }
    for(auto& pool: raid_cards)
    {
        ios << pool.first << " from:\n";
        for(auto& card: pool.second)
        {
            ios << "  " << card->m_name << "\n";
        }
    }
    return ios.str();
}

Deck* Deck::clone() const
{
    return(new Deck(*this));
}


const Card* Deck::get_commander()
{
    return(commander);
}

const Card* Deck::next()
{
    if(shuffled_cards.empty())
    {
        return(nullptr);
    }
    else if(strategy == DeckStrategy::random)
    {
        const Card* card = shuffled_cards.front();
        shuffled_cards.pop_front();
        return(card);
    }
    else if(strategy == DeckStrategy::ordered)
    {
        auto cardIter = std::min_element(shuffled_cards.begin(), shuffled_cards.begin() + std::min<unsigned>(3u, shuffled_cards.size()), [this](const Card* card1, const Card* card2) -> bool
                                         {
                                             auto card1_order = order.find(card1->m_id);
                                             if(!card1_order->second.empty())
                                             {
                                                 auto card2_order = order.find(card2->m_id);
                                                 if(!card2_order->second.empty())
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
    throw std::runtime_error("Unknown strategy for deck.");
}

void Deck::shuffle(std::mt19937& re)
{
    shuffled_cards.clear();
    boost::insert(shuffled_cards, shuffled_cards.end(), cards);
    if(!raid_cards.empty())
    {
        if(strategy != DeckStrategy::random)
        {
            throw std::runtime_error("Support only random strategy for raid/quest deck.");
        }
        for(auto& card_pool: raid_cards)
        {
            assert(card_pool.first <= card_pool.second.size());
            partial_shuffle(card_pool.second.begin(), card_pool.second.begin() + card_pool.first, card_pool.second.end(), re);
            shuffled_cards.insert(shuffled_cards.end(), card_pool.second.begin(), card_pool.second.begin() + card_pool.first);
        }
    }
    if(strategy == DeckStrategy::ordered)
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
    }
    std::shuffle(shuffled_cards.begin(), shuffled_cards.end(), re);
}

void Deck::place_at_bottom(const Card* card)
{
    shuffled_cards.push_back(card);
}

