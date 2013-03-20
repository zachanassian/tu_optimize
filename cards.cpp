#include "cards.h"

#include <boost/tokenizer.hpp>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <list>

#include "card.h"
#include "tyrant.h"

template<typename T>
std::string to_string(T val)
{
    std::stringstream s;
    s << val;
    return s.str();
}

std::string simplify_name(const std::string& card_name)
{
    std::string simple_name;
    for(auto c : card_name)
    {
        if(!strchr(";:, \"'-", c))
        {
            simple_name += ::tolower(c);
        }
    }
    return(simple_name);
}

std::list<std::string> get_abbreviations(const std::string& name)
{
    std::list<std::string> abbr_list;
    boost::tokenizer<boost::char_delimiters_separator<char> > word_token{name, boost::char_delimiters_separator<char>{false, " ", ""}};
    std::string initial;
    auto token_iter = word_token.begin();
    for(; token_iter != word_token.end(); ++token_iter)
    {
        abbr_list.push_back(simplify_name(std::string{token_iter->begin(), token_iter->end()}));
        initial += *token_iter->begin();
    }
    abbr_list.push_back(simplify_name(initial));
    return(abbr_list);
}

//------------------------------------------------------------------------------
Cards::~Cards()
{
    for(Card* c: cards) { delete(c); }
}

const Card* Cards::by_id(unsigned id) const
{
    std::map<unsigned, Card*>::const_iterator cardIter{cards_by_id.find(id)};
    if(cardIter == cards_by_id.end())
    {
        throw std::runtime_error("While trying to find the card with id " + to_string(id) + ": no such key in the cards_by_id map.");
    }
    else
    {
        return(cardIter->second);
    }
}
//------------------------------------------------------------------------------
void Cards::organize()
{
    cards_by_id.clear();
    player_cards.clear();
    player_cards_by_name.clear();
    player_commanders.clear();
    player_assaults.clear();
    player_structures.clear();
    player_actions.clear();
    for(Card* card: cards)
    {
        // Remove delimiters from card names
        size_t pos;
        while((pos = card->m_name.find_first_of(";:,")) != std::string::npos)
        {
            card->m_name.erase(pos, 1);
        }
        if(card->m_set == 5002)
        {
            card->m_name += '*';
        }
        cards_by_id[card->m_id] = card;
        // Card available to players
        if(card->m_set != -1)
        {
            player_cards.push_back(card);
            switch(card->m_type)
            {
            case CardType::commander: {
                player_commanders.push_back(card);
                break;
            }
            case CardType::assault: {
                player_assaults.push_back(card);
                break;
            }
            case CardType::structure: {
                player_structures.push_back(card);
                break;
            }
            case CardType::action: {
                player_actions.push_back(card);
                break;
            }
            case CardType::num_cardtypes: {
                throw card->m_type;
                break;
            }
            }
            std::string simple_name{simplify_name(card->m_name)};
            if(player_cards_by_name.find(simple_name) == player_cards_by_name.end())
            {
                player_cards_by_name[simple_name] = card;
            }
        }
    }
    for(Card* card: cards)
    {
        // generate abbreviations
        for(auto&& abbr_name : get_abbreviations(card->m_name))
        {
            if(abbr_name.length() > 1 && player_cards_by_name.find(abbr_name) == player_cards_by_name.end())
            {
                player_cards_abbr[abbr_name] = card->m_name;
            }
        }
        // update base_id
        std::string base_name{simplify_name(card->m_name)};
        if(card->m_set == 5002)
        {
            base_name.erase(base_name.size() - 1);
        }
        auto card_itr = player_cards_by_name.find(base_name);
        if(card_itr != player_cards_by_name.end() && card_itr->second != card)
        {
            card->m_base_id = card_itr->second->m_id;
        }
    }
}
