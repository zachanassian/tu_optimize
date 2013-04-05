#include "xml.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <algorithm>
#include "rapidxml.hpp"
#include "card.h"
#include "cards.h"
#include "deck.h"
#include "achievement.h"
#include "tyrant.h"
//---------------------- $20 cards.xml parsing ---------------------------------
// Sets: 1 enclave; 2 nexus; 3 blight; 4 purity; 5 homeworld;
// 6 phobos; 7 phobos aftermath; 8 awakening
// 1000 standard; 5000 rewards; 5001 promotional; 9000 exclusive
// mission only and test cards have no set
using namespace rapidxml;

std::map<unsigned, unsigned> sets_counts;

Faction map_to_faction(unsigned i)
{
    return(i == 1 ? imperial :
           i == 3 ? bloodthirsty :
           i == 4 ? xeno :
           i == 8 ? righteous :
           i == 9 ? raider :
           allfactions);
}

CardType::CardType map_to_type(unsigned i)
{
    return(i == 1 ? CardType::commander :
           i == 2 ? CardType::assault :
           i == 4 ? CardType::structure :
           i == 8 ? CardType::action :
           CardType::num_cardtypes);
}

Faction skill_faction(xml_node<>* skill)
{
    unsigned unmapped_faction(0);
    xml_attribute<>* y(skill->first_attribute("y"));
    if(y)
    {
        unmapped_faction = atoi(y->value());
    }
    return(unmapped_faction == 0 ? allfactions : map_to_faction(unmapped_faction));
}

unsigned skill_value(xml_node<>* skill)
{
    unsigned value(0);
    xml_attribute<>* x(skill->first_attribute("x"));
    if(x)
    {
        value = atoi(x->value());
    }
    return(value);
}

template<Skill skill>
void handle_skill(xml_node<>* node, Card* card)
{
    bool all(node->first_attribute("all"));
    bool played(node->first_attribute("played"));
    bool attacked(node->first_attribute("attacked"));
    bool kill(node->first_attribute("kill"));
    bool died(node->first_attribute("died"));
    bool normal(!(played || died || attacked || kill));
    if(played) { card->add_played_skill(skill, skill_value(node), skill_faction(node), all); }
    if(attacked) {card->add_attacked_skill(skill, skill_value(node), skill_faction(node), all); }
    if(kill) {card->add_kill_skill(skill, skill_value(node), skill_faction(node), all); }
    if(died) {card->add_died_skill(skill, skill_value(node), skill_faction(node), all); }
    if(normal) {card->add_skill(skill, skill_value(node), skill_faction(node), all); }
}
//------------------------------------------------------------------------------
void load_decks_xml(Decks& decks, const Cards& cards)
{
    try
    {
        read_missions(decks, cards, "missions.xml");
    }
    catch(const rapidxml::parse_error& e)
    {
        std::cout << "\nException while loading decks from file missions.xml\n";
    }
    try
    {
        read_raids(decks, cards, "raids.xml");
    }
    catch(const rapidxml::parse_error& e)
    {
        std::cout << "\nException while loading decks from file raids.xml\n";
    }
    try
    {
        read_quests(decks, cards, "quests.xml");
    }
    catch(const rapidxml::parse_error& e)
    {
        std::cout << "\nException while loading decks from file quests.xml\n";
    }
}

