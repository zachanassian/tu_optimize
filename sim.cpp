#include "sim.h"

#include <boost/range/adaptors.hpp>
#include <random>
#include <string>
#include <sstream>
#include <vector>

#include "card.h"
#include "cards.h"
#include "deck.h"

//---------------------- $00 general stuff -------------------------------------
template<typename T>
std::string to_string(T val)
{
    std::stringstream s;
    s << val;
    return s.str();
}
//---------------------- Debugging stuff ---------------------------------------
bool debug_print(false);
bool debug_line(true);
#ifndef NDEBUG
#define _DEBUG_MSG(format, args...)                                     \
    {                                                                   \
        if(debug_print)                                                 \
        {                                                               \
            if(debug_line) { printf("%i - " format, __LINE__ , ##args); }      \
            else { printf(format, ##args); }                           \
            std::cout << std::flush;                                    \
        }                                                               \
    }
#else
#define _DEBUG_MSG(format, args...)
#endif
//------------------------------------------------------------------------------

CardStatus::CardStatus(const Card* card) :
    m_card(card),
    m_index(0),
    m_player(0),
    m_augmented(0),
    m_berserk(0),
    m_blitzing(false),
    m_chaosed(false),
    m_delay(card->m_delay),
    m_diseased(false),
    m_enfeebled(0),
    m_faction(card->m_faction),
    m_frozen(false),
    m_hp(card->m_health),
    m_immobilized(false),
    m_infused(false),
    m_jammed(false),
    m_poisoned(0),
    m_protected(0),
    m_rallied(0),
    m_weakened(0),
    m_temporary_split(false)
{
}

//------------------------------------------------------------------------------
inline void CardStatus::set(const Card* card)
{
    this->set(*card);
}
//------------------------------------------------------------------------------
inline void CardStatus::set(const Card& card)
{
    m_card = &card;
    m_index = 0;
    m_player = 0;
    m_augmented = 0;
    m_berserk = 0;
    m_blitzing = false;
    m_chaosed = false;
    m_delay = card.m_delay;
    m_diseased = false;
    m_enfeebled = 0;
    m_faction = card.m_faction;
    m_frozen = false;
    m_hp = card.m_health;
    m_immobilized = false;
    m_infused = false;
    m_jammed = false;
    m_poisoned = 0;
    m_protected = 0;
    m_rallied = 0;
    m_weakened = 0;
    m_temporary_split = false;
}
//------------------------------------------------------------------------------
inline unsigned safe_minus(unsigned x, unsigned y)
{
    return(x - std::min(x, y));
}
inline int attack_power(CardStatus* att)
{
    return(safe_minus(att->m_card->m_attack + att->m_berserk + att->m_rallied, att->m_weakened));
}
//------------------------------------------------------------------------------
std::string skill_description(Field* fd, const SkillSpec& s)
{
    switch(std::get<0>(s))
    {
    case summon:
        return(skill_names[std::get<0>(s)] + " " + fd->cards.by_id(std::get<1>(s))->m_name.c_str());
    default:
        return(skill_names[std::get<0>(s)] +
           (std::get<2>(s) == allfactions ? "" : std::string(" ") + faction_names[std::get<2>(s)]) +
           (std::get<1>(s) == 0 ? "" : std::string(" ") + to_string(std::get<1>(s))));
    }
}
//------------------------------------------------------------------------------
std::string card_description(Field* fd, const Card* c)
{
    std::string desc;
    desc = c->m_name;
    switch(c->m_type)
    {
    case CardType::action:
        break;
    case CardType::assault:
        desc += " " + to_string(c->m_attack) + "/" + to_string(c->m_health) + "/" + to_string(c->m_delay);
        break;
    case CardType::structure:
        desc += " " + to_string(c->m_health) + "/" + to_string(c->m_delay);
        break;
    case CardType::commander:
        desc += " hp:" + to_string(c->m_health);
        break;
    case CardType::num_cardtypes:
        assert(false);
        break;
    }
    if(c->m_unique) { desc += " unique"; }
    if(c->m_rarity == 4) { desc += " legendary"; }
    if(c->m_faction != allfactions) { desc += " " + faction_names[c->m_faction]; }
    if(c->m_antiair > 0) { desc += ", antiair " + to_string(c->m_antiair); }
    if(c->m_armored > 0) { desc += ", armored " + to_string(c->m_armored); }
    if(c->m_berserk > 0) { desc += ", berserk " + to_string(c->m_berserk); }
    if(c->m_blitz) { desc += ", blitz"; }
    if(c->m_burst > 0) { desc += ", burst " + to_string(c->m_burst); }
    if(c->m_counter > 0) { desc += ", counter " + to_string(c->m_counter); }
    if(c->m_crush > 0) { desc += ", crush " + to_string(c->m_crush); }
    if(c->m_disease) { desc += ", disease"; }
    if(c->m_emulate) { desc += ", emulate"; }
    if(c->m_evade) { desc += ", evade"; }
    if(c->m_fear) { desc += ", fear"; }
    if(c->m_flurry > 0) { desc += ", flurry " + to_string(c->m_flurry); }
    if(c->m_flying) { desc += ", flying"; }
    if(c->m_fusion) { desc += ", fusion"; }
    if(c->m_immobilize) { desc += ", immobilize"; }
    if(c->m_intercept) { desc += ", intercept"; }
    if(c->m_leech > 0) { desc += ", leech " + to_string(c->m_leech); }
    if(c->m_payback) { desc += ", payback"; }
    if(c->m_pierce > 0) { desc += ", pierce " + to_string(c->m_pierce); }
    if(c->m_poison > 0) { desc += ", poison " + to_string(c->m_poison); }
    if(c->m_recharge) { desc += ", recharge"; }
    if(c->m_refresh) { desc += ", refresh"; }
    if(c->m_regenerate > 0) { desc += ", regenerate " + to_string(c->m_regenerate); }
    if(c->m_siphon > 0) { desc += ", siphon " + to_string(c->m_siphon); }
    if(c->m_split) { desc += ", split"; }
    if(c->m_swipe) { desc += ", swipe"; }
    if(c->m_tribute) { desc += ", tribute"; }
    if(c->m_valor > 0) { desc += ", valor " + to_string(c->m_valor); }
    if(c->m_wall) { desc += ", wall"; }
    for(auto& skill: c->m_skills) { desc += ", " + skill_description(fd, skill); }
    for(auto& skill: c->m_skills_on_play) { desc += ", " + skill_description(fd, skill) + " on play"; }
    for(auto& skill: c->m_skills_on_kill) { desc += ", " + skill_description(fd, skill) + " on kill"; }
    if(c->m_berserk_oa > 0) { desc += ", berserk " + to_string(c->m_berserk_oa) + " on attacked"; }
    if(c->m_disease_oa) { desc += ", disease on attacked"; }
    if(c->m_poison_oa > 0) { desc += ", poison " + to_string(c->m_poison_oa) + " on attacked"; }
    for(auto& skill: c->m_skills_on_attacked) { desc += ", " + skill_description(fd, skill) + " on attacked"; }
    for(auto& skill: c->m_skills_on_death) { desc += ", " + skill_description(fd, skill) + " on death"; }
    return(desc);
}
//------------------------------------------------------------------------------
std::string CardStatus::description()
{
    std::string desc;
    switch(m_card->m_type)
    {
    case CardType::commander: desc = "Commander "; break;
    case CardType::action: desc = "Action "; break;
    case CardType::assault: desc = "Assault " + to_string(m_index) + " "; break;
    case CardType::structure: desc = "Structure " + to_string(m_index) + " "; break;
    case CardType::num_cardtypes: assert(false); break;
    }
    desc += "[" + m_card->m_name;
    switch(m_card->m_type)
    {
    case CardType::action:
        break;
    case CardType::assault:
        desc += " att:" + to_string(m_card->m_attack);
        {
            std::string att_desc;
            if(m_berserk > 0) { att_desc += "+" + to_string(m_berserk) + "(berserk)"; }
            if(m_rallied > 0) { att_desc += "+" + to_string(m_rallied) + "(rallied)"; }
            if(m_weakened > 0) { att_desc += "-" + to_string(m_weakened) + "(weakened)"; }
            if(!att_desc.empty()) { desc += att_desc + "=" + to_string(attack_power(this)); }
        }
    case CardType::structure:
    case CardType::commander:
        desc += " hp:" + to_string(m_hp);
        break;
    case CardType::num_cardtypes:
        assert(false);
        break;
    }
    if(m_delay > 0) {
        desc += " cd:" + to_string(m_delay);
        if(m_blitzing) { desc += "(blitzing)"; }
    }
    if(m_chaosed) { desc += ", chaosed"; }
    if(m_diseased) { desc += ", diseaseded"; }
    if(m_frozen) { desc += ", frozen"; }
    if(m_immobilized) { desc += ", immobilized"; }
    if(m_infused) { desc += ", infused"; }
    if(m_jammed) { desc += ", jammed"; }
    if(m_temporary_split) { desc += ", cloning"; }
    if(m_augmented > 0) { desc += ", augmented " + to_string(m_augmented); }
    if(m_enfeebled > 0) { desc += ", enfeebled " + to_string(m_enfeebled); }
    if(m_poisoned > 0) { desc += ", poisoned " + to_string(m_poisoned); }
    if(m_protected > 0) { desc += ", protected " + to_string(m_protected); }
    desc += "]";
    return(desc);
}
//------------------------------------------------------------------------------
std::string status_description(CardStatus* status)
{
    if (!status) { return("A vanished card"); }  // Unknown Action card?
    return status->description();
}
//------------------------------------------------------------------------------
void Hand::reset(std::mt19937& re)
{
    assaults.reset();
    structures.reset();
    commander = CardStatus(deck->get_commander());
    deck->shuffle(re);
}
//---------------------- $40 Game rules implementation -------------------------
// Everything about how a battle plays out, except the following:
// the implementation of the attack by an assault card is in the next section;
// the implementation of the active skills is in the section after that.
unsigned turn_limit{50};
bool win_tie{false};
//------------------------------------------------------------------------------
inline unsigned opponent(unsigned player)
{
    return((player + 1) % 2);
}
//------------------------------------------------------------------------------
void prepend_on_death(Field* fd)
{
    for(auto status: boost::adaptors::reverse(fd->killed_with_on_death))
    {
        for(auto& skill: boost::adaptors::reverse(status->m_card->m_skills_on_death))
        {
            _DEBUG_MSG("On death skill pushed in front: %s\n", skill_description(fd, skill).c_str());
            fd->skill_queue.emplace_front(status, skill);
        }
    }
    fd->killed_with_on_death.clear();
}
//------------------------------------------------------------------------------
void(*skill_table[num_skills])(Field*, CardStatus* src_status, const SkillSpec&);
void resolve_skill(Field* fd)
{
    while(!fd->skill_queue.empty())
    {
        auto skill_instance(fd->skill_queue.front());
        auto& status(std::get<0>(skill_instance));
        auto& skill(std::get<1>(skill_instance));
        fd->skill_queue.pop_front();
        skill_table[std::get<0>(skill)](fd, status, skill);
    }
}
//------------------------------------------------------------------------------
void attack_phase(Field* fd);
SkillSpec augmented_skill(CardStatus* status, const SkillSpec& s)
{
    if (std::get<0>(s) == augment || std::get<0>(s) == augment_all || std::get<0>(s) == summon || std::get<1>(s) == 0)
    {
        return s;
    }
    SkillSpec augmented_s = s;
    std::get<1>(augmented_s) += status->m_augmented;
    return(augmented_s);
}
SkillSpec fusioned_skill(const SkillSpec& s)
{
    SkillSpec fusioned_s = s;
    std::get<1>(fusioned_s) *= 2;
    return(fusioned_s);
}
SkillSpec infused_skill(const SkillSpec& s)
{
    if (std::get<2>(s) == allfactions || std::get<2>(s) == bloodthirsty || helpful_skills.find(std::get<0>(s)) == helpful_skills.end())
    {
        return s;
    }
    SkillSpec infused_s = s;
    std::get<2>(infused_s) = bloodthirsty;
    return(infused_s);
}
void evaluate_skills(Field* fd, CardStatus* status, const std::vector<SkillSpec>& skills)
{
    assert(status);
    assert(fd->skill_queue.size() == 0);
    for(auto& skill: skills)
    {
        _DEBUG_MSG("Evaluating %s skill %s\n", status_description(status).c_str(), skill_description(fd, skill).c_str());
        bool fusion_active = status->m_card->m_fusion && status->m_player == fd->tapi && fd->fusion_count >= 3;
        auto& augmented_s = status->m_augmented > 0 ? augmented_skill(status, skill) : skill;
        auto& fusioned_s = fusion_active ? fusioned_skill(augmented_s) : augmented_s;
        auto& infused_s = status->m_infused ? infused_skill(fusioned_s) : fusioned_s;
        fd->skill_queue.emplace_back(status, infused_s);
        resolve_skill(fd);
    }
}
struct PlayCard
{
    const Card* card;
    Field* fd;
    CardStatus* status;
    Storage<CardStatus>* storage;

    PlayCard(const Card* card_, Field* fd_) :
        card{card_},
        fd{fd_},
        status{nullptr},
        storage{nullptr}
    {}

    template <enum CardType::CardType type>
    bool op()
    {
        setStorage<type>();
        placeCard<type>();
        placeDebugMsg<type>();
        onPlaySkills<type>();
        blitz<type>();
        fieldEffects<type>();
        return(true);
    }

    // action
    template <enum CardType::CardType>
    void setStorage()
    {
    }

    // assault + structure
    template <enum CardType::CardType type>
    void placeCard()
    {
        status = &storage->add_back();
        status->set(card);
        status->m_index = storage->size() - 1;
        status->m_player = fd->tapi;
		status->m_temporary_split = false;
        if(fd->turn == 1 && fd->gamemode == tournament && status->m_delay > 0)
        {
            ++status->m_delay;
        }
    }

    // all
    template <enum CardType::CardType type>
    void placeDebugMsg()
    {
        if (storage)
        {
            _DEBUG_MSG("%s plays %s %u [%s]\n", status_description(&fd->tap->commander).c_str(), cardtype_names[type].c_str(), storage->size() - 1, card_description(fd, card).c_str());
        }
        else
        {
            _DEBUG_MSG("%s plays %s [%s]\n", status_description(&fd->tap->commander).c_str(), cardtype_names[type].c_str(), card_description(fd, card).c_str());
        }
    }

    // all except assault: noop
    template <enum CardType::CardType>
    void blitz()
    {
    }

    // all except assault: noop
    template <enum CardType::CardType>
    void fieldEffects()
    {
    }

    // assault + structure
    template <enum CardType::CardType>
    void onPlaySkills()
    {
        for(auto& skill: card->m_skills_on_play)
        {
            _DEBUG_MSG("Evaluating %s skill %s on play\n", status_description(status).c_str(), skill_description(fd, skill).c_str());
            fd->skill_queue.emplace_back(status, skill);
            resolve_skill(fd);
        }
    }
};
// assault
template <>
void PlayCard::setStorage<CardType::assault>()
{
    storage = &fd->tap->assaults;
}
// structure
template <>
void PlayCard::setStorage<CardType::structure>()
{
    storage = &fd->tap->structures;
}
// action
template <>
void PlayCard::placeCard<CardType::action>()
{
}
// assault
template <>
void PlayCard::blitz<CardType::assault>()
{
    if(card->m_blitz && fd->tip->assaults.size() > status->m_index && fd->tip->assaults[status->m_index].m_hp > 0 && fd->tip->assaults[status->m_index].m_delay == 0)
    {
        status->m_blitzing = true;
    }
}
// assault
template <>
void PlayCard::fieldEffects<CardType::assault>()
{
    if(fd->effect == Effect::toxic)
    {
        status->m_poisoned = 1;
    }
    else if(fd->effect == Effect::decay)
    {
        status->m_poisoned = 1;
        status->m_diseased = true;
    }
}
// action
template <>
void PlayCard::onPlaySkills<CardType::action>()
{
    for(auto& skill: card->m_skills)
    {
        _DEBUG_MSG("Evaluating %s skill %s\n", status_description(status).c_str(), skill_description(fd, skill).c_str());
        fd->skill_queue.emplace_back(nullptr, skill);
        resolve_skill(fd);
        // Special case: enemy commander killed by a shock action card
        if(fd->tip->commander.m_hp == 0)
        {
            fd->end = true;
            break;
        }
    }
    // Special case: recharge ability
    if(card->m_recharge && fd->flip())
    {
        fd->tap->deck->place_at_bottom(card);
    }
}
//------------------------------------------------------------------------------
void turn_start_phase(Field* fd);
void prepend_on_death(Field* fd);
// return value : (raid points) -> attacker wins, 0 -> defender wins
unsigned play(Field* fd)
{
    fd->players[0]->commander.m_player = 0;
    fd->players[1]->commander.m_player = 1;
    fd->tapi = fd->gamemode == surge ? 1 : 0;
    fd->tipi = opponent(fd->tapi);
    fd->tap = fd->players[fd->tapi];
    fd->tip = fd->players[fd->tipi];
    fd->fusion_count = 0;
    fd->end = false;

    // ANP: Last decision point is second-to-last card played.
    fd->points_since_last_decision = 0;
    unsigned p0_size = fd->players[0]->deck->cards.size();
    fd->last_decision_turn = p0_size * 2 - (surge ? 2 : 3);

    // Shuffle deck
    while(fd->turn <= turn_limit && !fd->end)
    {
        fd->current_phase = Field::playcard_phase;
        // Initialize stuff, remove dead cards
        _DEBUG_MSG("------------------------------------------------------------------------\n");
        _DEBUG_MSG("TURN %u begins for %s\n", fd->turn, status_description(&fd->tap->commander).c_str());
        // ANP: If it's the player's turn and he's making a decision,
        // reset his points to 0.
        if(fd->tapi == 0 && fd->turn <= fd->last_decision_turn)
        {
            fd->points_since_last_decision = 0;
        }
        turn_start_phase(fd);
        // Special case: refresh on commander
        if(fd->tip->commander.m_card->m_refresh && fd->tip->commander.m_hp > 0)
        {
            fd->tip->commander.m_hp = fd->tip->commander.m_card->m_health;
        }

        if(fd->effect == Effect::clone_project ||
           (fd->effect == Effect::clone_experiment && (fd->turn == 9 || fd->turn == 10)))
        {
            std::vector<SkillSpec> skills;
            skills.emplace_back(temporary_split, 0, allfactions);
            // The skill doesn't actually come from the commander,
            // but we need to provide some source and it seemed most reasonable.
            evaluate_skills(fd, &fd->tap->commander, skills);
        }

        // Play a card
        const Card* played_card(fd->tap->deck->next());
        if(played_card)
        {
            switch(played_card->m_type)
            {
            case CardType::action:
                // end: handles commander death by shock
                PlayCard(played_card, fd).op<CardType::action>();
                break;
            case CardType::assault:
                PlayCard(played_card, fd).op<CardType::assault>();
                break;
            case CardType::structure:
                PlayCard(played_card, fd).op<CardType::structure>();
                break;
            case CardType::commander:
            case CardType::num_cardtypes:
                assert(false);
                break;
            }
        }
        // Evaluate commander
        fd->current_phase = Field::commander_phase;
        evaluate_skills(fd, &fd->tap->commander, fd->tap->commander.m_card->m_skills);

        if (fd->effect == Effect::genesis)
        {
            unsigned index(fd->rand(0, fd->cards.player_assaults.size() - 1));
            std::vector<SkillSpec> skills;
            skills.emplace_back(summon, fd->cards.player_assaults[index]->m_id, allfactions);
            evaluate_skills(fd, &fd->tap->commander, skills);
        }

        // Evaluate structures
        fd->current_phase = Field::structures_phase;
        for(fd->current_ci = 0; !fd->end && fd->current_ci < fd->tap->structures.size(); ++fd->current_ci)
        {
            CardStatus& current_status(fd->tap->structures[fd->current_ci]);
            if(current_status.m_delay == 0)
            {
                evaluate_skills(fd, &current_status, current_status.m_card->m_skills);
            }
        }
        // Evaluate assaults
        fd->current_phase = Field::assaults_phase;
        for(fd->current_ci = 0; !fd->end && fd->current_ci < fd->tap->assaults.size(); ++fd->current_ci)
        {
            // ca: current assault
            CardStatus& current_status(fd->tap->assaults[fd->current_ci]);
            if((current_status.m_delay > 0 && !current_status.m_blitzing) || current_status.m_hp == 0 || current_status.m_jammed || current_status.m_frozen)
            {
                //_DEBUG_MSG("! Assault %u (%s) hp: %u, jammed %u\n", card_index, current_status.m_card->m_name.c_str(), current_status.m_hp, current_status.m_jammed);
            }
            else
            {
                // Special case: check for split (tartarus swarm raid, or clone battlefield effects)
                if((current_status.m_temporary_split || current_status.m_card->m_split) && fd->tap->assaults.size() + fd->tap->structures.size() < 100)
                {
                    CardStatus& status_split(fd->tap->assaults.add_back());
                    status_split.set(current_status.m_card);
                    status_split.m_index = fd->tap->assaults.size() - 1;
                    status_split.m_player = fd->tapi;
                    _DEBUG_MSG("%s splits %s\n", status_description(&current_status).c_str(), status_description(&status_split).c_str());
                    for(auto& skill: status_split.m_card->m_skills_on_play)
                    {
                        _DEBUG_MSG("Evaluating %s skill %s on play\n", status_description(&current_status).c_str(), skill_description(fd, skill).c_str());
                        fd->skill_queue.emplace_back(&status_split, skill);
                        resolve_skill(fd);
                    }
                    // TODO: Use summon to implement split?
                    // TODO: Determine whether we need to check for Blitz for the newly-Split unit
                }
                // Evaluate skills
                evaluate_skills(fd, &current_status, current_status.m_card->m_skills);
                // Attack
                if(!current_status.m_immobilized && current_status.m_hp > 0)
                {
                    attack_phase(fd);
                }
            }
        }
        _DEBUG_MSG("TURN %u ends for %s\n", fd->turn, status_description(&fd->tap->commander).c_str());
        std::swap(fd->tapi, fd->tipi);
        std::swap(fd->tap, fd->tip);
        ++fd->turn;
    }
    // defender wins
    if(fd->players[0]->commander.m_hp == 0 || (!win_tie && fd->turn > turn_limit))
    {
        _DEBUG_MSG("Defender wins.\n");
        return(0);
    }
    // attacker wins
    if(fd->players[1]->commander.m_hp == 0 || (win_tie && fd->turn > turn_limit))
    {
        // ANP: Speedy if last_decision + 10 > turn.
        // fd->turn has advanced once past the actual turn the battle has ended.
        // So we were speedy if last_decision + 10 > (fd->turn - 1),
        // or, equivalently, if last_decision + 10 >= fd->turn.
        bool speedy = fd->last_decision_turn + 10 >= fd->turn;
        if(fd->points_since_last_decision > 10)
        {
            fd->points_since_last_decision = 10;
        }
        _DEBUG_MSG("Attacker wins.\n");
        return(10 + (speedy ? 5 : 0) + fd->points_since_last_decision);
    }

    // Huh? How did we get here?
    assert(false);
    return 0;
}
//------------------------------------------------------------------------------
// All the stuff that happens at the beginning of a turn, before a card is played
// returns true iff the card died.
void remove_hp(Field* fd, CardStatus& status, unsigned dmg)
{
    assert(status.m_hp > 0);
    _DEBUG_MSG("%s takes %u damage\n", status_description(&status).c_str(), dmg);
    status.m_hp = safe_minus(status.m_hp, dmg);
    if(status.m_hp == 0)
    {
        _DEBUG_MSG("%s dies\n", status_description(&status).c_str());
        if(status.m_card->m_skills_on_death.size() > 0)
        {
            fd->killed_with_on_death.push_back(&status);
        }
        if(status.m_card->m_regenerate)
        {
            fd->killed_with_regen.push_back(&status);
        }
    }
}
inline bool is_it_dead(CardStatus& c)
{
    if(c.m_hp == 0) // yes it is
    {
        _DEBUG_MSG("Dead: %s\n", status_description(&c).c_str());
        return(true);
    }
    else { return(false); } // nope still kickin'
}
inline void remove_dead(Storage<CardStatus>& storage)
{
    storage.remove(is_it_dead);
}
inline void add_hp(Field* field, CardStatus* target, unsigned v)
{
    unsigned old_hp = target->m_hp;
    target->m_hp = std::min(target->m_hp + v, target->m_card->m_health);
    if(field->effect == Effect::invigorate && target->m_card->m_type == CardType::assault)
    {
        unsigned healed = target->m_hp - old_hp;
        target->m_berserk += healed;
    }
}
void check_regeneration(Field* fd)
{
    for(unsigned i(0); i < fd->killed_with_regen.size(); ++i)
    {
        CardStatus& status = *fd->killed_with_regen[i];
        if(status.m_hp == 0 && status.m_card->m_regenerate > 0 && !status.m_diseased)
        {
            if (fd->flip())
            {
                _DEBUG_MSG("%s regenerates, hp 0 -> %u\n", status_description(&status).c_str(), status.m_card->m_health);
                add_hp(fd, &status, status.m_card->m_regenerate);
            }
        }
    }
    fd->killed_with_regen.clear();
}
void turn_start_phase(Field* fd)
{
    remove_dead(fd->tap->assaults);
    remove_dead(fd->tap->structures);
    remove_dead(fd->tip->assaults);
    remove_dead(fd->tip->structures);
    fd->fusion_count = 0;
    // Active player's assault cards:
    // update index
    // remove enfeeble, protect; apply poison damage, reduce delay
    {
        auto& assaults(fd->tap->assaults);
        for(unsigned index(0), end(assaults.size());
            index < end;
            ++index)
        {
            CardStatus& status(assaults[index]);
            status.m_index = index;
            status.m_enfeebled = 0;
            status.m_protected = 0;
            if(status.m_poisoned > 0)
            {
                _DEBUG_MSG("%s takes poison damage\n", status_description(&status).c_str());
                remove_hp(fd, status, status.m_poisoned);
            }
            if(status.m_delay > 0 && !status.m_frozen)
            {
                _DEBUG_MSG("%s reduces its timer\n", status_description(&status).c_str());
                --status.m_delay;
            }
            if(status.m_card->m_fusion && status.m_delay == 0) { ++fd->fusion_count; }
        }
    }
    // Active player's structure cards:
    // update index
    // reduce delay
    {
        auto& structures(fd->tap->structures);
        for(unsigned index(0), end(structures.size());
            index < end;
            ++index)
        {
            CardStatus& status(structures[index]);
            status.m_index = index;
            if(status.m_delay > 0)
            {
                _DEBUG_MSG("%s reduces its timer\n", status_description(&status).c_str());
                --status.m_delay;
            }
            if(status.m_card->m_fusion && status.m_delay == 0) { ++fd->fusion_count; }
        }
    }
    // Defending player's assault cards:
    // update index
    // remove augment, chaos, freeze, immobilize, jam, rally, weaken, apply refresh
    // remove temp split
    {
        auto& assaults(fd->tip->assaults);
        for(unsigned index(0), end(assaults.size());
            index < end;
            ++index)
        {
            CardStatus& status(assaults[index]);
            status.m_index = index;
            status.m_augmented = 0;
            status.m_blitzing = false;
            status.m_chaosed = false;
            status.m_frozen = false;
            status.m_immobilized = false;
            status.m_jammed = false;
            status.m_rallied = 0;
            status.m_weakened = 0;
            status.m_temporary_split = false;
            if(status.m_card->m_refresh && status.m_hp < status.m_card->m_health && !status.m_diseased)
            {
                _DEBUG_MSG("%s refreshes. hp -> %u.\n", status_description(&status).c_str(), status.m_card->m_health);
                add_hp(fd, &status, status.m_card->m_health);
            }
        }
    }
    // Defending player's structure cards:
    // update index
    // apply refresh
    {
        auto& structures(fd->tip->structures);
        for(unsigned index(0), end(structures.size());
            index < end;
            ++index)
        {
            CardStatus& status(structures[index]);
            status.m_index = index;
            if(status.m_card->m_refresh && status.m_hp < status.m_card->m_health)
            {
                _DEBUG_MSG("%s refreshes. hp -> %u.\n", status_description(&status).c_str(), status.m_card->m_health);
                add_hp(fd, &status, status.m_card->m_health);
            }
        }
    }
    // Perform on death skills (from cards killed by poison damage)
    prepend_on_death(fd);
    resolve_skill(fd);
    // Regen from poison
    check_regeneration(fd);
}
//---------------------- $50 attack by assault card implementation -------------
inline void apply_poison(CardStatus* target, unsigned v)
{
    if(v <= target->m_poisoned) { return; }
    _DEBUG_MSG("%s is poisoned (%u)\n", status_description(target).c_str(), v);
    target->m_poisoned = v;
}
// Counter damage dealt to the attacker (att) by defender (def)
// pre-condition: only valid if m_card->m_counter > 0
inline unsigned counter_damage(CardStatus* att, CardStatus* def)
{
    assert(att->m_card->m_type == CardType::assault);
    assert(def->m_card->m_type != CardType::action);
    return(safe_minus(def->m_card->m_counter + att->m_enfeebled, att->m_protected));
}
inline CardStatus* select_first_enemy_wall(Field* fd)
{
    for(unsigned i(0); i < fd->tip->structures.size(); ++i)
    {
        CardStatus& card(fd->tip->structures[i]);
        if(card.m_card->m_wall && card.m_hp > 0) { return(&card); }
    }
    return(nullptr);
}

inline unsigned valor_damage(Field* fd, CardStatus& status)
{
    if(status.m_card->m_valor > 0)
    {
        unsigned count_ta(0);
        unsigned count_td(0);
        for(unsigned i(0); i < fd->tap->assaults.size(); ++i)
        {
            if(fd->tap->assaults[i].m_hp > 0) { ++count_ta; }
        }
        for(unsigned i(0); i < fd->tip->assaults.size(); ++i)
        {
            if(fd->tip->assaults[i].m_hp > 0) { ++count_td; }
        }
        if(count_ta < count_td) { return(status.m_card->m_valor); }
    }
    return(0);
}

inline bool alive_assault(Storage<CardStatus>& assaults, unsigned index)
{
    return(assaults.size() > index && assaults[index].m_hp > 0);
}

void remove_commander_hp(Field* fd, CardStatus& status, unsigned dmg)
{
    assert(status.m_hp > 0);
    assert(status.m_card->m_type == CardType::commander);
    _DEBUG_MSG("%s takes %u damage\n", status_description(&status).c_str(), dmg);
    status.m_hp = safe_minus(status.m_hp, dmg);
    // ANP: If commander is enemy's, player gets points equal to damage.
    // Points are awarded for overkill, so it is correct to simply add dmg.
    if(status.m_player == 1)
    {
        fd->points_since_last_decision += dmg;
    }
    if(status.m_hp == 0)
    {
        _DEBUG_MSG("%s dies\n", status_description(&status).c_str());
        fd->end = true;
    }
}
//------------------------------------------------------------------------------
// implementation of one attack by an assault card, against either an enemy
// assault card, the first enemy wall, or the enemy commander.
struct PerformAttack
{
    Field* fd;
    CardStatus* att_status;
    CardStatus* def_status;
    unsigned att_dmg;
    bool killed_by_attack;

    PerformAttack(Field* fd_, CardStatus* att_status_, CardStatus* def_status_) :
        fd(fd_), att_status(att_status_), def_status(def_status_), att_dmg(0), killed_by_attack(false)
    {}

    template<enum CardType::CardType cardtype>
    void op()
    {
        att_dmg = calculate_attack_damage<cardtype>();
        if(att_dmg == 0) { return; }
        // Evaluation order:
        // assaults only: fly check
        // assaults only: immobilize
        // deal damage
        // assaults only: (siphon, poison, disease, on_kill)
        // on_attacked: poison, disease, assaults only: berserk, skills
        // counter, berserk
        // assaults only: (crush, leech if still alive)
        // check regeneration
        const bool dodge_by_fly(def_status->m_card->m_flying && !att_status->m_card->m_flying && !att_status->m_card->m_antiair > 0 &&
                         (fd->effect == Effect::high_skies || fd->flip()));
        if(dodge_by_fly) // unnecessary check for structures, commander -> fix later ?
        {
            _DEBUG_MSG("%s dodges with flying\n", status_description(def_status).c_str());
            return;
        }

        // If Impenetrable, force attack damage against walls to be 0,
        // but still activate Counter!
        if(fd->effect == Effect::impenetrable && def_status->m_card->m_wall)
        {
            att_dmg = 0;
        }

        if(att_dmg > 0)
        {
            immobilize<cardtype>();
            attack_damage<cardtype>();
            siphon_poison_disease<cardtype>();
            on_kill<cardtype>();
        }
        on_attacked<cardtype>();
        if(att_dmg > 0)
        {
            if(att_status->m_hp > 0)
            {
                counter<cardtype>();
                berserk<cardtype>();
            }
            crush_leech<cardtype>();
        }

        // If Impenetrable, force attack damage against walls to be 0,
        // but still activate Counter!
        if(fd->effect == Effect::impenetrable && def_status->m_card->m_wall && att_status->m_hp > 0)
        {
            counter<cardtype>();
        }

        prepend_on_death(fd);
        resolve_skill(fd);
        check_regeneration(fd);
    }

    template<enum CardType::CardType>
    unsigned calculate_attack_damage()
    {
        const Card& att_card(*att_status->m_card);
        const Card& def_card(*def_status->m_card);
        assert(att_card.m_type == CardType::assault);
        // pre modifier damage
        unsigned damage(attack_power(att_status));
        if(damage == 0) { return(0); }
        unsigned modified_damage = safe_minus(
                damage // pre-modifier damage
                + valor_damage(fd, *att_status) // valor
                + def_status->m_enfeebled // enfeeble
                + (def_card.m_flying ? att_card.m_antiair : 0) // anti-air
                + (def_status->m_hp == def_card.m_health ? att_card.m_burst : 0) // burst
                // armor + protect + pierce
                , safe_minus(def_card.m_armored + def_status->m_protected, att_card.m_pierce));
        if(debug_print)
        {
            std::string desc;
            if(valor_damage(fd, *att_status) > 0) { desc += "+" + to_string(valor_damage(fd, *att_status)) + "(valor)"; }
            if(def_status->m_enfeebled > 0) { desc += "+" + to_string(def_status->m_enfeebled) + "(enfeebled)"; }
            if(def_card.m_flying && att_card.m_antiair > 0) { desc += "+" + to_string(att_card.m_antiair) + "(antiair)"; }
            if(def_status->m_hp == def_card.m_health && att_card.m_burst > 0) { desc += "+" + to_string(att_card.m_burst) + "(burst)"; }
            std::string reduced_desc;
            if(def_card.m_armored > 0) { reduced_desc += to_string(def_card.m_armored) + "(armored)"; }
            if(def_status->m_protected > 0) { reduced_desc += (reduced_desc.empty() ? "" : "+") + to_string(def_status->m_protected) + "(protected)"; }
            if(!reduced_desc.empty() && att_card.m_pierce > 0) { reduced_desc += "-" + to_string(att_card.m_pierce) + "(pierce)"; }
            if(!reduced_desc.empty()) { desc += "-(" + reduced_desc + ")"; }
            if(!desc.empty()) { desc += "=" + to_string(modified_damage); }
            _DEBUG_MSG("%s attacks %s for %u%s damage\n", status_description(att_status).c_str(), status_description(def_status).c_str(), damage, desc.c_str());
        }
        return(modified_damage);
    }

    template<enum CardType::CardType>
    void immobilize() {}

    template<enum CardType::CardType>
    void attack_damage()
    {
        remove_hp(fd, *def_status, att_dmg);
        killed_by_attack = def_status->m_hp == 0;
    }

    template<enum CardType::CardType>
    void siphon_poison_disease() {}

    template<enum CardType::CardType>
    void on_kill() {}

    template<enum CardType::CardType cardtype>
    void on_attacked()
    {
        if(def_status->m_card->m_poison_oa > 0)
        {
            apply_poison(att_status, def_status->m_card->m_poison_oa);
        }
        if(def_status->m_card->m_disease_oa)
        {
            att_status->m_diseased = true;
        }
        oa_berserk<cardtype>();
        for(auto& oa_skill: def_status->m_card->m_skills_on_attacked)
        {
            _DEBUG_MSG("Evaluating %s skill %s on attacked\n", status_description(def_status).c_str(), skill_description(fd, oa_skill).c_str());
            fd->skill_queue.emplace_back(def_status, oa_skill);
            resolve_skill(fd);
        }
    }

    template<enum CardType::CardType>
    void oa_berserk() {}

    template<enum CardType::CardType>
    void counter()
    {
        if(def_status->m_card->m_counter > 0)
        {
            unsigned counter_dmg(counter_damage(att_status, def_status));
            _DEBUG_MSG("%s takes %u counter damage from %s\n", status_description(att_status).c_str(), counter_dmg, status_description(def_status).c_str());
            remove_hp(fd, *att_status, counter_dmg);
        }
    }

    template<enum CardType::CardType>
    void berserk()
    {
        att_status->m_berserk += att_status->m_card->m_berserk;
    }

    template<enum CardType::CardType>
    void crush_leech() {}
};

template<>
void PerformAttack::immobilize<CardType::assault>()
{
    if(att_status->m_card->m_immobilize && def_status->m_delay <= 1 && !def_status->m_jammed && !def_status->m_frozen && fd->flip())
    {
        _DEBUG_MSG("%s immobilizes %s\n", status_description(att_status).c_str(), status_description(def_status).c_str());
        def_status->m_immobilized = true;
    }
}

template<>
void PerformAttack::attack_damage<CardType::commander>()
{
    remove_commander_hp(fd, *def_status, att_dmg);
}

template<>
void PerformAttack::siphon_poison_disease<CardType::assault>()
{
    if(att_status->m_card->m_siphon > 0)
    {
        _DEBUG_MSG("%s siphons %u health for %s\n", status_description(att_status).c_str(), std::min(att_dmg, att_status->m_card->m_siphon), status_description(&fd->tap->commander).c_str());
        add_hp(fd, &fd->tap->commander, std::min(att_dmg, att_status->m_card->m_siphon));
    }
    if(att_status->m_card->m_poison > 0)
    {
        apply_poison(def_status, att_status->m_card->m_poison);
    }
    if(att_status->m_card->m_disease)
    {
        def_status->m_diseased = true;
    }
}

template<>
void PerformAttack::on_kill<CardType::assault>()
{
    if(killed_by_attack)
    {
        for(auto& on_kill_skill: att_status->m_card->m_skills_on_kill)
        {
            _DEBUG_MSG("Evaluating %s skill %s on kill\n", status_description(att_status).c_str(), skill_description(fd, on_kill_skill).c_str());
            fd->skill_queue.emplace_back(att_status, on_kill_skill);
            resolve_skill(fd);
        }
    }
}

template<>
void PerformAttack::oa_berserk<CardType::assault>() { def_status->m_berserk += def_status->m_card->m_berserk_oa; }

template<>
void PerformAttack::crush_leech<CardType::assault>()
{
    if(att_status->m_card->m_crush > 0 && killed_by_attack)
    {
        CardStatus* def_status{select_first_enemy_wall(fd)}; // defending wall
        if (def_status != nullptr)
        {
            _DEBUG_MSG("%s crushes %s for %u damage\n", status_description(att_status).c_str(), status_description(def_status).c_str(), att_status->m_card->m_crush);
            remove_hp(fd, *def_status, att_status->m_card->m_crush);
        }
        else
        {
            _DEBUG_MSG("%s crushes %s for %u damage\n", status_description(att_status).c_str(), status_description(&fd->tip->commander).c_str(), att_status->m_card->m_crush);
            remove_commander_hp(fd, fd->tip->commander, att_status->m_card->m_crush);
        }
    }
    if(att_status->m_card->m_leech > 0 && att_status->m_hp > 0 && !att_status->m_diseased)
    {
        _DEBUG_MSG("%s leeches %u health\n", status_description(att_status).c_str(), std::min(att_dmg, att_status->m_card->m_leech));
        add_hp(fd, att_status, std::min(att_dmg, att_status->m_card->m_leech));
    }
}

// General attack phase by the currently evaluated assault, taking into accounts exotic stuff such as flurry,swipe,etc.
void attack_phase(Field* fd)
{
    CardStatus* att_status(&fd->tap->assaults[fd->current_ci]); // attacking card
    Storage<CardStatus>& def_assaults(fd->tip->assaults);
    unsigned num_attacks(att_status->m_card->m_flurry > 0 && fd->flip() ? att_status->m_card->m_flurry + 1 : 1);
    if(num_attacks > 1) { _DEBUG_MSG("%s activates flurry\n", status_description(att_status).c_str()); }
    for(unsigned attack_index(0); attack_index < num_attacks && !att_status->m_jammed && !att_status->m_frozen && att_status->m_hp > 0 && fd->tip->commander.m_hp > 0; ++attack_index)
    {
        // 3 possibilities:
        // - 1. attack against the assault in front
        // - 2. swipe attack the assault in front and adjacent assaults if any
        // - 3. attack against the commander or walls (if there is no assault or if the attacker has the fear attribute)
        // Check if attack mode is 1. or 2. (there is a living assault card in front, and no fear)
        if(alive_assault(def_assaults, fd->current_ci) && !att_status->m_card->m_fear)
        {
            // attack mode 1.
            if(!att_status->m_card->m_swipe)
            {
                PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci]}.op<CardType::assault>();
            }
            // attack mode 2.
            else
            {
                // attack the card on the left
                if(fd->current_ci > 0 && alive_assault(def_assaults, fd->current_ci - 1))
                {
                    PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci-1]}.op<CardType::assault>();
                }
                // stille alive? attack the card in front
                if(fd->tip->commander.m_hp > 0 && att_status->m_hp > 0 && alive_assault(def_assaults, fd->current_ci))
                {
                    PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci]}.op<CardType::assault>();
                }
                // still alive? attack the card on the right
                if(fd->tip->commander.m_hp > 0 && att_status->m_hp > 0 && alive_assault(def_assaults, fd->current_ci + 1))
                {
                    PerformAttack{fd, att_status, &fd->tip->assaults[fd->current_ci+1]}.op<CardType::assault>();
                }
            }
        }
        // attack mode 3.
        else
        {
            CardStatus* def_status{select_first_enemy_wall(fd)}; // defending wall
            if(def_status != nullptr)
            {
                PerformAttack{fd, att_status, def_status}.op<CardType::structure>();
            }
            else
            {
                PerformAttack{fd, att_status, &fd->tip->commander}.op<CardType::commander>();
            }
        }
    }
}

