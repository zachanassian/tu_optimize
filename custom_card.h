#include <string>
#include "card.h"

class CustomCard: public Card {
    public:
        CustomCard(unsigned id, const std::string &card_spec); 

    private:
        void parse_token(const std::string &token);
};
