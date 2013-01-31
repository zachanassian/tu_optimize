#include "xml.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include "rapidxml.hpp"
#include "card.h"
#include "cards.h"
#include "deck.h"
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

template<unsigned>
struct GlobalSkill
{
    enum { type = 99 };
};

template<> struct GlobalSkill<augment> { enum {type = augment_all}; };
template<> struct GlobalSkill<chaos> { enum {type = chaos_all}; };
template<> struct GlobalSkill<cleanse> { enum {type = cleanse_all}; };
template<> struct GlobalSkill<enfeeble> { enum {type = enfeeble_all}; };
template<> struct GlobalSkill<freeze> { enum {type = freeze_all}; };
template<> struct GlobalSkill<heal> { enum {type = heal_all}; };
template<> struct GlobalSkill<jam> { enum {type = jam_all}; };
template<> struct GlobalSkill<protect> { enum {type = protect_all}; };
template<> struct GlobalSkill<rally> { enum {type = rally_all}; };
template<> struct GlobalSkill<repair> { enum {type = repair_all}; };
template<> struct GlobalSkill<siege> { enum {type = siege_all}; };
template<> struct GlobalSkill<strike> { enum {type = strike_all}; };
template<> struct GlobalSkill<weaken> { enum {type = weaken_all}; };

template<unsigned GlobalSkill>
bool handle_global_skill(xml_node<>* node, Card* card)
{
    bool played(node->first_attribute("played"));
    bool died(node->first_attribute("died"));
    bool attacked(node->first_attribute("attacked"));
    bool kill(node->first_attribute("kill"));
    if(node->first_attribute("all"))
    {
        if(played)
        {
            card->add_played_skill(ActiveSkill(GlobalSkill), skill_value(node), skill_faction(node));
            if(died) {card->add_died_skill(ActiveSkill(GlobalSkill), skill_value(node), skill_faction(node)); }
        }
        else if(died) {card->add_died_skill(ActiveSkill(GlobalSkill), skill_value(node), skill_faction(node)); }
        else if(attacked) {card->add_attacked_skill(ActiveSkill(GlobalSkill), skill_value(node), skill_faction(node)); }
        else if(kill) {card->add_kill_skill(ActiveSkill(GlobalSkill), skill_value(node), skill_faction(node)); }
        else {card->add_skill(ActiveSkill(GlobalSkill), skill_value(node), skill_faction(node)); }
        return(true);
    }
    return(false);
}

template<>
bool handle_global_skill<99>(xml_node<>* node, Card* card)
{
    return(false);
}