//---------------------- $65 active skills implementation ----------------------
unsigned strike_damage(CardStatus* target, unsigned v)
{
    return(safe_minus(v + target->m_enfeebled, target->m_protected));
}

template<
    bool C
    , typename T1
    , typename T2
    >
struct if_
{
    typedef T1 type;
};

template<
    typename T1
    , typename T2
    >
struct if_<false,T1,T2>
{
    typedef T2 type;
};

template<unsigned skill_id>
inline bool skill_predicate(Field* fd, CardStatus* c)
{ assert(false); return(false); }

// For the active player, delay == 0 or blitzing; for the inactive player, delay <= 1.
inline bool can_act(Field* fd, CardStatus* c) { return(c->m_hp > 0 && !c->m_jammed && !c->m_frozen && (fd->tapi == c->m_player ? c->m_delay == 0 || c->m_blitzing : c->m_delay <= 1)); }

template<>
inline bool skill_predicate<augment>(Field* fd, CardStatus* c)
{
    if(can_act(fd, c))
    {
        for(auto& s: c->m_card->m_skills)
        {
            // Any quantifiable skill except augment
            if(std::get<1>(s) > 0 && std::get<0>(s) != augment && std::get<0>(s) != augment_all && std::get<0>(s) != summon) { return(true); }
        }
    }
    return(false);
}

