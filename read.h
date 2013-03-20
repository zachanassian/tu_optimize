#ifndef READ_H_INCLUDED
#define READ_H_INCLUDED

#include <map>
#include <string>
#include <vector>

class Cards;
class Decks;
class Deck;

std::vector<unsigned int> deck_string_to_ids(const Cards& cards, const std::string &str);
void load_decks(Decks& decks, Cards& cards);
std::vector<std::pair<std::string, double> > parse_deck_list(std::string list_string);
unsigned read_custom_decks(Decks& decks, Cards& cards, std::string filename);
void read_owned_cards(Cards& cards, std::map<unsigned, unsigned>& owned_cards, const char *filename);

#endif
