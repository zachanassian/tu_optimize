#ifndef READ_H_INCLUDED
#define READ_H_INCLUDED

#include <map>
#include <string>
#include <vector>

class Cards;
class Decks;
class Deck;

void parse_card_spec(const Cards& cards, std::string& card_spec, unsigned& card_id, unsigned& card_num, signed& num_sign);
void load_decks(Decks& decks, Cards& cards);
std::vector<std::pair<std::string, double> > parse_deck_list(std::string list_string);
unsigned read_custom_decks(Decks& decks, Cards& cards, std::string filename);
void read_owned_cards(Cards& cards, std::map<unsigned, unsigned>& owned_cards, const char *filename);
unsigned read_card_abbrs(Cards& cards, const std::string& filename);

#endif
