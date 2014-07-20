#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/spirit/include/qi.hpp>

#include "tyrant.h"
#include "custom_card.h"
#include "xml.h" // handle_skill

using namespace boost::spirit::qi;

void CustomCard::parse_token(const std::string &token) {
    std::cout << "Parsing [" << token << "]" << std::endl;
    // decide what kind of token it is
    //if (boost::starts_with(token, "JAM "))
    int x;
    // there has to be a smarter way to do this
    if (phrase_parse(token.begin(), token.end(), "JAM EVERY " >> int_, space, x)
            || phrase_parse(token.begin(), token.end(), "JAM " >> int_, space, x)) {
        add_skill(jam, x, allfactions, false);
    } else if (phrase_parse(token.begin(), token.end(), "EVADE " >> int_, space, x)) {
        add_skill(evade, x, allfactions, false);
    }
}

CustomCard::CustomCard(unsigned id, const std::string &card_spec): Card() {
    // proposed format:
    // Name, Rarity Faction Atk/HP/delay, skill X, skill Y, skill Z
    // Examples
    // Auger Ream, Legendary Imperial 6/18/2, evade 3, rally imperial 5, jam every 4
    // Malika, rare raider -/15/-, counter 1, heal raider 1, strike 1
    m_id = id;
    m_max_level_id = id;
    std::stringstream ss(card_spec);
    std::getline(ss, m_name, ','); // first token is always card name
    boost::algorithm::trim(m_name);
    if (m_name.size() == 0) {
        throw std::runtime_error("Custom card missing name? \"" + card_spec + "\"");
    }
    std::cout << "Constructing card: [" << m_name << "]" << std::endl;
    std::string token;
    while (std::getline(ss, token, ',')) {
        boost::algorithm::trim(token);
        boost::to_upper(token);
        parse_token(token);
    }
}
