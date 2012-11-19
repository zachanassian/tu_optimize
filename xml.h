#ifndef XML_H_INCLUDED
#define XML_H_INCLUDED

#include <string>

class Cards;
class Decks;

void read_cards(Cards& cards);
void read_missions(Decks& decks, Cards& cards, std::string filename);
void read_raids(Decks& decks, Cards& cards, std::string filename);

#endif