template<>
inline bool skill_predicate<chaos>(Field* fd, CardStatus* c)
{ return(!c->m_chaosed && can_act(fd, c)); }

template<>
inline bool skill_predicate<cleanse>(Field* fd, CardStatus* c)
{
    return(c->m_hp > 0 && (
               c->m_chaosed ||
               c->m_diseased ||
               c->m_enfeebled > 0 ||
               (c->m_frozen && c->m_delay == 0) ||
               c->m_jammed ||
               c->m_poisoned
               ));
}

template<>
inline bool skill_predicate<enfeeble>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<freeze>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0 && !c->m_jammed && !c->m_frozen); }

template<>
inline bool skill_predicate<heal>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0 && c->m_hp < c->m_card->m_health && !c->m_diseased); }

template<>
inline bool skill_predicate<infuse>(Field* fd, CardStatus* c)
{ return(c->m_faction != bloodthirsty); }

template<>
inline bool skill_predicate<jam>(Field* fd, CardStatus* c)
{ return(can_act(fd, c)); }

template<>
inline bool skill_predicate<mimic>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<protect>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<rally>(Field* fd, CardStatus* c)
{ return(!c->m_immobilized && can_act(fd, c)); }

template<>
inline bool skill_predicate<repair>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0 && c->m_hp < c->m_card->m_health); }

