/*
 * Trinity Core and update by WoWSource Forums
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * World Boss: Nalak <The Storm Lord>.
*/

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "CreatureTextMgr.h"
#include "SpellScript.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "Player.h"

enum Texts
{
    // Nalak.
    SAY_INTRO         = 0, // I am born of thunder!
    SAY_AGGRO         = 1, // Can you feel a chill wind blow? The storm is coming...
    SAY_DEATH         = 2, // I am but... The darkness... Before the storm...
    SAY_SLAY          = 3, // The sky weeps for your demise!
    SAY_ARC_NOVA      = 4, // The clouds arc with vengeance!
    SAY_STORM_CLOUD   = 5  // The air crackles with anger!
};

enum Spells
{
    // Nalak.
    SPELL_ARC_NOVA          = 136338, // 39s, every 42 s.
    SPELL_LIGHTNING_TET_A   = 136339, // 28s, every 35 s.
    SPELL_LIGHTNING_TET_S   = 136350, // Scripting spell. 0 - SE SPELL_LIGHTNING_TET_D +30yd; 1 - Dummy close dmg. SPELL_LITHGNING_TET_N.
    SPELL_LIGHTNING_TET_D   = 136349, // Damage for over 30y.
    SPELL_LITHGNING_TET_N   = 136353, // Normal damage.
    SPELL_STATIC_SHIELD     = 136341, // As aggro, whole fight, triggers dmg each sec.
    SPELL_STORMCLOUD        = 136340  // 15s, every 24 s.
};

enum Events
{
    EVENT_ARC_NOVA         = 1,
    EVENT_LIGHTNING_TET    = 2,
    EVENT_STORMCLOUD       = 3
};

class boss_nalak : public CreatureScript
{
    public:
        boss_nalak() : CreatureScript("boss_nalak") { }

        struct boss_nalakAI : public ScriptedAI
        {
            boss_nalakAI(Creature* creature) : ScriptedAI(creature)
            {
                introDone = false;
            }

            EventMap events;
            bool introDone;

            void InitializeAI()
            {
                if (!me->isDead())
                    Reset();
            }

            void Reset()
            {
                events.Reset();
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!introDone && me->IsWithinDistInMap(who, 40) && who->GetTypeId() == TYPEID_PLAYER)
                {
                    Talk(SAY_INTRO);
                    introDone = true;
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);
                DoCast(me, SPELL_STATIC_SHIELD);

                events.ScheduleEvent(EVENT_ARC_NOVA, urand(37000, 41000)); // 37-41s
                events.ScheduleEvent(EVENT_LIGHTNING_TET, urand(28000, 35000)); // 28-35s
                events.ScheduleEvent(EVENT_STORMCLOUD, urand(13000, 17000)); // 13-17s
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void EnterEvadeMode()
            {
                me->RemoveAllAuras();
                Reset();
                me->DeleteThreatList();
                me->CombatStop(true);
                me->GetMotionMaster()->MoveTargetedHome();
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_DEATH);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARC_NOVA:
                            Talk(SAY_ARC_NOVA);
                            DoCast(me, SPELL_ARC_NOVA);
                            events.ScheduleEvent(EVENT_ARC_NOVA, urand(41000, 43000));
                            break;

                        case EVENT_LIGHTNING_TET:
                            DoCast(me, SPELL_LIGHTNING_TET_A);
                            events.ScheduleEvent(EVENT_LIGHTNING_TET, urand(37000, 39000));
                            break;

                        case EVENT_STORMCLOUD:
                            Talk(SAY_STORM_CLOUD);
                            DoCast(me, SPELL_STORMCLOUD);
                            events.ScheduleEvent(EVENT_STORMCLOUD, urand(33000, 37000));
                            break;

                        default: break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_nalakAI(creature);
        }
};

// Lightning Tether / Static Shield / StormCloud player check.
class PlayerCheck : public zonaxtrema::unary_function<Unit*, bool>
{
    public:
        explicit PlayerCheck(Unit* _caster) : caster(_caster) { }
        bool operator()(WorldObject* object)
        {
            return object->GetTypeId() != TYPEID_PLAYER;
        }

    private:
        Unit* caster;
};

// Static Shield damage 136343.
class spell_static_shield_damage : public SpellScriptLoader
{
    public:
        spell_static_shield_damage() : SpellScriptLoader("spell_static_shield_damage") { }

