#include "deck.h"

#include <boost/range/algorithm_ext/insert.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "card.h"
#include "cards.h"
#include "read.h"

DeckEncoding::DeckEncoding deck_encoding(DeckEncoding::wmt_b64);

template<typename T>
std::string to_string(T val)
{
    std::stringstream s;
    s << val;
    return s.str();
}

std::map<signed, char> empty_marks;

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

void append_wmt_b64_card_id(std::stringstream& ios, unsigned card_id, bool repeat=false) {
/*
enhancements for card_id > 4000 magic characters "-.~!*"
card_id
    0 -  4000: card_id encoded as two letter base64
 4001 -  8000: -(card_id-4000) encoded as two letter base64
 8001 - 12000: .(card_id-8000) encoded as two letter base64
12001 - 16000: ~(card_id-12000) encoded as two letter base64
16001 - 20000: !(card_id-20000) encoded as two letter base64
20001 - 24000: *(card_id-4000) encoded as two letter base64

if there is a base64 encoded two letter value > 4000 this means (value-4001) copied of last card
*/

    std::string base64= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string magic_char= " -.~!*";

    if(card_id > 24000)
    {
        throw std::runtime_error("Error for card [" + to_string(card_id) + "]. This deck encoding does not support card_ids greater then 24000.");
    }
    else if(!repeat && card_id > 4000)
    {
        ios << magic_char[(card_id-1) / 4000]; //8000,12000 still gets index 1
        card_id = card_id % 4000;
        if(card_id == 0){ card_id = 4000; } ////8000,12000 are encoded as 4000 (+g)
    }
    ios << base64[card_id / 64];
    ios << base64[card_id % 64];
}

//------------------------------------------------------------------------------
std::string deck_hash_wmt_b64(const Card* commander, std::vector<const Card*> cards, bool is_ordered)
{
    std::stringstream ios;
    // with custom cards, commander ids can be > 4000
    append_wmt_b64_card_id(ios, commander->m_id);
    if(!is_ordered)
    {
        std::sort(cards.begin(), cards.end(), [](const Card* a, const Card* b) { return a->m_id < b->m_id; });
    }
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
                append_wmt_b64_card_id(ios, num_repeat + 4000, true);
            }
            last_id = card_id;
            num_repeat = 1;
            append_wmt_b64_card_id(ios, card_id);
        }
    }
    if(num_repeat > 1)
    {
        append_wmt_b64_card_id(ios, num_repeat + 4000, true);
    }
    return ios.str();
}

void encode_ddd_b64(std::stringstream &ios, unsigned card_id)
{
    if(card_id >= 262144) //64^3
    {
        throw std::runtime_error("Error for card [" + to_string(card_id) + "]. This deck encoding does not support card_ids greater then 262144.");
    }

    std::string base64= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    ios << base64[card_id / 4096];
    ios << base64[(card_id%4096) / 64];
    ios << base64[card_id % 64];
}