template<>
inline bool skill_predicate<rush>(Field* fd, CardStatus* c)
{ return(c->m_delay > 0); }

template<>
inline bool skill_predicate<siege>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<strike>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0); }

template<>
inline bool skill_predicate<supply>(Field* fd, CardStatus* c)
{ return(c->m_hp > 0 && c->m_hp < c->m_card->m_health && !c->m_diseased); }

template<>
inline bool skill_predicate<temporary_split>(Field* fd, CardStatus* c)
// It is unnecessary to check for Blitz, since temporary_split status is
// awarded before a card is played.
{ return(c->m_delay == 0 && c->m_hp > 0); }

template<>
inline bool skill_predicate<weaken>(Field* fd, CardStatus* c)
{ return(!c->m_immobilized && attack_power(c) > 0 && can_act(fd, c)); }

template<unsigned skill_id>
inline void perform_skill(Field* fd, CardStatus* c, unsigned v)
{ assert(false); }

template<>
inline void perform_skill<augment>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_augmented += v;
}

template<>
inline void perform_skill<backfire>(Field* fd, CardStatus* c, unsigned v)
{
    // TODO backfire damage counts in ANP?
    remove_commander_hp(fd, *c, v);
}

template<>
inline void perform_skill<chaos>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_chaosed = true;
}

