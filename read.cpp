#include "read.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>
#include <exception>

#include "card.h"
#include "cards.h"
#include "deck.h"

namespace {
const char* base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Converts cards in `hash' to a deck.
// Stores resulting card IDs in `ids'.
void hash_to_ids(const char* hash, std::vector<unsigned int>& ids)
{
    unsigned int last_id = 0;
    const char* pc = hash;

    while(*pc)
    {
        unsigned id_plus = 0;
        if(*pc == '-')
        {
            ++ pc;
            id_plus = 4000;
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
} // end of namespace

void parse_card_spec(const Cards& cards, std::string& card_spec, unsigned& card_id, unsigned& card_num, signed& num_sign);
void namelist_to_ids(const Cards& all_cards, const std::string& deck_string, std::vector<unsigned>& ids)
{
    boost::tokenizer<boost::char_delimiters_separator<char> > deck_tokens{deck_string, boost::char_delimiters_separator<char>{false, ":,", ""}};
    auto token_iter = deck_tokens.begin();
    for(; token_iter != deck_tokens.end(); ++token_iter)
    {
        std::string card_spec(*token_iter);
        unsigned card_id{0};
        unsigned card_num{1};
        signed num_sign{0};
        parse_card_spec(all_cards, card_spec, card_id, card_num, num_sign);
        assert(num_sign == 0);
        for(unsigned i(0); i < card_num; ++i)
        {
            ids.push_back(card_id);
        }
    }
}

// Convert `str' (either hash or name list) to ids.
std::vector<unsigned int> deck_string_to_ids(const Cards& cards, const std::string &str)
{
    std::vector<unsigned int> ids;
    if(str.find_first_of(":,") == std::string::npos)
    {
        hash_to_ids(str.c_str(), ids);
    }
    else
    {
        namelist_to_ids(cards, str, ids);
    }
    return(ids);
}

void load_decks(Decks& decks, Cards& cards)
{
    if(boost::filesystem::exists("Custom.txt"))
    {
        read_custom_decks(decks, cards, "Custom.txt");
    }
}

std::vector<std::pair<std::string, double> > parse_deck_list(std::string list_string)
{
    std::vector<std::pair<std::string, double> > res;
    boost::tokenizer<boost::char_delimiters_separator<char> > list_tokens{list_string, boost::char_delimiters_separator<char>{false, ";", ""}};
    for(auto list_token = list_tokens.begin(); list_token != list_tokens.end(); ++list_token)
    {
        boost::tokenizer<boost::char_delimiters_separator<char> > deck_tokens{*list_token, boost::char_delimiters_separator<char>{false, ":", ""}};
        auto deck_token = deck_tokens.begin();
        res.push_back(std::make_pair(*deck_token, 1.0d));
        ++deck_token;
        if(deck_token != deck_tokens.end())
        {
            res.back().second = boost::lexical_cast<double>(*deck_token);
        }
    }
    return(res);
}

template<typename Iterator, typename Functor> Iterator advance_until(Iterator it, Iterator it_end, Functor f)
{
    while(it != it_end)
    {
        if(f(*it))
        {
            break;
        }
        ++it;
    }
    return(it);
}

// take care that "it" is 1 past current.
template<typename Iterator, typename Functor> Iterator recede_until(Iterator it, Iterator it_beg, Functor f)
{
    if(it == it_beg) { return(it_beg); }
    --it;
    do
    {
        if(f(*it))
        {
            return(++it);
        }
        --it;
    } while(it != it_beg);
    return(it_beg);
}

template<typename Iterator, typename Functor, typename Token> Iterator read_token(Iterator it, Iterator it_end, Functor f, Token& token)
{
    Iterator token_start = advance_until(it, it_end, [](const char& c){return(c != ' ');});
    Iterator token_end_after_spaces = advance_until(token_start, it_end, f);
    if(token_start != token_end_after_spaces)
    {
        Iterator token_end = recede_until(token_end_after_spaces, token_start, [](const char& c){return(c != ' ');});
        token = boost::lexical_cast<Token>(std::string{token_start, token_end});
    }
    return(token_end_after_spaces);
}

// num_sign = 0 if card_num is "N"; = +1 if "+N"; = -1 if "-N"
void parse_card_spec(const Cards& cards, std::string& card_spec, unsigned& card_id, unsigned& card_num, signed& num_sign)
{
    auto card_spec_iter = card_spec.begin();
    card_id = 0;
    card_num = 1;
    num_sign = 0;
    std::string card_name;
    card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c=='#' || c=='(' || c=='\r');}, card_name);
    if(card_name.empty())
    {
        throw std::runtime_error("no card name");
    }
    // If card name is not found, try find card id quoted in '[]' in name, ignoring other characters.
    auto card_it = cards.player_cards_by_name.find(card_name);
    auto card_id_iter = advance_until(card_name.begin(), card_name.end(), [](char c){return(c=='[');});
    if(card_it != cards.player_cards_by_name.end())
    {
        card_id = card_it->second->m_id;
    }
    else if(card_id_iter != card_name.end())
    {
        ++ card_id_iter;
        card_id_iter = read_token(card_id_iter, card_name.end(), [](char c){return(c==']');}, card_id);
    }
    if(card_spec_iter != card_spec.end() && (*card_spec_iter == '#' || *card_spec_iter == '('))
    {
        ++card_spec_iter;
        if(card_spec_iter != card_spec.end())
        {
           if(*card_spec_iter == '+')
           {
               num_sign = +1;
               ++card_spec_iter;
           }
           else if(*card_spec_iter == '-')
           {
               num_sign = -1;
               ++card_spec_iter;
           }
        }
        card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c < '0' || c > '9');}, card_num);
    }
    if(card_id == 0)
    {
        throw std::runtime_error("Card not found: " + card_name);
    }
}