template<unsigned Skill>
void handle_skill(xml_node<>* node, Card* card)
{
    bool played(node->first_attribute("played"));
    bool died(node->first_attribute("died"));
    bool attacked(node->first_attribute("attacked"));
    bool kill(node->first_attribute("kill"));
    if(handle_global_skill<GlobalSkill<Skill>::type>(node, card)) {}
    else
    {
        if(played)
        {
            card->add_played_skill(ActiveSkill(Skill), skill_value(node), skill_faction(node));
            if(died) {card->add_died_skill(ActiveSkill(Skill), skill_value(node), skill_faction(node)); }
        }
        else if(died) {card->add_died_skill(ActiveSkill(Skill), skill_value(node), skill_faction(node)); }
        else if(attacked) {card->add_attacked_skill(ActiveSkill(Skill), skill_value(node), skill_faction(node)); }
        else if(kill) {card->add_kill_skill(ActiveSkill(Skill), skill_value(node), skill_faction(node)); }
        else {card->add_skill(ActiveSkill(Skill), skill_value(node), skill_faction(node)); }
    }
}
//------------------------------------------------------------------------------
void load_decks_xml(Decks& decks, Cards& cards)
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
        std::cout << "Parse error exception.\n";
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
            int id(id_node ? atoi(id_node->value()) : -1);
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
                if(id < 1000)
                { c->m_type = CardType::assault; }
                else if(id < 2000)
                { c->m_type = CardType::commander; }
                else if(id < 3000)
                { c->m_type = CardType::structure; }
                else
                { c->m_type = CardType::action; }
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
                    if(strcmp(skill->first_attribute("id")->value(), "augment") == 0)
                    { handle_skill<augment>(skill, c); }
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
                    if(strcmp(skill->first_attribute("id")->value(), "recharge") == 0)
                    { c->m_recharge = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "refresh") == 0)
                    { c->m_refresh = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "regenerate") == 0)
                    { c->m_regenerate = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "siphon") == 0)
                    { c->m_siphon = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "split") == 0)
                    { c->m_split = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "swipe") == 0)
                    { c->m_swipe = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "tribute") == 0)
                    { c->m_tribute = true; }
                    if(strcmp(skill->first_attribute("id")->value(), "valor") == 0)
                    { c->m_valor = atoi(skill->first_attribute("x")->value()); }
                    if(strcmp(skill->first_attribute("id")->value(), "wall") == 0)
                    { c->m_wall = true; }
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
                    if(strcmp(skill->first_attribute("id")->value(), "repair") == 0)
                    { handle_skill<repair>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "rush") == 0)
                    { handle_skill<rush>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "shock") == 0)
                    { handle_skill<shock>(skill, c); }
                    if(strcmp(skill->first_attribute("id")->value(), "siege") == 0)
                    { handle_skill<siege>(skill, c); }
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
void read_missions(Decks& decks, Cards& cards, std::string filename)
{
    std::vector<char> buffer;
    xml_document<> doc;
    parse_file(filename.c_str(), buffer, doc);
    xml_node<>* root = doc.first_node();

    if(!root)
    {
        return;
    }

    for(xml_node<>* mission_node = root->first_node();
        mission_node;
        mission_node = mission_node->next_sibling())
    {
        if(strcmp(mission_node->name(), "mission") == 0)
        {
            std::vector<unsigned> card_ids;
            xml_node<>* id_node(mission_node->first_node("id"));
            int id(id_node ? atoi(id_node->value()) : -1);
            xml_node<>* name_node(mission_node->first_node("name"));
            std::string deck_name{name_node->value()};
            xml_node<>* commander_node(mission_node->first_node("commander"));
            card_ids.push_back(atoi(commander_node->value()));
            xml_node<>* deck_node(mission_node->first_node("deck"));
            for(xml_node<>* card_node = deck_node->first_node();
                card_node;
                card_node = card_node->next_sibling())
            {
                unsigned card_id{static_cast<unsigned>(atoi(card_node->value()))};
                card_ids.push_back(card_id);
            }
            decks.mission_decks.push_back(DeckRandom{cards, card_ids});
            DeckRandom* deck = &decks.mission_decks.back();
            decks.mission_decks_by_id[id] = deck;
            decks.mission_decks_by_name[deck_name] = deck;
        }
    }
}
//------------------------------------------------------------------------------
void read_raids(Decks& decks, Cards& cards, std::string filename)
{
    std::vector<char> buffer;
    xml_document<> doc;
    parse_file(filename.c_str(), buffer, doc);
    xml_node<>* root = doc.first_node();

    if(!root)
    {
        return;
    }

    for(xml_node<>* raid_node = root->first_node();
        raid_node;
        raid_node = raid_node->next_sibling())
    {
        if(strcmp(raid_node->name(), "raid") == 0)
        {
            std::vector<const Card*> always_cards;
            std::vector<std::pair<unsigned, std::vector<const Card*> > > some_cards;
            xml_node<>* id_node(raid_node->first_node("id"));
            int id(id_node ? atoi(id_node->value()) : -1);
            xml_node<>* name_node(raid_node->first_node("name"));
            std::string deck_name{name_node->value()};
            xml_node<>* commander_node(raid_node->first_node("commander"));
            const Card* commander_card{cards.by_id(atoi(commander_node->value()))};
            xml_node<>* deck_node(raid_node->first_node("deck"));
            xml_node<>* always_node{deck_node->first_node("always_include")};
            if(always_node)
            {
                for(xml_node<>* card_node = always_node->first_node();
                    card_node;
                    card_node = card_node->next_sibling())
                {
                    unsigned card_id{static_cast<unsigned>(atoi(card_node->value()))};
                    always_cards.push_back(cards.by_id(card_id));
                }
            }
            for(xml_node<>* pool_node = always_node->next_sibling();
                pool_node;
                pool_node = pool_node->next_sibling())
            {
                if(strcmp(pool_node->name(), "card_pool") == 0)
                {
                    unsigned num_cards_from_pool{static_cast<unsigned>(atoi(pool_node->first_attribute("amount")->value()))};
                    std::vector<const Card*> cards_from_pool;

                    for(xml_node<>* card_node = pool_node->first_node();
                        card_node;
                        card_node = card_node->next_sibling())
                    {
                        unsigned card_id{static_cast<unsigned>(atoi(card_node->value()))};
                        // Special case Arctis Vanguard id 0 because of stray ` character.
                        // Don't continue on other raids because I want to be notified of other errors.
                        if(card_id == 0 && id == 1)
                        {
                            continue;
                        }
                        cards_from_pool.push_back(cards.by_id(card_id));
                    }
                    some_cards.push_back(std::make_pair(num_cards_from_pool, cards_from_pool));
                }
            }
            decks.raid_decks.push_back(DeckRandom{commander_card, always_cards, some_cards});
            DeckRandom* deck = &decks.raid_decks.back();
            decks.raid_decks_by_id[id] = deck;
            decks.raid_decks_by_name[deck_name] = deck;
        }
    }
}
//------------------------------------------------------------------------------
void read_quests(Decks& decks, Cards& cards, std::string filename)
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

    for(xml_node<>* quest_node = root->first_node();
        quest_node;
        quest_node = quest_node->next_sibling())
    {
        if(strcmp(quest_node->name(), "step") == 0)
        {
            std::vector<std::pair<unsigned, std::vector<const Card*> > > some_cards;
            xml_node<>* id_node(quest_node->first_node("id"));
            int id(id_node ? atoi(id_node->value()) : -1);
            std::string deck_name{"Step " + std::string{id_node->value()}};
            xml_node<>* commander_node(quest_node->first_node("commander"));
            const Card* commander_card{cards.by_id(atoi(commander_node->value()))};
            xml_node<>* battleground_id_node(quest_node->first_node("battleground_id"));
            int battleground_id(battleground_id_node ? atoi(battleground_id_node->value()) : -1);
            xml_node<>* deck_node(quest_node->first_node("deck"));
            for(xml_node<>* pool_node = deck_node->first_node("card_pool");
                pool_node;
                pool_node = pool_node->next_sibling())
            {
                if(strcmp(pool_node->name(), "card_pool") == 0)
                {
                    unsigned num_cards_from_pool{static_cast<unsigned>(atoi(pool_node->first_attribute("amount")->value()))};
                    std::vector<const Card*> cards_from_pool;

                    for(xml_node<>* card_node = pool_node->first_node();
                        card_node;
                        card_node = card_node->next_sibling())
                    {
                        unsigned card_id{static_cast<unsigned>(atoi(card_node->value()))};
                        cards_from_pool.push_back(cards.by_id(card_id));
                    }
                    some_cards.push_back(std::make_pair(num_cards_from_pool, cards_from_pool));
                }
            }
            decks.quest_decks.push_back(DeckRandom{commander_card, always_cards, some_cards});
            DeckRandom* deck = &decks.quest_decks.back();
            decks.quest_decks_by_id[id] = deck;
            decks.quest_effects_by_id[id] = battleground_id;
            decks.quest_decks_by_name[deck_name] = deck;
            decks.quest_effects_by_name[deck_name] = battleground_id;
        }
    }
}