template<>
inline void perform_skill<cleanse>(Field* fd, CardStatus* c, unsigned v)
{
    if(fd->effect == Effect::decay)
    {
        return;
    }
    c->m_chaosed = false;
    c->m_diseased = false;
    c->m_enfeebled = 0;
    c->m_frozen = false;
    c->m_immobilized = false;
    c->m_jammed = false;
    c->m_poisoned = 0;
}

template<>
inline void perform_skill<enfeeble>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_enfeebled += v;
}

template<>
inline void perform_skill<freeze>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_frozen = true;
}

template<>
inline void perform_skill<heal>(Field* fd, CardStatus* c, unsigned v)
{
    add_hp(fd, c, v);
}

template<>
inline void perform_skill<infuse>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_faction = bloodthirsty;
    c->m_infused = true;
}

template<>
inline void perform_skill<jam>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_jammed = fd->flip();
}

template<>
inline void perform_skill<protect>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_protected += v;
}

template<>
inline void perform_skill<rally>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_rallied += v;
}

template<>
inline void perform_skill<repair>(Field* fd, CardStatus* c, unsigned v)
{
    add_hp(fd, c, v);
}

template<>
inline void perform_skill<rush>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_delay = safe_minus(c->m_delay, v);
}

template<>
inline void perform_skill<shock>(Field* fd, CardStatus* c, unsigned v)
{
//    _DEBUG_MSG("shock %u. hp %u -> %u.\n", v, c->m_hp, safe_minus(c->m_hp, v));
    c->m_hp = safe_minus(c->m_hp, v);
}

