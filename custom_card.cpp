/***
 * Implements a Boost::Spirit parser for custom cards
 * pseudo-EBNF grammar as follows:
 *  Card := CardName { CardSkill | CardInfo } (comma-separated list)
 *  CardInfo := Rarity Faction ( CardStats | CommanderStats ) (in any order)
 *  CardStats := ("-"|int) "/" int ("/" ("-"|int))?
 *  CommanderStats := int "HP"
 *  CardSkill := SkillName "all"? Faction? "every"? int
 * Some examples:
 *  Auger Ream, Legendary Imperial 6/18/2, evade 3, rally imperial 5, jam every 4
 *  Dream Mutator, progenitor, protect all 5, jam all every 5, corrosive 10, 5/32/4 vindicator
 *  Commander Sheppard, 100HP imperial, rally all imperial 10, enhance berserk 3
 *  Oreworks, epic raider -/18/1, Enfeeble All 1, Strike All 1, Weaken All 1
 ***/
//#define BOOST_SPIRIT_DEBUG

#include <boost/tokenizer.hpp>
//#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
//#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <complex>
#include <unistd.h> // access

#include "tyrant.h"
#include "custom_card.h"

#define STAT_NOT_AVAILABLE (99999) // a bit of a hack, used to determine card type

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;

struct custom_card_stats
{
    unsigned attack;
    unsigned health;
    unsigned delay;

    custom_card_stats():
        attack(STAT_NOT_AVAILABLE),
        health(STAT_NOT_AVAILABLE),
        delay(STAT_NOT_AVAILABLE)
    {
    }
};

BOOST_FUSION_ADAPT_STRUCT
(
    custom_card_stats,
    (unsigned, attack)
    (unsigned, health)
    (unsigned, delay)
)

struct custom_card_info 
{
    Faction faction;
    unsigned rarity;
    struct custom_card_stats stats;
};

BOOST_FUSION_ADAPT_STRUCT
(
    custom_card_info,
    (Faction, faction)
    (unsigned, rarity)
    (struct custom_card_stats, stats)
)

struct custom_card_skill
{
    Skill skill;
    bool all;
    Faction faction;
    unsigned value;
};

BOOST_FUSION_ADAPT_STRUCT
(
    custom_card_skill,
    (Skill, skill)
    (bool, all)
    (Faction, faction)
    (unsigned, value)
)

/* symbol tables */
struct factions_ : qi::symbols<char, Faction>
{
    factions_() 
    {
        add
            ("bloodthirsty", bloodthirsty)
            ("imperial", imperial)
            ("raider", raider)
            ("righteous", righteous)
            ("xeno", xeno)
            ("progenitor", progenitor)
        ;
    }
} factions;

struct rarity_ : qi::symbols<char, int> 
{
    rarity_() 
    {
        add
            ("common", 1)
            ("rare", 2)
            ("epic", 3)
            ("legendary", 4)
            ("vindicator", 5)
        ;
    }
} rarity;

/* TODO */
struct skills_ : qi::symbols<char, Skill> 
{
    skills_() 
    {
        add
            ("jam", jam)
            ("rally", rally)
            ("evade", evade)
            ("heal", heal)
            ("enhance counter", enhance_counter)
        ;
    }
} skills;

template <typename Iterator>
struct CustomCardParser: qi::grammar<Iterator, CustomCard(), ascii::space_type> 
{
    CustomCardParser(): CustomCardParser::base_type(start) 
    {
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::lexeme;
        using ascii::no_case;

        // card name is a string
        card_name = lexeme [ +(qi::char_ - ',') ];

        // commanders only have HP (20HP or -/20 or -/20/-)
        cmdr_stats =
            qi::attr(STAT_NOT_AVAILABLE) >> qi::int_ >> -no_case[ "HP" ] >> qi::attr(STAT_NOT_AVAILABLE);

        // attack/health/delay
        card_stats =
            (("-" >> qi::attr(STAT_NOT_AVAILABLE)) | qi::int_)
            >> '/' >> qi::int_
            >> -('/' >> (("-" >> qi::attr(STAT_NOT_AVAILABLE)) | qi::int_))
            ;

        // allow card info to be specified in any order
        card_info = no_case[ factions ]
            ^ no_case[ rarity ]
            ^ (card_stats | cmdr_stats)
            ;

        // SkillName "all"? Faction? "every"? number
        card_skill = no_case[ skills ]
            >> -( qi::lit("all") >> qi::attr(true) )
            >> -no_case[ factions ]
            >> -qi::lit("every") // optional for jam
            >> qi::int_
            ;

        // name, (card_info|card_skill), ...
        start = card_name [ phx::bind(&CustomCard::m_name, _val) = _1 ]
            >> ','
            >> (card_info [ phx::bind(&CustomCard::handle_info, _val, _1) ]
                    | card_skill [ phx::bind(&CustomCard::add_skill, _val, _1) ]) % ','
            ;
    }
    qi::rule<Iterator, CustomCard(), ascii::space_type> start;
    qi::rule<Iterator, std::string(), ascii::space_type> card_name;
    qi::rule<Iterator, custom_card_skill(), ascii::space_type> card_skill;
    qi::rule<Iterator, custom_card_info(), ascii::space_type> card_info;
    qi::rule<Iterator, custom_card_stats(), ascii::space_type> card_stats;
    qi::rule<Iterator, custom_card_stats(), ascii::space_type> cmdr_stats;
};

