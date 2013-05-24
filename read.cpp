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

void load_decks(Decks& decks, Cards& cards)
{
    if(boost::filesystem::exists("Custom.txt"))
    {
        read_custom_decks(decks, cards, "Custom.txt");
    }
}

std::vector<std::pair<std::string, long double> > parse_deck_list(std::string list_string)
{
    std::vector<std::pair<std::string, long double> > res;
    boost::tokenizer<boost::char_delimiters_separator<char> > list_tokens{list_string, boost::char_delimiters_separator<char>{false, ";", ""}};
    for(auto list_token = list_tokens.begin(); list_token != list_tokens.end(); ++list_token)
    {
        boost::tokenizer<boost::char_delimiters_separator<char> > deck_tokens{*list_token, boost::char_delimiters_separator<char>{false, ":", ""}};
        auto deck_token = deck_tokens.begin();
        res.push_back(std::make_pair(*deck_token, 1.0d));
        ++deck_token;
        if(deck_token != deck_tokens.end())
        {
            res.back().second = boost::lexical_cast<long double>(*deck_token);
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
void parse_card_spec(const Cards& cards, std::string& card_spec, unsigned& card_id, unsigned& card_num, signed& num_sign, char& mark)
{
    static std::set<std::string> recognized_abbr;
    auto card_spec_iter = card_spec.begin();
    card_id = 0;
    card_num = 1;
    num_sign = 0;
    mark = 0;
    std::string card_name;
    card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c=='#' || c=='(' || c=='\r');}, card_name);
    if(card_name[0] == '!')
    {
        mark = card_name[0];
        card_name.erase(0, 1);
    }
    if(card_name.empty())
    {
        throw std::runtime_error("no card name");
    }
    // If card name is not found, try find card id quoted in '[]' in name, ignoring other characters.
    std::string simple_name{simplify_name(card_name)};
    auto abbr_it = cards.player_cards_abbr.find(simple_name);
    if(abbr_it != cards.player_cards_abbr.end())
    {
        if(recognized_abbr.count(card_name) == 0)
        {
            std::cout << "Recognize abbreviation " << card_name << ": " << abbr_it->second << std::endl;
            recognized_abbr.insert(card_name);
        }
        simple_name = simplify_name(abbr_it->second);
    }
    auto card_it = cards.player_cards_by_name.find(simple_name);
    auto card_id_iter = advance_until(simple_name.begin(), simple_name.end(), [](char c){return(c=='[');});
    if(card_it != cards.player_cards_by_name.end())
    {
        card_id = card_it->second->m_id;
    }
    else if(card_id_iter != simple_name.end())
    {
        ++ card_id_iter;
        card_id_iter = read_token(card_id_iter, simple_name.end(), [](char c){return(c==']');}, card_id);
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
        throw std::runtime_error("Unknown card: " + card_name);
    }
}

unsigned read_card_abbrs(Cards& cards, const std::string& filename)
{
    if(!boost::filesystem::exists(filename))
    {
        return(0);
    }
    std::ifstream abbr_file(filename);
    if(!abbr_file.is_open())
    {
        std::cerr << "Error: Card abbreviation file " << filename << " could not be opened\n";
        return(2);
    }
    unsigned num_line(0);
    abbr_file.exceptions(std::ifstream::badbit);
    try
    {
        while(abbr_file && !abbr_file.eof())
        {
            std::string abbr_string;
            getline(abbr_file, abbr_string);
            ++num_line;
            if(abbr_string.size() == 0 || strncmp(abbr_string.c_str(), "//", 2) == 0)
            {
                continue;
            }
            std::string abbr_name;
            auto abbr_string_iter = read_token(abbr_string.begin(), abbr_string.end(), [](char c){return(strchr(":", c));}, abbr_name);
            if(abbr_string_iter == abbr_string.end() || abbr_name.empty())
            {
                std::cerr << "Error in custom deck file " << filename << " at line " << num_line << ", could not read the deck name.\n";
                continue;
            }
            abbr_string_iter = advance_until(abbr_string_iter + 1, abbr_string.end(), [](const char& c){return(c != ' ');});
            if(cards.player_cards_by_name.find(abbr_name) != cards.player_cards_by_name.end())
            {
                std::cerr << "Warning in card abbreviation file " << filename << " at line " << num_line << ": ignored because the name has been used by an existing card." << std::endl;
            }
            else
            {
                cards.player_cards_abbr[abbr_name] = std::string{abbr_string_iter, abbr_string.end()};
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception while parsing the card abbreviation file " << filename;
        if(num_line > 0)
        {
            std::cerr << " at line " << num_line;
        }
        std::cerr << ": " << e.what() << ".\n";
        return(3);
    }
    return(0);
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
            deck_string_iter = advance_until(deck_string_iter + 1, deck_string.end(), [](const char& c){return(c != ' ');});
            auto deck_iter = decks.by_name.find(deck_name);
            if(deck_iter != decks.by_name.end())
            {
                std::cerr << "Warning in custom deck file " << filename << " at line " << num_line << ", name conflicts, overrides " << deck_iter->second->short_description() << std::endl;
            }
            decks.decks.push_back(Deck{DeckType::custom_deck, num_line, deck_name});
            Deck* deck = &decks.decks.back();
            deck->set(cards, std::string{deck_string_iter, deck_string.end()});
            decks.by_name[deck_name] = deck;
            std::stringstream alt_name;
            alt_name << decktype_names[deck->decktype] << " #" << deck->id;
            decks.by_name[alt_name.str()] = deck;
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
            unsigned card_id{0};
            unsigned card_num{1};
            signed num_sign{0};
            char mark{0};
            parse_card_spec(cards, card_spec, card_id, card_num, num_sign, mark);
            assert(mark == 0);
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