template<>
inline void perform_skill<siege>(Field* fd, CardStatus* c, unsigned v)
{
//    _DEBUG_MSG(" hp %u -> %u.\n", c->m_hp, safe_minus(c->m_hp, v));
    remove_hp(fd, *c, v);
}

template<>
inline void perform_skill<strike>(Field* fd, CardStatus* c, unsigned v)
{
    remove_hp(fd, *c, strike_damage(c, v));
}

template<>
inline void perform_skill<supply>(Field* fd, CardStatus* c, unsigned v)
{
    add_hp(fd, c, v);
}

template<>
inline void perform_skill<temporary_split>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_temporary_split = true;
}

template<>
inline void perform_skill<weaken>(Field* fd, CardStatus* c, unsigned v)
{
    c->m_weakened += v;
}

template<unsigned skill_id>
inline unsigned select_fast(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    unsigned array_head{0};
    if(std::get<2>(s) == allfactions)
    {
        for(auto card: cards)
        {
            if(skill_predicate<skill_id>(fd, card))
            {
                fd->selection_array[array_head] = card;
                ++array_head;
            }
        }
    }
    else
    {
        for(auto card: cards)
        {
            if(card->m_faction == std::get<2>(s) &&
               skill_predicate<skill_id>(fd, card))
            {
                fd->selection_array[array_head] = card;
                ++array_head;
            }
        }
    }
    return(array_head);
}

template<unsigned skill_id>
inline unsigned select_rally_like(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    unsigned array_head{0};
    unsigned card_index(fd->current_phase == Field::assaults_phase ? fd->current_ci : 0);
    if(std::get<2>(s) == allfactions)
    {
        for(; card_index < cards.size(); ++card_index)
        {
            if(skill_predicate<skill_id>(fd, cards[card_index]))
            {
                fd->selection_array[array_head] = cards[card_index];
                ++array_head;
            }
        }
    }
    else
    {
        for(; card_index < cards.size(); ++card_index)
        {
            if(cards[card_index]->m_faction == std::get<2>(s) &&
               skill_predicate<skill_id>(fd, cards[card_index]))
            {
                fd->selection_array[array_head] = cards[card_index];
                ++array_head;
            }
        }
    }
    return(array_head);
}

template<>
inline unsigned select_fast<augment>(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    return(select_rally_like<augment>(fd, src_status, cards, s));
}

template<>
inline unsigned select_fast<rally>(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    return(select_rally_like<rally>(fd, src_status, cards, s));
}

template<>
inline unsigned select_fast<supply>(Field* fd, CardStatus* src_status, const std::vector<CardStatus*>& cards, const SkillSpec& s)
{
    // mimiced supply by a structure, etc ?
    if(!(src_status && src_status->m_card->m_type == CardType::assault)) { return(0); }
    unsigned array_head{0};
    const unsigned min_index(src_status->m_index - (src_status->m_index == 0 ? 0 : 1));
    const unsigned max_index(src_status->m_index + (src_status->m_index == cards.size() - 1 ? 0 : 1));
    for(unsigned card_index(min_index); card_index <= max_index; ++card_index)
    {
        if(skill_predicate<supply>(fd, cards[card_index]))
        {
            fd->selection_array[array_head] = cards[card_index];
            ++array_head;
        }
    }
    return(array_head);
}

inline unsigned select_infuse(Field* fd, const SkillSpec& s)
{
    unsigned array_head{0};
    // Select candidates among attacker's assaults
    for(auto card_status: fd->tap->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(fd, card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    // Select candidates among defender's assaults
    for(auto card_status: fd->tip->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(fd, card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    return(array_head);
}

inline std::vector<CardStatus*>& skill_targets_hostile_assault(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? (src_status->m_chaosed ? src_status->m_player : opponent(src_status->m_player)) : fd->tipi]->assaults.m_indirect);
}

inline std::vector<CardStatus*>& skill_targets_allied_assault(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? src_status->m_player : fd->tapi]->assaults.m_indirect);
}

inline std::vector<CardStatus*>& skill_targets_hostile_structure(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? (src_status->m_chaosed ? src_status->m_player : opponent(src_status->m_player)) : fd->tipi]->structures.m_indirect);
}

inline std::vector<CardStatus*>& skill_targets_allied_structure(Field* fd, CardStatus* src_status)
{
    return(fd->players[src_status ? src_status->m_player : fd->tapi]->structures.m_indirect);
}

template<unsigned skill>
std::vector<CardStatus*>& skill_targets(Field* fd, CardStatus* src_status)
{
    std::cout << "skill_targets: Error: no specialization for " << skill_names[skill] << "\n";
    throw;
}

template<> inline std::vector<CardStatus*>& skill_targets<augment>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<chaos>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<cleanse>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<enfeeble>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<freeze>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<heal>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<jam>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<mimic>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<protect>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<rally>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<repair>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_structure(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<rush>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<strike>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<supply>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<temporary_split>(Field* fd, CardStatus* src_status)
{ return(skill_targets_allied_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<weaken>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_assault(fd, src_status)); }

template<> std::vector<CardStatus*>& skill_targets<siege>(Field* fd, CardStatus* src_status)
{ return(skill_targets_hostile_structure(fd, src_status)); }

template<typename T>
void maybeTriggerRegen(Field* fd)
{
}

template<>
void maybeTriggerRegen<true_>(Field* fd)
{
    fd->skill_queue.emplace_front(nullptr, std::make_tuple(trigger_regen, 0, allfactions));
}

template<unsigned skill_id>
CardStatus* get_target_hostile_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    if(array_head > 0)
    {
        unsigned rand_index(fd->rand(0, array_head - 1));
        CardStatus* c(fd->selection_array[rand_index]);
        // intercept
        if(src_status && !src_status->m_chaosed)
        {
            CardStatus* intercept_card(nullptr);
            if(rand_index > 0)
            {
                CardStatus* left_status(fd->selection_array[rand_index-1]);
                if(left_status->m_card->m_intercept && left_status->m_index == c->m_index-1)
                {
                    intercept_card = left_status;
                }
            }
            if(rand_index+1 < array_head && !intercept_card)
            {
                CardStatus* right_status(fd->selection_array[rand_index+1]);
                if(right_status->m_card->m_intercept && right_status->m_index == c->m_index+1)
                {
                    intercept_card = right_status;
                }
            }
            if(intercept_card) { c = intercept_card; }
        }
        return(c);
    }
    return(nullptr);
}

