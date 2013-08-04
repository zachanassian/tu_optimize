#ifndef CARDS_H_INCLUDED
#define CARDS_H_INCLUDED

#include <map>
#include <string>
#include <vector>

class Card;

struct Cards
{
    ~Cards();

    std::vector<Card*> cards;
    std::map<unsigned, Card*> cards_by_id;
    std::vector<Card*> player_cards;
    std::map<std::pair<std::string, unsigned>, Card*> player_cards_by_name;
    std::vector<Card*> player_commanders;
    std::vector<Card*> player_assaults;
    std::vector<Card*> player_structures;
    std::vector<Card*> player_actions;
    std::map<std::string, std::string> player_cards_abbr;
    const Card * by_id(unsigned id) const;
    void organize();
};

std::string simplify_name(const std::string& card_name);

#endif