//------------------------------------------------------------------------------
void parse_file(const char* filename, std::vector<char>& buffer, xml_document<>& doc)
{
    std::ifstream cards_stream(filename, std::ios::binary);
    if(!cards_stream.good())
    {
        std::cout << "Warning: The file '" << filename << "' does not exist. Proceeding without reading from this file.\n";
        buffer.resize(1);
        buffer[0] = 0;
        doc.parse<0>(&buffer[0]);
        return;
    }
    // Get the size of the file
    cards_stream.seekg(0,std::ios::end);
    std::streampos length = cards_stream.tellg();
    cards_stream.seekg(0,std::ios::beg);
    buffer.resize(length + std::streampos(1));
    cards_stream.read(&buffer[0],length);
    // zero-terminate
    buffer[length] = '\0';
    try
    {
        doc.parse<0>(&buffer[0]);
    }
    catch(rapidxml::parse_error& e)
    {
        std::cerr << "Parse error exception.\n";
        std::cout << e.what();
        throw(e);
    }
}
//------------------------------------------------------------------------------
void read_cards(Cards& cards)
{
    std::vector<char> buffer;
    xml_document<> doc;
    parse_file("cards.xml", buffer, doc);
    xml_node<>* root = doc.first_node();

    if(!root)
    {
        return;
    }

    bool mission_only(false);
    unsigned nb_cards(0);
    for(xml_node<>* card = root->first_node();
        card;
        card = card->next_sibling())
    {
        if(strcmp(card->name(), "unit") == 0)
        {
            xml_node<>* id_node(card->first_node("id"));
            assert(id_node);
            unsigned id(id_node ? atoi(id_node->value()) : 0);
            xml_node<>* name_node(card->first_node("name"));
            xml_node<>* attack_node(card->first_node("attack"));
            xml_node<>* health_node(card->first_node("health"));
            xml_node<>* cost_node(card->first_node("cost"));
            xml_node<>* unique_node(card->first_node("unique"));
            xml_node<>* rarity_node(card->first_node("rarity"));
            xml_node<>* type_node(card->first_node("type"));
            xml_node<>* set_node(card->first_node("set"));
            int set(set_node ? atoi(set_node->value()) : -1);
            mission_only = set == -1;
            if((mission_only || set >= 0) && name_node && rarity_node)
            {
                if(!mission_only)
                {
                    nb_cards++;
                    sets_counts[set]++;
                }
                Card* c(new Card());
                c->m_id = id;
                c->m_base_id = id;
                c->m_name = name_node->value();
                // So far, commanders have attack_node (value == 0)
                if(id < 1000)
                { c->m_type = CardType::assault; }
                else if(id < 2000)
                { c->m_type = CardType::commander; }
                else if(id < 3000)
                { c->m_type = CardType::structure; }
                else if(id < 4000)
                { c->m_type = CardType::action; }
                else if(id < 5000)
                { c->m_type = CardType::assault; }
                else
                { c->m_type = cost_node ? (attack_node ? CardType::assault : CardType::structure) : (health_node ? CardType::commander : CardType::action); }
                if(attack_node) { c->m_attack = atoi(attack_node->value()); }
                if(health_node) { c->m_health = atoi(health_node->value()); }
                if(cost_node) { c->m_delay = atoi(cost_node->value()); }
                if(unique_node) { c->m_unique = true; }
                c->m_rarity = atoi(rarity_node->value());
                unsigned type(type_node ? atoi(type_node->value()) : 0);
                c->m_faction = map_to_faction(type);
                c->m_set = set;
                for(xml_node<>* skill = card->first_node("skill"); skill;
                    skill = skill->next_sibling("skill"))
                {
                    if(strcmp(skill->first_attribute("id")->value(), "antiair") == 0)
                    { c->m_antiair = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "armored") == 0)
                    { c->m_armored = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "berserk") == 0)
                    {
                        bool attacked(skill->first_attribute("attacked"));
                        if(attacked) { c->m_berserk_oa = atoi(skill->first_attribute("x")->value()); }
                        else {c->m_berserk = atoi(skill->first_attribute("x")->value()); }
                    }
                    if(strcmp(skill->first_attribute("id")->value(), "blitz") == 0)
                    { c->m_blitz = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "burst") == 0)
                    { c->m_burst = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "counter") == 0)
                    { c->m_counter = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "crush") == 0)
                    { c->m_crush = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "disease") == 0)
                    {
                        bool attacked(skill->first_attribute("attacked"));
                        if(attacked) { c->m_disease_oa = true; }
                        else {c->m_disease = true; }
                    }
                    if(strcmp(skill->first_attribute("id")->value(), "emulate") == 0)
                    { c->m_emulate = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "evade") == 0)
                    { c->m_evade = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "fear") == 0)
                    { c->m_fear = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "flurry") == 0)
                    { c->m_flurry = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "flying") == 0)
                    { c->m_flying = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "fusion") == 0)
                    { c->m_fusion = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "immobilize") == 0)
                    { c->m_immobilize = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "intercept") == 0)
                    { c->m_intercept = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "leech") == 0)
                    { c->m_leech = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "legion") == 0)
                    { c->m_legion = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "payback") == 0)
                    { c->m_payback = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "pierce") == 0)
                    { c->m_pierce = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "poison") == 0)
                    {
                        bool attacked(skill->first_attribute("attacked"));
                        if(attacked) { c->m_poison_oa = atoi(skill->first_attribute("x")->value()); }
                        else {c->m_poison = atoi(skill->first_attribute("x")->value()); }
                    }
                    if(strcmp(skill->first_attribute("id")->value(), "refresh") == 0)
                    { c->m_refresh = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "regenerate") == 0)
                    { c->m_regenerate = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "siphon") == 0)
                    { c->m_siphon = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "stun") == 0)
                    { c->m_stun = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "swipe") == 0)
                    { c->m_swipe = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "tribute") == 0)
                    { c->m_tribute = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "valor") == 0)
                    { c->m_valor = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "wall") == 0)
                    { c->m_wall = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "augment") == 0)
                    { handle_skill<augment>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "backfire") == 0)
                    { handle_skill<backfire>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "chaos") == 0)
                    { handle_skill<chaos>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "cleanse") == 0)
                    { handle_skill<cleanse>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "enfeeble") == 0)
                    { handle_skill<enfeeble>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "freeze") == 0)
                    { handle_skill<freeze>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "heal") == 0)
                    { handle_skill<heal>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "infuse") == 0)
                    { handle_skill<infuse>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "jam") == 0)
                    { handle_skill<jam>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "mimic") == 0)
                    { handle_skill<mimic>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "protect") == 0)
                    { handle_skill<protect>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "rally") == 0)
                    { handle_skill<rally>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "recharge") == 0)
                    { handle_skill<recharge>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "repair") == 0)
                    { handle_skill<repair>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "rush") == 0)
                    { handle_skill<rush>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "shock") == 0)
                    { handle_skill<shock>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "siege") == 0)
                    { handle_skill<siege>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "split") == 0)
                    { handle_skill<split>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "strike") == 0)
                    { handle_skill<strike>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "summon") == 0)
                    { handle_skill<summon>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "supply") == 0)
                    { handle_skill<supply>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "weaken") == 0)
                    { handle_skill<weaken>(skill, c); }
                }
                cards.cards.push_back(c);
            }
        }
    }
    cards.organize();
    // std::cout << "nb cards: " << nb_cards << "\n";
    // for(auto counts: sets_counts)
    // {
    //   std::cout << "set " << counts.first << " (" << sets[counts.first] << ")" << ": " << counts.second << "\n";
    // }
    // std::cout << "nb mission cards: " << cards.mission_cards.size() << "\n";
}
//------------------------------------------------------------------------------
Deck* read_deck(Decks& decks, const Cards& cards, xml_node<>* node, DeckType::DeckType decktype, unsigned id, std::string& deck_name)
{
    xml_node<>* commander_node(node->first_node("commander"));
    const Card* commander_card{cards.by_id(atoi(commander_node->value()))};
    xml_node<>* deck_node(node->first_node("deck"));
    xml_node<>* always_node{deck_node->first_node("always_include")};
    std::vector<const Card*> always_cards;
    for(xml_node<>* card_node = (always_node ? always_node : deck_node)->first_node("card");
            card_node;
            card_node = card_node->next_sibling("card"))
    {
        unsigned card_id{static_cast<unsigned>(atoi(card_node->value()))};
        always_cards.push_back(cards.by_id(card_id));
    }
    std::vector<std::pair<unsigned, std::vector<const Card*> > > some_cards;
    for(xml_node<>* pool_node = deck_node->first_node("card_pool");
            pool_node;
            pool_node = pool_node->next_sibling("card_pool"))
    {
        unsigned num_cards_from_pool{static_cast<unsigned>(atoi(pool_node->first_attribute("amount")->value()))};
        std::vector<const Card*> cards_from_pool;

        for(xml_node<>* card_node = pool_node->first_node("card");
                card_node;
                card_node = card_node->next_sibling("card"))
        {
            unsigned card_id{static_cast<unsigned>(atoi(card_node->value()))};
            // Special case Arctis Vanguard id 0 because of stray ` character.
            // Don't continue on other raids because I want to be notified of other errors.
            if(card_id == 0 && decktype == DeckType::raid && id == 1)
            {
                continue;
            }
            cards_from_pool.push_back(cards.by_id(card_id));
        }
        some_cards.push_back(std::make_pair(num_cards_from_pool, cards_from_pool));
    }
    decks.decks.push_back(Deck{decktype, id, deck_name});
    Deck* deck = &decks.decks.back();
    deck->set(commander_card, always_cards, some_cards);
    decks.by_name[deck_name] = deck;
    std::stringstream alt_name;
    alt_name << decktype_names[decktype] << " #" << id;
    decks.by_name[alt_name.str()] = deck;
    return deck;
}
//------------------------------------------------------------------------------
void read_missions(Decks& decks, const Cards& cards, std::string filename)
{
    std::vector<char> buffer;
    xml_document<> doc;
    parse_file(filename.c_str(), buffer, doc);
    xml_node<>* root = doc.first_node();

    if(!root)
    {
        return;
    }

    for(xml_node<>* mission_node = root->first_node("mission");
        mission_node;
        mission_node = mission_node->next_sibling("mission"))
    {
        std::vector<unsigned> card_ids;
        xml_node<>* id_node(mission_node->first_node("id"));
        assert(id_node);
        unsigned id(id_node ? atoi(id_node->value()) : 0);
        xml_node<>* name_node(mission_node->first_node("name"));
        std::string deck_name{name_node->value()};
        read_deck(decks, cards, mission_node, DeckType::mission, id, deck_name);
        decks.mission_names_by_id[id] = deck_name;
    }
}
//------------------------------------------------------------------------------
void read_raids(Decks& decks, const Cards& cards, std::string filename)
{
    std::vector<char> buffer;
    xml_document<> doc;
    parse_file(filename.c_str(), buffer, doc);
    xml_node<>* root = doc.first_node();

    if(!root)
    {
        return;
    }

    for(xml_node<>* raid_node = root->first_node("raid");
        raid_node;
        raid_node = raid_node->next_sibling("raid"))
    {
        xml_node<>* id_node(raid_node->first_node("id"));
        assert(id_node);
        unsigned id(id_node ? atoi(id_node->value()) : 0);
        xml_node<>* name_node(raid_node->first_node("name"));
        std::string deck_name{name_node->value()};
        read_deck(decks, cards, raid_node, DeckType::raid, id, deck_name);
    }
}
//------------------------------------------------------------------------------
void read_quests(Decks& decks, const Cards& cards, std::string filename)
{
    std::vector<char> buffer;
    xml_document<> doc;
    parse_file(filename.c_str(), buffer, doc);
    xml_node<>* root = doc.first_node();

    if(!root)
    {
        return;
    }

    // Seems always_cards is empty for all quests.
    std::vector<const Card*> always_cards;

    for(xml_node<>* quest_node = root->first_node("step");
        quest_node;
        quest_node = quest_node->next_sibling("step"))
    {
        xml_node<>* id_node(quest_node->first_node("id"));
        assert(id_node);
        unsigned id(id_node ? atoi(id_node->value()) : 0);
        std::string deck_name{"Step " + std::string{id_node->value()}};
        xml_node<>* battleground_id_node(quest_node->first_node("battleground_id"));
        int battleground_id(battleground_id_node ? atoi(battleground_id_node->value()) : -1);
        Deck* deck = read_deck(decks, cards, quest_node, DeckType::quest, id, deck_name);
        deck->effect = static_cast<enum Effect>(battleground_id);
    }
}
//------------------------------------------------------------------------------
extern unsigned turn_limit;
Comparator get_comparator(xml_node<>* node, Comparator default_comparator)
{
    xml_attribute<>* compare(node->first_attribute("compare"));
    if(!compare) { return default_comparator; }
    else if(strcmp(compare->value(), "equal") == 0) { return equal; }
    else if(strcmp(compare->value(), "great_equal") == 0) { return great_equal; }
    else if(strcmp(compare->value(), "less_equal") == 0) { return less_equal; }
    else { throw std::runtime_error(std::string("Not implemented: compare=\"") + compare->value() + "\""); }
}