bool negate_by_evade(Field* fd, CardStatus* c)
{
    if(c->m_card->m_evade && fd->flip())
    {
        _DEBUG_MSG("%s evades\n", status_description(c).c_str());
        return(true);
    }
    else
    { return(false); }
}

template<unsigned skill_id>
void perform_targetted_hostile_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    // null status = action card
    CardStatus* c(get_target_hostile_fast<skill_id>(fd, src_status, s));
    if(c)
    {
        _DEBUG_MSG("%s %s (%u) on %s\n", status_description(src_status).c_str(), skill_names[skill_id].c_str(), std::get<1>(s), status_description(c).c_str());
        if((src_status && src_status->m_chaosed) || !negate_by_evade(fd, c))
        {
            perform_skill<skill_id>(fd, c, std::get<1>(s));
            // payback
            if(c->m_card->m_payback &&
               src_status &&
               src_status->m_card->m_type == CardType::assault &&
               !src_status->m_chaosed &&
               src_status->m_hp > 0 &&
               fd->flip())
            {
                if(skill_predicate<skill_id>(fd, src_status))
                {
                    _DEBUG_MSG("Payback (%s %u) on %s\n", skill_names[skill_id].c_str(), std::get<1>(s), status_description(src_status).c_str());
                    // payback skill
                    if(!negate_by_evade(fd, c)) { perform_skill<skill_id>(fd, src_status, std::get<1>(s)); }
                }
            }
        }
    }
    maybeTriggerRegen<typename skillTriggersRegen<skill_id>::T>(fd);
    prepend_on_death(fd);
}

template<unsigned skill_id>
void perform_targetted_allied_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    if(array_head > 0)
    {
        CardStatus* c(fd->selection_array[fd->rand(0, array_head - 1)]);
        _DEBUG_MSG("%s %s (%u) on %s\n", status_description(src_status).c_str(), skill_names[skill_id].c_str(), std::get<1>(s), status_description(c).c_str());
        perform_skill<skill_id>(fd, c, std::get<1>(s));
        if(c->m_card->m_tribute &&
           src_status &&
           src_status->m_card->m_type == CardType::assault &&
           src_status != c &&
           src_status->m_hp > 0 &&
           fd->flip())
        {
            if(skill_predicate<skill_id>(fd, src_status))
            {
                _DEBUG_MSG("Tribute (%s %u) on %s\n", skill_names[skill_id].c_str(), std::get<1>(s), status_description(src_status).c_str());
                perform_skill<skill_id>(fd, src_status, std::get<1>(s));
            }
        }

        // check emulate
        Hand* opp = fd->players[opponent(c->m_player)];
        if(opp->assaults.size() > c->m_index)
        {
            CardStatus& emulator = opp->assaults[c->m_index];
            if(emulator.m_card->m_emulate && skill_predicate<skill_id>(fd, &emulator))
            {
                _DEBUG_MSG("Emulate (%s %u) on %s\n", skill_names[skill_id].c_str(), std::get<1>(s), status_description(&emulator).c_str());
                perform_skill<skill_id>(fd, &emulator, std::get<1>(s));
            }
        }
    }
}

template<unsigned skill_id>
void perform_global_hostile_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    unsigned payback_count(0);
    for(unsigned s_index(0); s_index < array_head; ++s_index)
    {
        CardStatus* c(fd->selection_array[s_index]);
        _DEBUG_MSG("%s %s (%u) on %s\n", status_description(src_status).c_str(), skill_names[skill_id].c_str(), std::get<1>(s), status_description(c).c_str());
        if((src_status && src_status->m_chaosed) || !negate_by_evade(fd, c))
        {
            perform_skill<skill_id>(fd, c, std::get<1>(s));
            // payback
            if(c->m_card->m_payback &&
               src_status &&
               src_status->m_card->m_type == CardType::assault &&
               !src_status->m_chaosed &&
               src_status->m_hp > 0 &&
               fd->flip())
            {
                ++payback_count;
            }
        }
    }
    for(unsigned i(0); i < payback_count && skill_predicate<skill_id>(fd, src_status); ++i)
    {
        _DEBUG_MSG("Payback (%s %u) on %s\n", skill_names[skill_id].c_str(), std::get<1>(s), status_description(src_status).c_str());
        if(!negate_by_evade(fd, src_status)) { perform_skill<skill_id>(fd, src_status, std::get<1>(s)); }
    }
    maybeTriggerRegen<typename skillTriggersRegen<skill_id>::T>(fd);
    prepend_on_death(fd);
}

template<unsigned skill_id>
void perform_global_allied_fast(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    std::vector<CardStatus*>& cards(skill_targets<skill_id>(fd, src_status));
    unsigned array_head{select_fast<skill_id>(fd, src_status, cards, s)};
    for(unsigned s_index(0); s_index < array_head; ++s_index)
    {
        CardStatus* c(fd->selection_array[s_index]);
        _DEBUG_MSG("%s %s (%u) on %s\n", status_description(src_status).c_str(), skill_names[skill_id].c_str(), std::get<1>(s), status_description(c).c_str());
        perform_skill<skill_id>(fd, c, std::get<1>(s));
        if(c->m_card->m_tribute &&
           src_status &&
           src_status->m_card->m_type == CardType::assault &&
           src_status != c &&
           src_status->m_hp > 0 &&
           fd->flip())
        {
            if(skill_predicate<skill_id>(fd, src_status))
            {
                _DEBUG_MSG("Tribute (%s %u) on %s\n", skill_names[skill_id].c_str(), std::get<1>(s), status_description(src_status).c_str());
                perform_skill<skill_id>(fd, src_status, std::get<1>(s));
            }
        }

        // check emulate
        Hand* opp = fd->players[opponent(c->m_player)];
        if(opp->assaults.size() > c->m_index)
        {
            CardStatus& emulator = opp->assaults[c->m_index];
            if(emulator.m_card->m_emulate && skill_predicate<skill_id>(fd, &emulator))
            {
                _DEBUG_MSG("Emulate (%s %u) on %s\n", skill_names[skill_id].c_str(), std::get<1>(s), status_description(&emulator).c_str());
                perform_skill<skill_id>(fd, &emulator, std::get<1>(s));
            }
        }
    }
}

void perform_backfire(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    CardStatus* c(&fd->players[src_status->m_player]->commander);
    _DEBUG_MSG("%s %s (%u) on %s\n", status_description(src_status).c_str(), skill_names[std::get<0>(s)].c_str(), std::get<1>(s), status_description(c).c_str());
    perform_skill<backfire>(fd, c, std::get<1>(s));
}