CustomCard::CustomCard(unsigned id, const std::string &card_spec): Card() 
{
    m_id = id;
    m_base_id = id;
    m_max_level_id = id;
    m_set = CUSTOM_CARD_SET; // need to set this or organize() will ignore it

    CustomCardParser<std::string::const_iterator> parser;
    std::string::const_iterator iter = card_spec.begin();
    bool r = qi::phrase_parse(iter, card_spec.end(), parser, ascii::space, *this);
    // make sure the whole string has been parsed
    if (!r || iter != card_spec.end()) 
    {
        throw std::runtime_error("Failed to parse custom card: \"" + card_spec + "\"");
    }

    // we can only figure out the type after parsing
    if (m_attack != STAT_NOT_AVAILABLE) 
    {
        // only assaults have attack stat
        m_type = CardType::assault;
    } 
    else if (m_delay != STAT_NOT_AVAILABLE) 
    {
        // commanders have no delay, so this must be structure
        m_type = CardType::structure;
        m_attack = 0;
    } 
    else if (m_health == STAT_NOT_AVAILABLE || m_health == 0) 
    {
        throw std::runtime_error("Custom card \"" + m_name + "\" has no health!");
    } 
    else 
    {
        m_type = CardType::commander;
        m_attack = 0;
        m_delay = 0;
    }
    debug(); // XXX
}

void CustomCard::debug() {
    std::cout << "Custom Card \"" << m_name << "\" (" << m_id << ")" << std::endl;
}

void CustomCard::add_skill(const custom_card_skill &skill) {
    Card::add_skill(skill.skill, skill.value, skill.faction, skill.all);
    switch (skill.skill) 
    {
        case jam:
            m_jam = skill.value;
            break;
            // TODO
        default:
            break;
    }
}

void CustomCard::handle_info(const custom_card_info &info) {
    if (info.faction != allfactions) 
    {
        m_faction = info.faction;
    }
    if (info.rarity > 0) 
    {
        m_rarity = info.rarity;
    }
    m_attack = info.stats.attack;
    m_health = info.stats.health;
    m_delay = info.stats.delay;
}

int parse_custom_cards(Cards& cards, unsigned id, const std::string& card_list) {
    boost::tokenizer<boost::char_delimiters_separator<char>> card_tokens{card_list, boost::char_delimiters_separator<char>{false, ":;", ""}};

    auto token_iter = card_tokens.begin();
    for (; token_iter != card_tokens.end(); ++token_iter)
    {
        std::string card_spec(*token_iter);
        Card* custom_card(new CustomCard(id++, card_spec));
        cards.cards.push_back(custom_card);
    }
    return id;
}

void read_custom_cards_file(Cards& cards, unsigned id, const char *filename) {
    std::ifstream custom_cards_file{filename};
    if (!custom_cards_file.good())
    {
        throw std::runtime_error(std::string("Failed to open custom cards file: ") + filename);
    }
    unsigned num_line(0);
    while (!custom_cards_file.eof())
    {
        std::string card_spec;
        getline(custom_cards_file, card_spec);
        ++num_line;
        if(card_spec.size() == 0 || strncmp(card_spec.c_str(), "//", 2) == 0)
        {
            continue;
        }
        try
        {
            id = parse_custom_cards(cards, id, card_spec);
        }
        catch(std::exception& e)
        {
            std::cerr << "Error in custom cards file " << filename << " at line " << num_line << " while parsing card '" << card_spec << "': " << e.what() << "\n";
        }
    }
}

void process_custom_cards(Cards& cards, const char *param) {
    // maps are sorted so we can get biggest id this way
    unsigned highest_id = (--cards.cards_by_id.end())->first;
    // is the supplied argument an actual file?
    if (access(param, R_OK) == 0)
    {
        read_custom_cards_file(cards, highest_id+1, param);
    }
    else
    {
        parse_custom_cards(cards, highest_id+1, param);
    }
    // organize() again so the new cards are included
    cards.organize();
}

void process_args_for_custom_cards(Cards& cards, int argc, char *argv[]) {
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-C") == 0)
        {
            process_custom_cards(cards, "data/customcards.txt");
        }
        else if (strncmp(argv[i], "-C=", 3) == 0)
        {
            process_custom_cards(cards, argv[i]+3);
        }
    }
}
