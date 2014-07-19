#include <string>
#include "card.h"
#include "cards.h"

#define CUSTOM_CARD_SET (10000)

struct custom_card_skill;
struct custom_card_info;

class CustomCard: public Card {
public:
    CustomCard(unsigned id, const std::string &card_spec);
    void debug();
private:
    void add_skill(const custom_card_skill &skill);
    void handle_info(const custom_card_info &info);
};

void process_args_for_custom_cards(Cards& cards, int argc, char *argv[]);