std::string deck_hash_ddd_b64(const Card* commander, std::vector<const Card*> cards, bool is_ordered)
{
/*
each card_id is stored as a three letter base64 value
*/
    std::stringstream ios;
    encode_ddd_b64(ios, commander->m_id);
    for(const Card* card: cards)
    {
        encode_ddd_b64(ios, card->m_id);
    }
    return ios.str();
}
//------------------------------------------------------------------------------
namespace {
const char* base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

const char* magic_chars = "-.~!*";
// Converts cards in `hash' to a deck.
// Stores resulting card IDs in `ids'.
void hash_to_ids_wmt_b64(const char* hash, std::vector<unsigned>& ids)
{
    unsigned int last_id = 0;
    const char* pc = hash;

    while(*pc)
    {
        unsigned id_plus = 0;
        const char* pmagic = strchr(magic_chars, *pc);
        if (pmagic)
        {
          ++pc;
          id_plus = 4000*(pmagic-magic_chars+1);
        }
        if(!*pc || !*(pc + 1))
        {
            throw std::runtime_error("Invalid hash length");
        }
        const char* p0 = strchr(base64_chars, *pc);
        const char* p1 = strchr(base64_chars, *(pc + 1));
        if (!p0 || !p1)
        {
            throw std::runtime_error("Invalid hash character");
        }
        pc += 2;
        size_t index0 = p0 - base64_chars;
        size_t index1 = p1 - base64_chars;
        unsigned int id = (index0 << 6) + index1;

        if (id < 4001)
        {
            id += id_plus;
            ids.push_back(id);
            last_id = id;
        }
        else for (unsigned int j = 0; j < id - 4001; ++j)
        {
            ids.push_back(last_id);
        }
    }
}

void hash_to_ids_ddd_b64(const char* hash, std::vector<unsigned>& ids)
{
    const char* pc = hash;

    while(*pc)
    {
        if(!*pc || !*(pc + 1) || !*(pc + 2))
        {
            throw std::runtime_error("Invalid hash length");
        }
        const char* p0 = strchr(base64_chars, *pc);
        const char* p1 = strchr(base64_chars, *(pc + 1));
        const char* p2 = strchr(base64_chars, *(pc + 2));
        if (!p0 || !p1 || !p2)
        {
            throw std::runtime_error("Invalid hash character");
        }
        pc += 3;
        size_t index0 = p0 - base64_chars;
        size_t index1 = p1 - base64_chars;
        size_t index2 = p2 - base64_chars;
        unsigned int id = (index0 << 12) + (index1 << 6) + index2;

        ids.push_back(id);
    }
}

void hash_to_ids(const char* hash, std::vector<unsigned>& ids)
{
    switch(deck_encoding)
    {
        case DeckEncoding::wmt_b64:
            hash_to_ids_wmt_b64(hash, ids);
            break;
        case DeckEncoding::ddd_b64:
            hash_to_ids_ddd_b64(hash, ids);
            break;
        default:
            throw std::runtime_error("Unsupported Deck Encoding");
    }
}

const std::pair<std::vector<unsigned>, std::map<signed, char>> string_to_ids(const Cards& all_cards, const std::string& deck_string, const std::string & description)
{
    std::vector<unsigned> card_ids;
    std::map<signed, char> card_marks;
    if(deck_string.find_first_of(":,") == std::string::npos)
    {
        try
        {
            hash_to_ids(deck_string.c_str(), card_ids);
        }
        catch(std::exception& e)
        {
            std::cerr << "Error while resolving " << description << ": " << e.what() << std::endl;
            throw;
        }
    }
    else
    {
        boost::tokenizer<boost::char_delimiters_separator<char>> deck_tokens{deck_string, boost::char_delimiters_separator<char>{false, ":,", ""}};
        auto token_iter = deck_tokens.begin();
        signed p = -1;
        for(; token_iter != deck_tokens.end(); ++token_iter)
        {
            std::string card_spec(*token_iter);
            unsigned card_id{0};
            unsigned card_num{1};
            char num_sign{0};
            char mark{0};
            try
            {
                parse_card_spec(all_cards, card_spec, card_id, card_num, num_sign, mark);
                assert(num_sign == 0);
                for(unsigned i(0); i < card_num; ++i)
                {
                    card_ids.push_back(card_id);
                    if(mark) { card_marks[p] = mark; }
                    ++ p;
                }
            }
            catch(std::exception& e)
            {
                std::cerr << "Warning while resolving " << description << ": " << e.what() << std::endl;
                continue;
            }
        }
    }
    return {card_ids, card_marks};
}

} // end of namespace

std::string deck_hash(const Card* commander, std::vector<const Card*> cards, bool is_ordered)
{
    switch(deck_encoding)
    {
        case DeckEncoding::wmt_b64:
            return deck_hash_wmt_b64(commander, cards, is_ordered);
            break;
        case DeckEncoding::ddd_b64:
            return deck_hash_ddd_b64(commander, cards, is_ordered);
            break;
        default:
            throw std::runtime_error("Unsupported Deck Encoding");
    }
}

namespace range = boost::range;

