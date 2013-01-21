#include "read.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/tokenizer.hpp>
#include <cstring>
#include <vector>
#include <fstream>

#include "card.h"
#include "cards.h"
#include "deck.h"

namespace {
const char* base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Converts `pairs' pairs of cards in `hash' to a deck.
// Stores resulting card IDs in `ids'.
void hash_to_ids(const char* hash, size_t pairs,
                 std::vector<unsigned int>& ids)
{
    unsigned int last_id = 0;

    for (size_t i = 0; i < pairs; ++i)
    {
        const char* p0 = strchr(base64_chars, hash[2 * i]);
        const char* p1 = strchr(base64_chars, hash[2 * i + 1]);
        if (!p0 || !p1)
        {
            throw std::runtime_error(hash);
        }
        size_t index0 = p0 - base64_chars;
        size_t index1 = p1 - base64_chars;
        unsigned int id = (index0 << 6) + index1;

        if (id < 4000)
        {
            ids.push_back(id);
            last_id = id;
        }
        else for (unsigned int j = 0; j < id - 4001; ++j)
        {
            ids.push_back(last_id);
        }
    }
}
}

// Constructs and returns a deck from `hash'.
// The caller is responsible for freeing the deck.
DeckIface* hash_to_deck(const char* hash, const Cards& cards)
{
    std::vector<unsigned int> ids;
    size_t pairs = strlen(hash) / 2;
    hash_to_ids(hash, pairs, ids);

    return new DeckRandom(cards, ids);
}

void load_decks(Decks& decks, Cards& cards)
{
    if(boost::filesystem::exists("Custom.txt"))
    {
        try
        {
            read_custom_decks(cards, std::string{"Custom.txt"}, decks.custom_decks);
        }
        catch(const std::runtime_error& e)
        {
            std::cout << "Exception while loading custom decks: " << e.what() << "\n";
        }
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

template<typename Iterator, typename Functor, typename Token> Iterator read_token(Iterator it, Iterator it_end, Functor f, boost::optional<Token>& token)
{
    Iterator token_start = advance_until(it, it_end, [](const char& c){return(c != ' ');});
    Iterator token_end_after_spaces = advance_until(token_start, it_end, f);
    if(token_start != token_end_after_spaces)
    {
        Iterator token_end = recede_until(token_end_after_spaces, token_start, [](const char& c){return(c != ' ');});
        token = boost::lexical_cast<Token>(std::string{token_start, token_end});
        return(token_end_after_spaces);
    }
    return(token_end_after_spaces);
}

// Error codes:
// 2 -> file not readable
// 3 -> error while parsing file
unsigned read_custom_decks(Cards& cards, std::string filename, std::map<std::string, DeckIface*>& custom_decks)
{
    std::ifstream decks_file(filename.c_str());
    if(!decks_file.is_open())
    {
        std::cerr << "File " << filename << " could not be opened\n";
        return(2);
    }
    unsigned num_line(0);
    decks_file.exceptions(std::ifstream::badbit);
    try
    {
        while(decks_file && !decks_file.eof())
        {
            std::vector<unsigned> card_ids;
            std::string deck_string;
            getline(decks_file, deck_string);
            ++num_line;
            if(deck_string.size() > 0)
            {
                if(strncmp(deck_string.c_str(), "//", 2) == 0)
                {
                    continue;
                }
                boost::tokenizer<boost::char_delimiters_separator<char> > deck_tokens{deck_string, boost::char_delimiters_separator<char>{false, ":,", ""}};
                auto token_iter = deck_tokens.begin();
                boost::optional<std::string> deck_name;
                if(token_iter != deck_tokens.end())
                {
                    read_token(token_iter->begin(), token_iter->end(), [](char c){return(false);}, deck_name);
                    if(!deck_name || (*deck_name).size() == 0)
                    {
                        std::cerr << "Error in file " << filename << " at line " << num_line << ", could not read the deck name.\n";
                        continue;
                    }
                }
                else
                {
                    std::cerr << "Error in file " << filename << " at line " << num_line << ", could not read the deck name.\n";
                    continue;
                }
                ++token_iter;
                for(; token_iter != deck_tokens.end(); ++token_iter)
                {
                    std::string card_spec(*token_iter);
                    try
                    {
                        auto card_spec_iter = card_spec.begin();
                        boost::optional<std::string> card_name;
                        card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c=='[' || c=='#' || c=='\r');}, card_name);
                        if(!card_name)
                        {
                            std::cerr << "Error in file " << filename << " at line " << num_line << " while parsing card " << card_spec << " in deck " << deck_name << "\n";
                            break;
                        }
                        else
                        {
                            boost::optional<unsigned> card_id;
                            if(*card_spec_iter == '[')
                            {
                                ++card_spec_iter;
                                card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c==']');}, card_id);
                                card_spec_iter = advance_until(card_spec_iter, card_spec.end(), [](char c){return(c!=' ');});
                            }
                            boost::optional<unsigned> card_num;
                            if(*card_spec_iter == '#')
                            {
                                ++card_spec_iter;
                                card_spec_iter = read_token(card_spec_iter, card_spec.end(), [](char c){return(c < '0' || c > '9');}, card_num);
                            }
                            unsigned resolved_id{card_id ? *card_id : 0};
                            if(resolved_id == 0)
                            {
                                auto card_it = cards.player_cards_by_name.find(*card_name);
                                if(card_it != cards.player_cards_by_name.end())
                                {
                                    resolved_id = card_it->second->m_id;
                                }
                                else
                                {
                                    std::cerr << "Error in file " << filename << " at line " << num_line << " while parsing card " << card_spec << " in deck " << *deck_name << ": card not found\n";
                                    break;
                                }
                            }
                            for(unsigned i(0); i < (card_num ? *card_num : 1); ++i)
                            {
                                card_ids.push_back(resolved_id);
                            }
                        }
                    }
                    catch(boost::bad_lexical_cast e)
                    {
                        std::cerr << "Error in file " << filename << " at line " << num_line << " while parsing card " << card_spec << " in deck " << deck_name << "\n";
                    }
                }
                if(deck_name)
                {
                    custom_decks.insert({*deck_name, new DeckRandom{cards, card_ids}});
                }
            }
        }
    }
    catch (std::ifstream::failure e)
    {
        std::cerr << "Exception while parsing the file " << filename << " (badbit is set).\n";
        e.what();
        return(3);
    }
    return(0);
}

void read_owned_cards(Cards& cards, std::map<unsigned, unsigned>& owned_cards)
{
    std::ifstream owned_file{"ownedcards.txt"};

    if(!owned_file.good())
    {
        std::cerr << "Warning: The file 'ownedcards.txt' does not exist. This will result in you not owning any cards.\n";
        return;
    }

    std::string owned_str{(std::istreambuf_iterator<char>(owned_file)), std::istreambuf_iterator<char>()};
    boost::tokenizer<boost::char_delimiters_separator<char> > tok{owned_str, boost::char_delimiters_separator<char>{false, "()\n", ""}};
    for(boost::tokenizer<boost::char_delimiters_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg)
    {
        std::string name{*beg};
        ++beg;
        assert(beg != tok.end());
        unsigned num{static_cast<unsigned>(atoi((*beg).c_str()))};
        auto card_itr = cards.player_cards_by_name.find(name);
        if(card_itr == cards.player_cards_by_name.end())
        {
            std::cerr << "Error in file ownedcards.txt, the card \"" << name << "\" does not seem to be a valid card.\n";
        }
        else
        {
            owned_cards[card_itr->second->m_id] = num;
        }
    }
}

