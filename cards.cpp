#include "cards.h"

#include <map>
#include <sstream>
#include <stdexcept>

#include "tyrant.h"

template<typename T>
std::string to_string(T val)
{
    std::stringstream s;
    s << val;
    return s.str();
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
            }
            if(player_cards_by_name.find(card->m_name) != player_cards_by_name.end())
            {
                throw std::runtime_error("While trying to insert the card [" + card->m_name + ", id " + to_string(card->m_id) + "] in the player_cards_by_name map: the key already exists [id " + to_string(player_cards_by_name[card->m_name]->m_id) + "].");
            }
            else
            {
                player_cards_by_name[card->m_name] = card;
            }
        }
    }
}