// Error codes:
// 2 -> file not readable
// 3 -> error while parsing file
unsigned read_custom_decks(Decks& decks, Cards& cards, std::string filename)
{
    std::ifstream decks_file(filename);
    if(!decks_file.is_open())
    {
        std::cerr << "Error: Custom deck file " << filename << " could not be opened\n";
        return(2);
    }
    unsigned num_line(0);
    decks_file.exceptions(std::ifstream::badbit);
    try
    {
        while(decks_file && !decks_file.eof())
        {
            std::string deck_string;
            getline(decks_file, deck_string);
            ++num_line;
            if(deck_string.size() == 0 || strncmp(deck_string.c_str(), "//", 2) == 0)
            {
                continue;
            }
            std::string deck_name;
            auto deck_string_iter = read_token(deck_string.begin(), deck_string.end(), [](char c){return(strchr(":,", c));}, deck_name);
            if(deck_string_iter == deck_string.end() || deck_name.empty())
            {
                std::cerr << "Error in custom deck file " << filename << " at line " << num_line << ", could not read the deck name.\n";
                continue;
            }
            auto deck_iter = decks.by_name.find(deck_name);
            if(deck_iter != decks.by_name.end())
            {
                std::cerr << "Warning in custom deck file " << filename << " at line " << num_line << ", name conflicts, overrides " << deck_iter->second->short_description() << std::endl;
            }
            decks.decks.push_back(Deck{DeckType::custom_deck, num_line, deck_name});
            try
            {
                Deck* deck = &decks.decks.back();
                deck->set(cards, deck_string_to_ids(cards, std::string{deck_string_iter, deck_string.end()}));
                decks.by_name[deck_name] = deck;
            }
            catch(std::exception& e)
            {
                std::cerr << "Error in custom deck file " << filename << " at line " << num_line << ": Deck " << deck_name << ": " << e.what() << std::endl;
                decks.decks.pop_back();
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception while parsing the custom deck file " << filename;
        if(num_line > 0)
        {
            std::cerr << " at line " << num_line;
        }
        std::cerr << ": " << e.what() << ".\n";
        return(3);
    }
    return(0);
}

void read_owned_cards(Cards& cards, std::map<unsigned, unsigned>& owned_cards, const char *filename)
{
    std::ifstream owned_file{filename};
    if(!owned_file.good())
    {
        std::cerr << "Warning: Owned cards file '" << filename << "' does not exist.\n";
        return;
    }
    unsigned num_line(0);
    while(owned_file && !owned_file.eof())
    {
        std::string card_spec;
        getline(owned_file, card_spec);
        ++num_line;
        if(card_spec.size() == 0 || strncmp(card_spec.c_str(), "//", 2) == 0)
        {
            continue;
        }
        try
        {
            // Remove ',' from card names
            auto pos = card_spec.find(',');
            if(pos != std::string::npos)
            {
                card_spec.erase(pos, 1);
            }
            unsigned card_id{0};
            unsigned card_num{1};
            signed num_sign{0};
            parse_card_spec(cards, card_spec, card_id, card_num, num_sign);
            if(num_sign == 0)
            {
                owned_cards[card_id] = card_num;
            }
            else if(num_sign > 0)
            {
                owned_cards[card_id] += card_num;
            }
            else if(num_sign < 0)
            {
                owned_cards[card_id] = owned_cards[card_id] > card_num ? owned_cards[card_id] - card_num : 0;
            }
        }
        catch(std::exception& e)
        {
            std::cerr << "Error in owned cards file " << filename << " at line " << num_line << " while parsing card '" << card_spec << "': " << e.what() << "\n";
        }
    }
}