void Deck::set(const Cards& all_cards, const std::vector<unsigned>& ids, const std::map<signed, char> &marks)
{
    commander = nullptr;
    //fortress modification
    fortress1 = nullptr;
    fortress2 = nullptr;
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
        else if(card->m_fortress > 0)
        {
            if(fortress1 == nullptr)
            {
                fortress1 = card;
            }
            else if(fortress2 == nullptr)
            {
                fortress2 = card;
            }
            else
            {
                throw std::runtime_error("While constructing a deck: three fortress cards detected (" + card->m_name + ", " + fortress1->m_name + " and " + fortress2->m_name + ")");
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
    card_marks = marks;
}

void Deck::set(const Cards& all_cards, const std::string& deck_string_)
{
    deck_string = deck_string_;
}

void Deck::resolve(const Cards& all_cards)
{
    if(commander != nullptr)
    {
        return;
    }
    auto && id_marks = string_to_ids(all_cards, deck_string, short_description());
    set(all_cards, id_marks.first, id_marks.second);
    deck_string.clear();
}

void Deck::set_given_hand(const Cards& all_cards, const std::string& hand_string)
{
    auto && id_marks = string_to_ids(all_cards, hand_string, "hand");
    given_hand = id_marks.first;
}

std::string Deck::short_description() const
{
    std::stringstream ios;
    ios << decktype_names[decktype];
    if(id > 0) { ios << " #" << id; }
    if(!name.empty()) { ios << " \"" << name << "\""; }
    if(deck_string.empty())
    {
        if(raid_cards.empty()) { ios << ": " << deck_hash(commander, cards, strategy == DeckStrategy::ordered || strategy == DeckStrategy::exact_ordered); }
    }
    else
    {
        ios << ": " << deck_string;
    }
    return ios.str();
}

extern std::string card_description(const Cards& cards, const Card* c);

std::string Deck::long_description(const Cards& all_cards) const
{
    std::stringstream ios;
    ios << short_description() << std::endl;
    if(effect != Effect::none)
    {
        ios << "Effect: " << effect_names[effect] << "\n";
    }
    if(commander)
    {
        ios << card_description(all_cards, commander) << "\n";
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
        ios << "  " << card_description(all_cards, card) << "\n";
    }
    for(auto& pool: raid_cards)
    {
        ios << pool.first << " from:\n";
        for(auto& card: pool.second)
        {
            ios << "  " << card_description(all_cards, card) << "\n";
        }
    }
    return ios.str();
}

std::string Deck::medium_description() const
{
    std::stringstream ios;
    ios << "[" << deck_hash(commander, cards, strategy == DeckStrategy::ordered || strategy == DeckStrategy::exact_ordered) << "] ";
    if(commander)
    {
        ios << commander->m_name;
    }
    else
    {
        ios << "No commander";
    }

    if(!cards.empty() && !raid_cards.empty())
    {
        ios << "Always include: ";
    }
    for(const Card* card: cards)
    {
        ios << ", " << card->m_name;
    }
    for(auto& pool: raid_cards)
    {
        ios << pool.first << " from: ";
        for(auto& card: pool.second)
        {
            ios << ", " << card->m_name;
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

const Card* Deck::get_fortress1()
{
    return(fortress1);
}
void Deck::set_fortress1(const Card* card)
{
    fortress1 = card;
}

const Card* Deck::get_fortress2()
{
    return(fortress2);
}
void Deck::set_fortress2(const Card* card)
{
    fortress2 = card;
}

const Card* Deck::next()
{
    if(shuffled_cards.empty())
    {
        return(nullptr);
    }
    else if(strategy == DeckStrategy::random || strategy == DeckStrategy::exact_ordered)
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
    }
    if(strategy != DeckStrategy::exact_ordered)
    {
        auto shufflable_iter = shuffled_cards.begin();
        for(auto hand_card_id: given_hand)
        {
            auto it = std::find_if(shufflable_iter, shuffled_cards.end(), [hand_card_id](const Card* card) -> bool { return card->m_id == hand_card_id; });
            if(it != shuffled_cards.end())
            {
                std::swap(*shufflable_iter, *it);
                ++ shufflable_iter;
            }
        }
        std::shuffle(shufflable_iter, shuffled_cards.end(), re);
#if 0
        if(!given_hand.empty())
        {
            for(auto card: cards) std::cout << ", " << card->m_name;
            std::cout << std::endl;
            std::cout << strategy;
            for(auto card: shuffled_cards) std::cout << ", " << card->m_name;
            std::cout << std::endl;
        }
#endif
    }
}

void Deck::place_at_bottom(const Card* card)
{
    shuffled_cards.push_back(card);
}