        class spell_static_shield_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_static_shield_damage_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                // Set targets.
                targets.remove_if(PlayerCheck(GetCaster()));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_static_shield_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_static_shield_damage_SpellScript();
        }
};

// Lightning Tether trigger aura 136339.
class spell_lightning_tether_dmg_trigger : public SpellScriptLoader
{
    public:
        spell_lightning_tether_dmg_trigger() : SpellScriptLoader("spell_lightning_tether_dmg_trigger") { }

        class spell_lightning_tether_dmg_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_lightning_tether_dmg_trigger_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                // Set targets.
                targets.remove_if(PlayerCheck(GetCaster()));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lightning_tether_dmg_trigger_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_lightning_tether_dmg_trigger_SpellScript();
        }
};

// Lightning Tether aura triggered spell 136350.
class spell_lightning_tether_trigger : public SpellScriptLoader
{
    public:
        spell_lightning_tether_trigger() : SpellScriptLoader("spell_lightning_tether_trigger") { }

        class spell_lightning_tether_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_lightning_tether_trigger_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(PlayerCheck(GetCaster()));
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[EFFECT_0].BasePoints, true); // SPELL_LIGHTNING_TET_D
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[EFFECT_1].BasePoints, true); // SPELL_LIGHTNING_TET_N
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_lightning_tether_trigger_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnEffectHitTarget += SpellEffectFn(spell_lightning_tether_trigger_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lightning_tether_trigger_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lightning_tether_trigger_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_NEARBY_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_lightning_tether_trigger_SpellScript();
        }
};

// Lightning Tether dmg distance check.
class DistanceCheck : public zonaxtrema::unary_function<Unit*, bool>
{
    public:
        DistanceCheck(Unit* caster, float dist) : _caster(caster), _dist(dist) { }

        bool operator()(WorldObject* object)
        {
            return _caster->GetExactDist2d(object) < _dist;
        }

    private:
        WorldObject* _caster;
        float _dist;
};

// Lightning Tether dmg distance 136349.
class spell_lightning_tether_dmg_dist : public SpellScriptLoader
{
    public:
        spell_lightning_tether_dmg_dist() : SpellScriptLoader("spell_lightning_tether_dmg_dist") { }

        class spell_lightning_tether_dmg_dist_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_lightning_tether_dmg_dist_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                // Set targets.
                targets.remove_if(PlayerCheck(GetCaster()));
                targets.remove_if(DistanceCheck(GetCaster(), 30.0f));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lightning_tether_dmg_dist_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_TARGET_ANY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_lightning_tether_dmg_dist_SpellScript();
        }
};

// Lightning Tether dmg close 136353.
class spell_lightning_tether_dmg_close : public SpellScriptLoader
{
    public:
        spell_lightning_tether_dmg_close() : SpellScriptLoader("spell_lightning_tether_dmg_close") { }

        class spell_lightning_tether_dmg_close_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_lightning_tether_dmg_close_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                // Set targets.
                targets.remove_if(PlayerCheck(GetCaster()));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lightning_tether_dmg_close_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_TARGET_ANY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_lightning_tether_dmg_close_SpellScript();
        }
};

// StormCloud trigger aura 136340.
class spell_stormcloud_dmg_trigger : public SpellScriptLoader
{
    public:
        spell_stormcloud_dmg_trigger() : SpellScriptLoader("spell_stormcloud_dmg_trigger") { }

        class spell_stormcloud_dmg_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_stormcloud_dmg_trigger_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                // Set targets.
                targets.remove_if(PlayerCheck(GetCaster()));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_stormcloud_dmg_trigger_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_stormcloud_dmg_trigger_SpellScript();
        }
};

// StormCloud damage 136345.
class spell_stormcloud_dmg : public SpellScriptLoader
{
    public:
        spell_stormcloud_dmg() : SpellScriptLoader("spell_stormcloud_dmg") { }

        class spell_stormcloud_dmg_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_stormcloud_dmg_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                // Set targets.
                targets.remove_if(PlayerCheck(GetCaster()));
            }

            void Register() 
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_stormcloud_dmg_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const 
        {
            return new spell_stormcloud_dmg_SpellScript();
        }
};

void AddSC_boss_nalak()
{
    new boss_nalak();
    new spell_static_shield_damage();
    new spell_lightning_tether_dmg_trigger();
    new spell_lightning_tether_trigger();
    new spell_lightning_tether_dmg_dist();
    new spell_lightning_tether_dmg_close();
    new spell_stormcloud_dmg_trigger();
    new spell_stormcloud_dmg();
}
