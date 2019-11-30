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
class EmptyWeaponConfig : public ConfigElementList
{
};

//-----------------------------------------------------------------------------

class WeaponConfig : public EmptyWeaponConfig
{
public:
  uint damage;

public:
  WeaponConfig();
};

//-----------------------------------------------------------------------------

class ExplosiveWeaponConfig : public WeaponConfig
{
public:
  uint   timeout;
  bool   allow_change_timeout;
  Double blast_range;
  Double blast_force;
  Double explosion_range;
  Double particle_range;
  Double speed_on_hit;

public:
  ExplosiveWeaponConfig();
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

    if (auto e = dynamic_cast<WeaponConfig *>(&ewc)) {
        explosion->emplace_back(new UintConfigElement("damage", &e->damage, 10, 0, 2000));
    }
    return explosion;
}

//-----------------------------------------------------------------------------
#endif
