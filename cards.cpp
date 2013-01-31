#include "cards.h"

#include <map>
#include <sstream>
#include <stdexcept>

#include "card.h"
#include "tyrant.h"

template<typename T>
std::string to_string(T val)
{
    std::stringstream s;
    s << val;
    return s.str();
}

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
            player_cards_by_name[card->m_name] = card;
        }
    }
}