void perform_infuse(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    unsigned array_head{0};
    // Select candidates among attacker's assaults
    for(auto card_status: fd->players[src_status->m_player]->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(fd, card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    // Select candidates among defender's assaults
    for(auto card_status: fd->players[opponent(src_status->m_player)]->assaults.m_indirect)
    {
        if(skill_predicate<infuse>(fd, card_status))
        {
            fd->selection_array[array_head] = card_status;
            ++array_head;
        }
    }
    if(array_head > 0)
    {
        CardStatus* c(fd->selection_array[fd->rand(0, array_head - 1)]);
        // check evade for enemy assaults only
        if(c->m_player == src_status->m_player)
        {
            _DEBUG_MSG("%s %s on %s\n", status_description(src_status).c_str(), skill_names[infuse].c_str(), status_description(c).c_str());
            if(!negate_by_evade(fd, c)) { perform_skill<infuse>(fd, c, std::get<1>(s)); }
        }
    }
}

// a summoned card's on play skills seem to be evaluated before any other skills on the skill queue.
inline void prepend_skills(Field* fd, CardStatus* status)
{
    for(auto& skill: boost::adaptors::reverse(status->m_card->m_skills_on_play))
    {
        fd->skill_queue.emplace_front(status, skill);
    }
}
void summon_card(Field* fd, unsigned player, const Card* summoned)
{
    assert(summoned->m_type == CardType::assault || summoned->m_type == CardType::structure);
    Hand* hand{fd->players[player]};
    if(hand->assaults.size() + hand->structures.size() < 100)
    {
        Storage<CardStatus>* storage{summoned->m_type == CardType::assault ? &hand->assaults : &hand->structures};
        CardStatus& card_status(storage->add_back());
        card_status.set(summoned);
        card_status.m_index = storage->size() - 1;
        card_status.m_player = player;
        _DEBUG_MSG("Summon %s %u [%s]\n", cardtype_names[summoned->m_type].c_str(), card_status.m_index, card_description(fd, summoned).c_str());
        prepend_skills(fd, &card_status);
        if(card_status.m_card->m_blitz &&
           fd->players[opponent(player)]->assaults.size() > card_status.m_index &&
           fd->players[opponent(player)]->assaults[card_status.m_index].m_hp > 0 &&
           fd->players[opponent(player)]->assaults[card_status.m_index].m_delay == 0)
        {
            card_status.m_blitzing = true;
        }

        if(fd->effect == Effect::toxic)
        {
            card_status.m_poisoned = 1;
        }
        else if(fd->effect == Effect::decay)
        {
            card_status.m_poisoned = 1;
            card_status.m_diseased = true;
        }
    }
}
void perform_summon(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    summon_card(fd, src_status ? src_status->m_player : fd->tapi, fd->cards.by_id(std::get<1>(s)));
}

void perform_trigger_regen(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    check_regeneration(fd);
}

void perform_shock(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    _DEBUG_MSG("%s shocks %s for %u damage\n", status_description(src_status).c_str(), status_description(&fd->tip->commander).c_str(), std::get<1>(s));
    perform_skill<shock>(fd, &fd->tip->commander, std::get<1>(s));
}

// Special rules for mimic :
// cannot mimic mimic,
// structures cannot mimic supply,
// and is not affected by payback.
void perform_mimic(Field* fd, CardStatus* src_status, const SkillSpec& s)
{
    // mimic cannot be triggered by anything. So it should be the only skill in the unresolved skill table.
    // so we can probably clear it safely. This is necessary, because mimic calls resolve_skill as well (infinite loop).
    fd->skill_queue.clear();
    CardStatus* c(nullptr);
    if(fd->effect == Effect::copycat)
    {
        std::vector<CardStatus*>& cards(skill_targets_allied_assault(fd, src_status));
        unsigned array_head{select_fast<mimic>(fd, src_status, cards, s)};
        if(array_head > 0)
        {
            c = fd->selection_array[fd->rand(0, array_head - 1)];
            _DEBUG_MSG("%s on %s\n", skill_names[std::get<0>(s)].c_str(), status_description(c).c_str());
        }
    }
    else
    {
        c = get_target_hostile_fast<mimic>(fd, src_status, s);
        // evade check for mimic
        // individual skills are subject to evade checks too,
        // but resolve_skill will handle those.
        if(c && (!src_status || !src_status->m_chaosed))
        {
            _DEBUG_MSG("%s on %s\n", skill_names[std::get<0>(s)].c_str(), status_description(c).c_str());
            if(negate_by_evade(fd, c)) { return; }
        }
    }

    if(c)
    {
        for(auto skill: c->m_card->m_skills)
        {
            if(src_status && src_status->m_card->m_type == CardType::assault && src_status->m_hp == 0)
            { break; }
            if(std::get<0>(skill) != mimic &&
               (std::get<0>(skill) != supply || (src_status && src_status->m_card->m_type == CardType::assault)))
            {
                SkillSpec mimic_s(std::get<0>(skill), std::get<1>(skill), allfactions);
                _DEBUG_MSG("Evaluating mimiced %s skill %s\n", status_description(c).c_str(), skill_description(fd, skill).c_str());
                fd->skill_queue.emplace_back(src_status, src_status && src_status->m_augmented > 0 ? augmented_skill(src_status, mimic_s) : mimic_s);
                resolve_skill(fd);
                check_regeneration(fd);
            }
        }
    }
}
//------------------------------------------------------------------------------
void fill_skill_table()
{
    skill_table[augment] = perform_targetted_allied_fast<augment>;
    skill_table[augment_all] = perform_global_allied_fast<augment>;
    skill_table[backfire] = perform_backfire;
    skill_table[chaos] = perform_targetted_hostile_fast<chaos>;
    skill_table[chaos_all] = perform_global_hostile_fast<chaos>;
    skill_table[cleanse] = perform_targetted_allied_fast<cleanse>;
    skill_table[cleanse_all] = perform_global_allied_fast<cleanse>;
    skill_table[enfeeble] = perform_targetted_hostile_fast<enfeeble>;
    skill_table[enfeeble_all] = perform_global_hostile_fast<enfeeble>;
    skill_table[freeze] = perform_targetted_hostile_fast<freeze>;
    skill_table[freeze_all] = perform_global_hostile_fast<freeze>;
    skill_table[heal] = perform_targetted_allied_fast<heal>;
    skill_table[heal_all] = perform_global_allied_fast<heal>;
    skill_table[infuse] = perform_infuse;
    skill_table[jam] = perform_targetted_hostile_fast<jam>;
    skill_table[jam_all] = perform_global_hostile_fast<jam>;
    skill_table[mimic] = perform_mimic;
    skill_table[protect] = perform_targetted_allied_fast<protect>;
    skill_table[protect_all] = perform_global_allied_fast<protect>;
    skill_table[rally] = perform_targetted_allied_fast<rally>;
    skill_table[rally_all] = perform_global_allied_fast<rally>;
    skill_table[repair] = perform_targetted_allied_fast<repair>;
    skill_table[repair_all] = perform_global_allied_fast<repair>;
    skill_table[rush] = perform_targetted_allied_fast<rush>;
    skill_table[shock] = perform_shock;
    skill_table[siege] = perform_targetted_hostile_fast<siege>;
    skill_table[siege_all] = perform_global_hostile_fast<siege>;
    skill_table[supply] = perform_global_allied_fast<supply>;
    skill_table[strike] = perform_targetted_hostile_fast<strike>;
    skill_table[strike_all] = perform_global_hostile_fast<strike>;
    skill_table[summon] = perform_summon;
    skill_table[temporary_split] = perform_targetted_allied_fast<temporary_split>;
    skill_table[trigger_regen] = perform_trigger_regen;
    skill_table[weaken] = perform_targetted_hostile_fast<weaken>;
    skill_table[weaken_all] = perform_global_hostile_fast<weaken>;
}
//------------------------------------------------------------------------------
// Utility functions for modify_cards.

// Adds the skill "<new_skill> <magnitude>" to all assaults,
// except those who have any instance of either <new_skill> or <conflict>.
inline void maybe_add_to_assaults(Cards& cards, ActiveSkill new_skill, unsigned magnitude, ActiveSkill conflict)
{
    for(Card* card: cards.cards)
    {
        if(card->m_type != CardType::assault)
        {
            continue;
        }

        bool conflict(false);
        for(auto& skill: card->m_skills)
        {
            if(std::get<0>(skill) == new_skill ||
               std::get<0>(skill) == conflict)
            {
                conflict = true;
            }
        }

        if(!conflict)
        {
            card->add_skill(new_skill, magnitude, allfactions);
        }
    }
}
// Adds the skill "<skill> <magnitude>" to all commanders.
inline void add_to_commanders(Cards& cards, ActiveSkill skill, unsigned magnitude)
{
    for(Card* card: cards.cards)
    {
        if(card->m_type != CardType::commander)
        {
            continue;
        }
        card->add_skill(skill, magnitude, allfactions);
    }
}

// Adds the skill "<new> <magnitude>" to all commanders.
// If the commander has an instance of either <old> or <new> in its skill list,
// the new skill replaces it.
// Otherwise, the new skill is added on to the end.
inline void replace_on_commanders(Cards& cards, ActiveSkill old_skill, ActiveSkill new_skill, unsigned magnitude)
{
    for(Card* card: cards.cards)
    {
        if(card->m_type != CardType::commander)
        {
            continue;
        }

        bool replaced(false);
        for(auto& skill: card->m_skills)
        {
            if(std::get<0>(skill) == old_skill ||
               std::get<0>(skill) == new_skill)
            {
                skill = std::make_tuple(new_skill, magnitude, allfactions);
                replaced = true;
            }
        }

        if(!replaced)
        {
            card->add_skill(new_skill, magnitude, allfactions);
        }
    }
}
//------------------------------------------------------------------------------
void modify_cards(Cards& cards, enum Effect effect)
{
    switch (effect)
    {
        case Effect::none:
            break;
        case Effect::time_surge:
            add_to_commanders(cards, rush, 1);
            break;
        case Effect::copycat:
            // Do nothing; this is implemented in perform_mimic
            break;
        case Effect::quicksilver:
            for(Card* card: cards.cards)
            {
                if(card->m_type == CardType::assault)
                {
                    card->m_evade = true;
                }
            }
            break;
        case Effect::decay:
            // Do nothing; this is implemented in PlayCard::fieldEffects,
            // summon_card, and perform_skill<cleanse>
            break;
        case Effect::high_skies:
            // Do nothing; this is implemented in PerformAttack
            break;
        case Effect::impenetrable:
            // Also implemented in PerformAttack
            for(Card* card: cards.cards)
            {
                if(card->m_type == CardType::structure)
                {
                    card->m_refresh = false;
                }
            }
            break;
        case Effect::invigorate:
            // Do nothing; this is implemented in add_hp
            break;
        case Effect::clone_project:
        case Effect::clone_experiment:
            // Do nothing; these are implemented in the temporary_split skill
            break;
        case Effect::friendly_fire:
            replace_on_commanders(cards, chaos, chaos_all, 0);
            maybe_add_to_assaults(cards, strike, 1, strike_all);
            break;
        case Effect::genesis:
            // Do nothing; this is implemented in play
            break;
        case Effect::decrepit:
            replace_on_commanders(cards, enfeeble, enfeeble_all, 1);
            break;
        case Effect::forcefield:
            replace_on_commanders(cards, protect, protect_all, 1);
            break;
        case Effect::chilling_touch:
            add_to_commanders(cards, freeze, 0);
            break;
        case Effect::toxic:
            // Do nothing; this is implemented in PlayCard::fieldEffects
            // and summon_card
            break;
        default:
            // TODO: throw something more useful
            throw effect;
            break;
    }
}
