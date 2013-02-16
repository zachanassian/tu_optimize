#ifndef XML_H_INCLUDED
#define XML_H_INCLUDED

#include <string>

class Cards;
class Decks;
class Achievement;

void load_decks_xml(Decks& decks, Cards& cards);
void read_cards(Cards& cards);
void read_missions(Decks& decks, Cards& cards, std::string filename);
void read_raids(Decks& decks, Cards& cards, std::string filename);
void read_quests(Decks& decks, Cards& cards, std::string filename);
void read_achievement(Decks& decks, Cards& cards, Achievement& achievement, const char* achievement_name, std::string filename="achievements.xml");

#endif
