#ifndef CARD_H_INCLUDED
#define CARD_H_INCLUDED

#include <string>
#include <vector>
#include "tyrant.h"

class Card
{
public:
    Card() :
        m_antiair(0),
        m_armored(0),
        m_attack(0),
        m_base_id(0),
        m_berserk(0),
        m_berserk_oa(0),
        m_blitz(false),
        m_burst(0),
        m_counter(0),
        m_crush(0),
        m_delay(0),
        m_disease(false),
        m_disease_oa(false),
        m_emulate(false),
        m_evade(false),
        m_faction(imperial),
        m_fear(false),
        m_flurry(0),
        m_flying(false),
        m_fusion(false),
        m_health(0),
        m_hidden(0),
        m_id(0),
        m_immobilize(false),
        m_intercept(false),
        m_leech(0),
        m_legion(0),
        m_name(""),
        m_payback(false),
        m_pierce(0),
        m_phase(0),
        m_poison(0),
        m_poison_oa(0),
        m_proto_id(0),
        m_rarity(1),
        m_refresh(false),
        m_regenerate(0),
        m_replace(0),
        m_set(0),
        m_siphon(0),
        m_split(false),
        m_stun(false),
        m_sunder(false),
        m_sunder_oa(false),
        m_swipe(false),
        m_tribute(false),
        m_unique(false),
        m_valor(0),
        m_wall(false),
        m_skills(),
        m_type(CardType::assault)
    {
    }

    void add_skill(Skill v1, unsigned v2, Faction v3, bool v4)
    { m_skills.push_back(std::make_tuple(v1, v2, v3, v4, SkillMod::on_activate)); }
    void add_played_skill(Skill v1, unsigned v2, Faction v3, bool v4)
    { m_skills_on_play.push_back(std::make_tuple(v1, v2, v3, v4, SkillMod::on_play)); }
    void add_died_skill(Skill v1, unsigned v2, Faction v3, bool v4)
    { m_skills_on_death.push_back(std::make_tuple(v1, v2, v3, v4, SkillMod::on_death)); }
    void add_attacked_skill(Skill v1, unsigned v2, Faction v3, bool v4)
    { m_skills_on_attacked.push_back(std::make_tuple(v1, v2, v3, v4, SkillMod::on_attacked)); }
    void add_kill_skill(Skill v1, unsigned v2, Faction v3, bool v4)
    { m_skills_on_kill.push_back(std::make_tuple(v1, v2, v3, v4, SkillMod::on_kill)); }

    unsigned m_antiair;
    unsigned m_armored;
    unsigned m_attack;
    unsigned m_base_id;  // The id of the original card if a card is unique and alt/upgraded. The own id of the card otherwise.
    unsigned m_berserk;
    unsigned m_berserk_oa;
    bool m_blitz;
    unsigned m_burst;
    unsigned m_counter;
    unsigned m_crush;
    unsigned m_delay;
    bool m_disease;
    bool m_disease_oa;
    bool m_emulate;
    bool m_evade;
    Faction m_faction;
    bool m_fear;
    unsigned m_flurry;
    bool m_flying;
    bool m_fusion;
    unsigned m_health;
    unsigned m_hidden;
    unsigned m_id;
    bool m_immobilize;
    bool m_intercept;
    unsigned m_leech;
    unsigned m_legion;
    std::string m_name;
    bool m_payback;
    unsigned m_pierce;
    unsigned m_phase;
    unsigned m_poison;
    unsigned m_poison_oa;
    unsigned m_proto_id;  // The id of the prototype card (before upgraded) for an upgraded card. 0 otherwise.
    unsigned m_rarity;
    bool m_refresh;
    unsigned m_regenerate;
    unsigned m_replace;
    unsigned m_reserve;
    unsigned m_set;
    unsigned m_siphon;
    bool m_split;
    bool m_stun;
    bool m_sunder;
    bool m_sunder_oa;
    bool m_swipe;
    bool m_tribute;
    bool m_unique;
    unsigned m_upgrade_consumables;
    unsigned m_upgrade_gold_cost;
    unsigned m_upgraded_id;  // The id of the upgraded card for an upgradable card. 0 otherwise.
    unsigned m_valor;
    bool m_wall;
    std::vector<SkillSpec> m_skills;
    std::vector<SkillSpec> m_skills_on_play;
    std::vector<SkillSpec> m_skills_on_death;
    std::vector<SkillSpec> m_skills_on_attacked;
    std::vector<SkillSpec> m_skills_on_kill;
    CardType::CardType m_type;
};

#endif
