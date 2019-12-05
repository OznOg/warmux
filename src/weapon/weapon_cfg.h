/******************************************************************************
 *  Warmux is a convivial mass murder game.
 *  Copyright (C) 2001-2011 Warmux Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Base classes for weapons configuration.
 *****************************************************************************/

#ifndef WEAPON_CONFIGURATION_H
#define WEAPON_CONFIGURATION_H
//-----------------------------------------------------------------------------
#include <list>
#include <WARMUX_base.h>
#include "tool/xml_document.h"
#include "tool/config_element.h"

//-----------------------------------------------------------------------------
class EmptyWeaponConfig
{
public:
    EmptyWeaponConfig() = default;
    EmptyWeaponConfig(const EmptyWeaponConfig&) = delete;
    const EmptyWeaponConfig operator=(const EmptyWeaponConfig&) = delete;
    virtual ~EmptyWeaponConfig() = default;
};

//-----------------------------------------------------------------------------

struct WeaponConfig : public EmptyWeaponConfig
{
  uint damage = 10;
};

//-----------------------------------------------------------------------------

struct ExplosiveWeaponConfig : public WeaponConfig
{
  uint   timeout = 0;
  bool   allow_change_timeout = false;
  Double blast_range = 0;
  Double blast_force = 0;
  Double explosion_range = 0;
  Double particle_range = 0;
  Double speed_on_hit = 0;
};

struct SuperTuxWeaponConfig : public ExplosiveWeaponConfig
{
  uint speed = 600;
};

struct AirAttackConfig : public ExplosiveWeaponConfig
{
  Double speed = 3;
  uint nbr_obus = 3;
};

struct AutomaticBazookaConfig : public ExplosiveWeaponConfig
{
  Double uncontrolled_turn_speed = PI*8;
  Double max_controlled_turn_speed = PI*4;
  Double fuel_time = 10;
  Double rocket_force = 2500;
};

struct ClusterBombConfig : public ExplosiveWeaponConfig
{
  uint nb_fragments = 5;
};

struct CluzookaConfig : public ExplosiveWeaponConfig
{
  uint m_fragments = 5;
  uint m_angle_dispersion = 45;
};

struct FootBombConfig : public ExplosiveWeaponConfig
{
  uint nb_fragments = 2;
  uint nb_recursions = 2;
  Double nb_angle_dispersion = 0;
  Double nb_min_speed = 0;
  Double nb_max_speed = 0;
};

struct MineConfig : public ExplosiveWeaponConfig
{
  uint escape_time = 2;
  Double detection_range = 1;
  Double speed_detection = 2;
};

struct SyringeConfig : public WeaponConfig
{
  Double range = 45;
  uint turns = 10;
};

struct BaseballConfig : public WeaponConfig
{
  uint range = 70;
  uint strength = 2500;
};

struct BlowtorchConfig : public WeaponConfig
{
  uint range = 20;
};

struct ParachuteConfig : public WeaponConfig
{
  Double wind_factor = 10;
  Double air_resist_factor = 140;
  Double force_side_displacement = 2000;

};

struct SlapConfig : public WeaponConfig
{
  Double range = 20;
  uint strength = 300;
};

struct AirhammerConfig : public WeaponConfig
{
  uint range = 30;
};

struct GrappleConfig : public EmptyWeaponConfig
{
  uint max_rope_length = 450; // Max rope length in pixels
  int push_force = 10;
};

