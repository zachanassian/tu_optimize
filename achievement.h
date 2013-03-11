#ifndef ACHIEVEMENT_H_INCLUDED
#define ACHIEVEMENT_H_INCLUDED

#include <sstream>

enum Comparator
{
    equal,
    great_equal,
    less_equal
};

class Counter
{
public:
    unsigned m_target;
    Comparator m_comparator;
public:
    Counter(unsigned target=1, Comparator comparator=great_equal): m_target(target), m_comparator(comparator) {}
    void init(unsigned target, Comparator comparator)
    {
        m_target = target;
        m_comparator = comparator;
    }
    bool check(unsigned value) const
    {
        switch(m_comparator)
        {
        case equal: return value == m_target;
        case great_equal: return value >= m_target;
        case less_equal: return value <= m_target;
        default: throw;
        }
    }
    // predict whether the monotonic increasing counter will be met: +1: true; 0: unknown; -1: false.
    int predict_monoinc(unsigned value) const
    {
        switch(m_comparator)
        {
        case equal:
        case less_equal: return value <= m_target ? 0 : -1;
        case great_equal: return value < m_target ? 0 : +1;
        default: throw;
        }
    }
    std::string str() const
    {
        std::stringstream ios;
        switch(m_comparator)
        {
        case equal: ios << " = "; break;
        case great_equal: ios << " >= "; break;
        case less_equal: ios << " <= "; break;
        default: throw;
        }
        ios << m_target;
        return ios.str();
    }
};

struct Achievement
{
    unsigned id;
    std::string name;
    Counter mission_condition;
    std::vector<Counter> req_counter;
    // Following are indexes to the req_counter
    std::map<unsigned, unsigned> skill_used;
    std::map<unsigned, unsigned> unit_played;
    std::map<unsigned, unsigned> unit_type_played;
    std::map<unsigned, unsigned> unit_faction_played;
    std::map<unsigned, unsigned> unit_rarity_played;
    std::map<unsigned, unsigned> unit_type_killed;
    std::map<unsigned, unsigned> unit_with_skill_killed;
    std::map<unsigned, unsigned> misc_req;

    Achievement() : id(0) {}
};

#endif
