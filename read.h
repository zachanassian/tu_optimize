#ifndef READ_H_INCLUDED
#define READ_H_INCLUDED

#include <map>
#include <string>
#include <vector>

class Cards;
class Decks;
class DeckIface;

DeckIface* hash_to_deck(const char* hash, const Cards& cards);
void load_decks(Decks& decks, Cards& cards);
std::vector<std::pair<std::string, double> > parse_deck_list(std::string list_string);
unsigned read_custom_decks(Cards& cards, std::string filename, std::map<std::string, DeckIface*>& custom_decks);
void read_owned_cards(Cards& cards, std::map<unsigned, unsigned>& owned_cards);

#endif