static inline auto bindExplosiveWeaponConfig(EmptyWeaponConfig &ewc) {
    auto explosion = std::make_unique<ConfigElementList>();
    if (auto e = dynamic_cast<ExplosiveWeaponConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("timeout", &e->timeout, 0));
        explosion->emplace_back(new BoolConfigElement("allow_change_timeout", &e->allow_change_timeout, false));
        explosion->emplace_back(new DoubleConfigElement("explosion_range", &e->explosion_range, 0, 0, 200));
        explosion->emplace_back(new DoubleConfigElement("particle_range", &e->particle_range, 0));
        explosion->emplace_back(new DoubleConfigElement("blast_range", &e->blast_range, 0));
        explosion->emplace_back(new DoubleConfigElement("blast_force", &e->blast_force, 0));
        explosion->emplace_back(new DoubleConfigElement("speed_on_hit", &e->speed_on_hit, 0));
    }

    if (auto e = dynamic_cast<GrappleConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("max_rope_length", &e->max_rope_length, 450, 200, 800));
        explosion->emplace_back(new IntConfigElement("push_force", &e->push_force, 10));
    }
    if (auto e = dynamic_cast<MineConfig *>(&ewc)) {
        explosion->emplace_back(new DoubleConfigElement("detection_range", &e->detection_range, 1));
        explosion->emplace_back(new DoubleConfigElement("speed_detection", &e->speed_detection, 2));
        explosion->emplace_back(new UintConfigElement("timeout", &e->timeout, 2));
        explosion->emplace_back(new UintConfigElement("escape_time", &e->escape_time, 2, 1, 4));
    }
    if (auto e = dynamic_cast<FootBombConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("nb_fragments", &e->nb_fragments, 2, 1, 4));
        explosion->emplace_back(new UintConfigElement("nb_recursions", &e->nb_recursions, 2));
        explosion->emplace_back(new DoubleConfigElement("nb_angle_dispersion", &e->nb_angle_dispersion, 0));
        explosion->emplace_back(new DoubleConfigElement("nb_min_speed", &e->nb_min_speed, 0));
        explosion->emplace_back(new DoubleConfigElement("nb_max_speed", &e->nb_max_speed, 0));
    }
    if (auto e = dynamic_cast<CluzookaConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("nb_fragments", &e->m_fragments, 5, 2, 10));
        explosion->emplace_back(new UintConfigElement("nb_angle_dispersion", &e->m_angle_dispersion, 45));
    }
    if (auto e = dynamic_cast<ClusterBombConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("nb_fragments", &e->nb_fragments, 5, 2, 10));
    }
    if (auto e = dynamic_cast<AutomaticBazookaConfig *>(&ewc)) {
        explosion->emplace_back(new DoubleConfigElement("uncontrolled_turn_speed", &e->uncontrolled_turn_speed, PI*8));
        explosion->emplace_back(new DoubleConfigElement("max_controlled_turn_speed", &e->max_controlled_turn_speed, PI*4));
        explosion->emplace_back(new DoubleConfigElement("fuel_time", &e->fuel_time, 10));
        explosion->emplace_back(new DoubleConfigElement("rocket_force", &e->rocket_force, 2500));
    }
    if (auto e = dynamic_cast<AirAttackConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("nbr_obus", &e->nbr_obus, 3, 1, 8));
        explosion->emplace_back(new DoubleConfigElement("speed", &e->speed, 3));
    }
    if (auto e = dynamic_cast<SuperTuxWeaponConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("speed", &e->speed, 600, 100, 1000));
    }
    if (auto e = dynamic_cast<BlowtorchConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("range", &e->range, 20, 10, 30));
    }
    if (auto e = dynamic_cast<SyringeConfig *>(&ewc)) {
        explosion->emplace_back(new DoubleConfigElement("range", &e->range, 45, 0, 100));
        explosion->emplace_back(new UintConfigElement("turns", &e->turns, 10, 1, 50));
    }
    if (auto e = dynamic_cast<ParachuteConfig *>(&ewc)) {
        explosion->emplace_back(new DoubleConfigElement("wind_factor", &e->wind_factor, 10));
        explosion->emplace_back(new DoubleConfigElement("air_resist_factor", &e->air_resist_factor, 140));
        explosion->emplace_back(new DoubleConfigElement("force_side_displacement", &e->wind_factor, 2000));
    }
    if (auto e = dynamic_cast<SlapConfig *>(&ewc)) {
        explosion->emplace_back(new DoubleConfigElement("range", &e->range, 20, 1, 50));
        explosion->emplace_back(new UintConfigElement("strength", &e->strength, 300, 100, 500));
    }
    if (auto e = dynamic_cast<AirhammerConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("range", &e->range, 30, 1, 50));
    }
    if (auto e = dynamic_cast<BaseballConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("range", &e->range, 70));
        explosion->emplace_back(new UintConfigElement("strength", &e->strength, 2500, 500, 4000));
    }
    if (auto e = dynamic_cast<WeaponConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("damage", &e->damage, 10, 0, 2000));
    }
    return explosion;
}

//-----------------------------------------------------------------------------
#endif
