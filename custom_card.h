#include <string>
#include "card.h"
#include "cards.h"

#define CUSTOM_CARD_SET (10000)

struct custom_card_skill;
struct custom_card_info;

class CustomCard: public Card
{
public:
    CustomCard(unsigned id, const std::string &card_spec);
    void debug(const Cards& cards); // cards is used in card_description()
private:
    void add_skill(const custom_card_skill &skill);
    void handle_info(const custom_card_info &info);
};

class CustomCardReader
{
private:
    Cards& cards;
    bool quiet;
    unsigned next_card_id;

    void parse_custom_cards(const std::string& card_list);
    void read_custom_cards_file(const char *filename);
    void process_custom_cards(const char *param);

public:
    CustomCardReader(Cards& cards);
    // looks for -C= and loads into cards
    void process_args(int argc, char *argv[]);
};