void read_achievement(Decks& decks, const Cards& cards, Achievement& achievement, const char* achievement_id_name, std::string filename/* = "achievements.xml"*/)
{
    std::vector<char> buffer;
    xml_document<> doc;
    parse_file(filename.c_str(), buffer, doc);
    xml_node<>* root = doc.first_node();

    if(!root)
    {
        throw std::runtime_error("Failed to parse " + filename);
    }

    std::map<std::string, int> skill_map;
    for(unsigned i(0); i < Skill::num_skills; ++i)
    {
        std::string skill_id{skill_names[i]};
        std::transform(skill_id.begin(), skill_id.end(), skill_id.begin(), ::tolower);
        skill_map[skill_id] = i;
    }

    for(xml_node<>* achievement_node = root->first_node();
        achievement_node;
        achievement_node = achievement_node->next_sibling())
    {
        if(strcmp(achievement_node->name(), "achievement") != 0) { continue; }
        xml_node<>* id_node(achievement_node->first_node("id"));
        xml_node<>* name_node(achievement_node->first_node("name"));
        if(!id_node || !name_node || (strcmp(id_node->value(), achievement_id_name) != 0 && strcmp(name_node->value(), achievement_id_name) != 0)) { continue; }
        achievement.id = atoi(id_node->value());
        achievement.name = name_node->value();
        std::cout << "Achievement " << id_node->value() << " " << name_node->value() << ": " << achievement_node->first_node("desc")->value() << std::endl;
        xml_node<>* type_node(achievement_node->first_node("type"));
        xml_attribute<>* mission_id(type_node ? type_node->first_attribute("mission_id") : NULL);
        if(!type_node || !mission_id)
        {
            throw std::runtime_error("Must be 'mission' type.");
        }
        assert(strcmp(type_node->first_attribute("winner")->value(), "1") == 0);
        if(strcmp(mission_id->value(), "*") != 0)
        {
            achievement.mission_condition.init(atoi(mission_id->value()), get_comparator(type_node, equal));
            std::cout << "  Mission" << achievement.mission_condition.str() << " (" << decks.mission_names_by_id[atoi(mission_id->value())] << ") and win" << std::endl;
        }
        for (xml_node<>* req_node = achievement_node->first_node("req");
            req_node;
            req_node = req_node->next_sibling("req"))
        {
            Comparator comparator = get_comparator(req_node, great_equal);
            xml_attribute<>* skill_id(req_node->first_attribute("skill_id"));
            xml_attribute<>* unit_id(req_node->first_attribute("unit_id"));
            xml_attribute<>* unit_type(req_node->first_attribute("unit_type"));
            xml_attribute<>* unit_race(req_node->first_attribute("unit_race"));
            xml_attribute<>* unit_rarity(req_node->first_attribute("unit_rarity"));
            xml_attribute<>* num_turns(req_node->first_attribute("num_turns"));
            xml_attribute<>* num_used(req_node->first_attribute("num_used"));
            xml_attribute<>* num_played(req_node->first_attribute("num_played"));
            xml_attribute<>* num_killed(req_node->first_attribute("num_killed"));
            xml_attribute<>* num_killed_with(req_node->first_attribute("num_killed_with"));
            xml_attribute<>* damage(req_node->first_attribute("damage"));
            xml_attribute<>* com_total(req_node->first_attribute("com_total"));
            xml_attribute<>* only(req_node->first_attribute("only"));
            if(skill_id && num_used)
            {
                auto x = skill_map.find(skill_id->value());
                if(x == skill_map.end())
                {
                    throw std::runtime_error(std::string("Unknown skill ") + skill_id->value());
                }
                achievement.skill_used[x->second] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_used->value()), comparator);
                std::cout << "  Use skills: " << skill_id->value() << achievement.req_counter.back().str() << std::endl;
            }
            else if(unit_id && num_played)
            {
                achievement.unit_played[atoi(unit_id->value())] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_played->value()), comparator);
                std::cout << "  Play units: " << cards.by_id(atoi(unit_id->value()))->m_name << achievement.req_counter.back().str() << std::endl;
            }
            else if(unit_type && num_played)
            {
                auto i = map_to_type(atoi(unit_type->value()));
                if(i == CardType::num_cardtypes)
                {
                    throw std::runtime_error(std::string("Unknown unit_type ") + unit_type->value());
                }
                achievement.unit_type_played[i] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_played->value()), comparator);
                std::cout << "  Play units of type: " << cardtype_names[i] << achievement.req_counter.back().str() << std::endl;
            }
            else if(unit_race && num_played)
            {
                auto i = map_to_faction(atoi(unit_race->value()));
                if(i == Faction::allfactions)
                {
                    throw std::runtime_error(std::string("Unknown unit_race ") + unit_race->value());
                }
                achievement.unit_faction_played[i] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_played->value()), comparator);
                std::cout << "  Play units of race (faction): " << faction_names[i] << achievement.req_counter.back().str() << std::endl;
            }
            else if(unit_rarity && num_played)
            {
                achievement.unit_rarity_played[atoi(unit_rarity->value())] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_played->value()), comparator);
                std::cout << "  Play units of rarity: " << rarity_names[atoi(unit_rarity->value())] << achievement.req_counter.back().str() << std::endl;
            }
            else if(unit_type && num_killed)
            {
                auto i = map_to_type(atoi(unit_type->value()));
                if(i == CardType::num_cardtypes)
                {
                    throw std::runtime_error(std::string("Unknown unit_type ") + unit_type->value());
                }
                achievement.unit_type_killed[i] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_killed->value()), comparator);
                std::cout << "  Kill units of type: " << cardtype_names[i] << achievement.req_counter.back().str() << std::endl;
            }
            else if(num_killed_with && skill_id && strcmp(skill_id->value(), "flying") == 0)
            {
                achievement.misc_req[AchievementMiscReq::unit_with_flying_killed] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_killed_with->value()), comparator);
                std::cout << "  " << achievement_misc_req_names[AchievementMiscReq::unit_with_flying_killed] << achievement.req_counter.back().str() << std::endl;
            }
            else if(only && skill_id && strcmp(skill_id->value(), "0") == 0)
            {
                achievement.misc_req[AchievementMiscReq::skill_activated] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(0, equal);
                std::cout << "  " << achievement_misc_req_names[AchievementMiscReq::skill_activated] << achievement.req_counter.back().str() << std::endl;
            }
            else if(num_turns)
            {
                if(comparator == less_equal)
                {
                    turn_limit = atoi(num_turns->value());
                }
                achievement.misc_req[AchievementMiscReq::turns] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(num_turns->value()), comparator);
                std::cout << "  " << achievement_misc_req_names[AchievementMiscReq::turns] << achievement.req_counter.back().str() << std::endl;
            }
            else if(damage)
            {
                achievement.misc_req[AchievementMiscReq::damage] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(damage->value()), comparator);
                std::cout << "  " << achievement_misc_req_names[AchievementMiscReq::damage] << achievement.req_counter.back().str() << std::endl;
            }
            else if(com_total)
            {
                achievement.misc_req[AchievementMiscReq::com_total] = achievement.req_counter.size();
                achievement.req_counter.emplace_back(atoi(com_total->value()), comparator);
                std::cout << "  " << achievement_misc_req_names[AchievementMiscReq::com_total] << achievement.req_counter.back().str() << std::endl;
            }
            else
            {
                throw std::runtime_error("Not implemented.");
            }
        }
        return;
    }
    throw std::runtime_error("No such achievement.");
}
