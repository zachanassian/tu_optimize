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
#include <boost/algorithm/string.hpp> // to_lower()
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <complex>
#include <unistd.h> // access()

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

        // define the symbol tables using data from tyrant.h/cpp
        // Factions
        qi::symbols<char, Faction> faction_symbols;
        for (int i = 1; i < Faction::num_factions - 1; i++)
        {
            faction_symbols.add(faction_names[i].c_str(), (Faction)i);
        }

        // Rarities
        qi::symbols<char, int> rarity_symbols;
        rarity_symbols.add
            ("common", 1)
            ("rare", 2)
            ("epic", 3)
            ("legendary", 4)
            ("vindicator", 5)
            ;

        // Skills
        qi::symbols<char, Skill> skill_symbols;
        for (int i = 1; i < Skill::num_skills - 1; i++)
        {
            std::string skill(skill_names[i]);
            boost::algorithm::to_lower(skill);
            skill_symbols.add(skill.c_str(), (Skill)i);
        }

        // now the grammar rule definitions...
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
        card_info = no_case[ faction_symbols ]
            ^ no_case[ rarity_symbols ]
            ^ (card_stats | cmdr_stats)
            ;

        // SkillName "all"? Faction? "every"? number?
        card_skill = no_case[ skill_symbols ]
            >> -( qi::lit("all") >> qi::attr(true) )
            >> -no_case[ faction_symbols ]
            >> -qi::lit("every") // optional for jam
            >> -qi::int_
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
    // fill in some dummy values so we know what hasn't been specified
    m_attack = STAT_NOT_AVAILABLE;
    m_health = STAT_NOT_AVAILABLE;
    m_delay = STAT_NOT_AVAILABLE;
    m_faction = allfactions;

    // FIXME a new parser is created for every card, not very efficient.
    // But it's once off and the time is really minimal anyway
    CustomCardParser<std::string::const_iterator> parser;
    std::string::const_iterator iter = card_spec.begin();
    bool r = qi::phrase_parse(iter, card_spec.end(), parser, ascii::space, *this);
    // make sure the whole string has been parsed
    if (!r || iter != card_spec.end())
    {
        std::string remaining_unparsed(iter, card_spec.end());
        throw std::runtime_error(card_spec + ": Custom card failed to parse after \"" + remaining_unparsed + "\"");
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
    else if (m_faction == allfactions)
    {
        throw std::runtime_error("Custom card \"" + m_name + "\" has no faction!");
    }
    else
    {
        m_type = CardType::commander;
        m_attack = 0;
        m_delay = 0;
    }
}

// from sim.cpp
extern std::string card_description(const Cards& cards, const Card* c);

void CustomCard::debug(const Cards& cards)
{
    std::cout << "Custom Card: [" << m_id << "]: " << card_description(cards, this) << std::endl;
}

void CustomCard::add_skill(const custom_card_skill &skill)
{
// passive skills, just assign member
#define PASSIVE_SKILL(sk) \
    case sk: \
        m_ ## sk = skill.value; \
        break
// enhance skills, just add_skill() - also some active skills also do not have members in Card()
#define ENHANCE_SKILL(sk) \
    case sk: \
        Card::add_skill(skill.skill, skill.value, skill.faction, skill.all); \
        break
// active skills, assign member + invoke add_skill()
#define ACTIVE_SKILL(sk) \
    case sk: \
        m_## sk = skill.value; \
        Card::add_skill(skill.skill, skill.value, skill.faction, skill.all); \
        break
    switch (skill.skill)
    {
        PASSIVE_SKILL(armored);
        PASSIVE_SKILL(berserk);
        PASSIVE_SKILL(corrosive);
        PASSIVE_SKILL(counter);
        PASSIVE_SKILL(evade);
        PASSIVE_SKILL(flurry);
        PASSIVE_SKILL(inhibit);
        PASSIVE_SKILL(leech);
        PASSIVE_SKILL(pierce);
        PASSIVE_SKILL(poison);
        case wall: m_wall = true; break; // just a simple boolean
        ACTIVE_SKILL(enfeeble);
        ENHANCE_SKILL(enhance_armored);
        ENHANCE_SKILL(enhance_berserk);
        ENHANCE_SKILL(enhance_corrosive);
        ENHANCE_SKILL(enhance_counter);
        ENHANCE_SKILL(enhance_enfeeble);
        ENHANCE_SKILL(enhance_evade);
        ENHANCE_SKILL(enhance_leech);
        ENHANCE_SKILL(enhance_heal);
        ENHANCE_SKILL(enhance_poison);
        ENHANCE_SKILL(enhance_rally);
        ENHANCE_SKILL(enhance_strike);
        ACTIVE_SKILL(heal);
        ACTIVE_SKILL(jam);
        ENHANCE_SKILL(protect); // there is no m_protect in Card?
        ACTIVE_SKILL(rally);
        ENHANCE_SKILL(siege);   // or m_siege
        ACTIVE_SKILL(strike);
        ENHANCE_SKILL(weaken);
        default:
            // left out some skills not in TU
            std::cerr << "Warning: Skill \"" << skill_names[skill.skill] << "\" in custom card is not supported by sim, ignoring!" << std::endl;
            break;
    }
}

void CustomCard::handle_info(const custom_card_info &info)
{
    if (info.faction != allfactions)
    {
        m_faction = info.faction;
    }
    if (info.rarity > 0)
    {
        m_rarity = info.rarity;
    }
    // only overwrite the stats if at least health is defined
    if (info.stats.health != STAT_NOT_AVAILABLE) {
        m_attack = info.stats.attack;
        m_health = info.stats.health;
        m_delay = info.stats.delay;
    }
}

void CustomCardReader::parse_custom_cards(const std::string& card_list)
{
    boost::tokenizer<boost::char_delimiters_separator<char>> card_tokens{card_list, boost::char_delimiters_separator<char>{false, ":;", ""}};

    auto token_iter = card_tokens.begin();
    for (; token_iter != card_tokens.end(); ++token_iter)
    {
        std::string card_spec(*token_iter);
        CustomCard* custom_card = new CustomCard(next_card_id++, card_spec);
        if (!quiet)
        {
            custom_card->debug(cards);
        }
        cards.cards.push_back(custom_card);
    }
}

void CustomCardReader::read_custom_cards_file(const char *filename)
{
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
            parse_custom_cards(card_spec);
        }
        catch(std::exception& e)
        {
            std::cerr << "Error in custom cards file " << filename << " at line " << num_line << " while parsing card '" << card_spec << "': " << e.what() << "\n";
        }
    }
}

void CustomCardReader::process_custom_cards(const char *param)
{
    if (access(param, R_OK) == 0)
    {
        // read the param as a file if it exists
        read_custom_cards_file(param);
    }
    else
    {
        // just parse the string otherwise
        parse_custom_cards(param);
    }
}

CustomCardReader::CustomCardReader(Cards& cards): cards(cards), quiet(false)
{
    // give the custom cards the max card ID + 2
    next_card_id = (--cards.cards_by_id.end())->first;
    next_card_id++;
}

void CustomCardReader::process_args(int argc, char *argv[])
{
    const char *custom_cards = "data/customcards.txt";
    bool use_custom_cards = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-C") == 0)
        {
            use_custom_cards = true;
        }
        else if (strncmp(argv[i], "-C=", 3) == 0)
        {
            use_custom_cards = true;
            custom_cards = argv[i] + 3;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            // bit ugly that we parse this flag in multiple places...
            quiet = true;
        }
    }
    if (use_custom_cards)
    {
        process_custom_cards(custom_cards);
        // organize() again so the new cards are included
        cards.organize();
    }
}
