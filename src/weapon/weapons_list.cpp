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
 * Refresh des armes.
 *****************************************************************************/

#include "weapon/weapons_list.h"
//-----------------------------------------------------------------------------
#include <algorithm>
#include "weapon/all.h"
#include "weapon/explosion.h"
#include "interface/interface.h"
#include "map/camera.h"
#include "map/maps_list.h"
#include "object/objects_list.h"
#include "team/macro.h"
#include "team/team.h"
#include "tool/resource_manager.h"
#include "network/randomsync.h"
#include "tool/math_tools.h"
#include "graphic/sprite.h"

#include <iostream>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

bool _SaveXml(Weapon &w, XmlWriter& writer, xmlNode*  weapon)
{
  xmlNode* elem = XmlWriter::AddNode(weapon, w.m_id.c_str());
  if (!elem) {
    fprintf(stderr, "Couldn't save weapon config for %s\n", w.m_id.c_str());
    return false;
  }

  writer.WriteElement(elem, "available_after_turn", int2str(w.m_available_after_turn));
  writer.WriteElement(elem, "nb_ammo", int2str(w.m_initial_nb_ammo));
  if (w.m_initial_nb_unit_per_ammo!=1)
    writer.WriteElement(elem, "unit_per_ammo", int2str(w.m_initial_nb_unit_per_ammo));
  writer.WriteElement(elem, "ammo_per_drop", int2str(w.ammo_per_drop));
  writer.WriteElement(elem, "drop_probability", Double2str(w.drop_probability));
  // max strength
  // if max_strength = 0, no strength_bar !
  if (w.max_strength.IsNotZero())
    writer.WriteElement(elem, "max_strength", Double2str(w.max_strength));

  // change weapon after ? (for the grapple = true)
  if (w.m_can_change_weapon)
    writer.WriteElement(elem, "change_weapon", bool2str(w.m_can_change_weapon));

  // Disable crosshair ?
  if (w.m_display_crosshair)
    writer.WriteElement(elem, "display_crosshair", bool2str(w.m_display_crosshair));
  // angle of weapon when drawing
  // if (min_angle == max_angle) no cross_hair !
  // between -90 to 90 degrees
  int min_angle_deg = uround(w.min_angle * 180 / PI);
  int max_angle_deg = uround(w.max_angle * 180 / PI);
  if (min_angle_deg || max_angle_deg) {
    writer.WriteElement(elem, "min_angle", int2str(min_angle_deg));
    writer.WriteElement(elem, "max_angle", int2str(max_angle_deg));
  }

  // Save extra parameters if existing
  if (w.extra_params)
      bindExplosiveWeaponConfig(*w.extra_params)->SaveXml(writer, elem);

  return true;
}

bool WeaponsList::Save(XmlWriter& writer, xmlNode* weapons_xml) const
{
  for (auto &it : m_weapons_list) {
    if (!_SaveXml(*it, writer, weapons_xml))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

void WeaponsList::UpdateTranslation() const
{
  for (auto &it : m_weapons_list)
    it->UpdateTranslationStrings();
}

//-----------------------------------------------------------------------------

const Weapon &WeaponsList::GetRandomWeaponToDrop()
{
  Double probability_sum = 0;

  for (auto &w : m_weapons_list) {
    probability_sum += w->GetDropProbability();
  }
  ASSERT(probability_sum > 0);

  MSG_DEBUG("random.get", "WeaponList::GetRandomWeaponToDrop()");
  Double num = RandomSync().GetDouble(0, probability_sum);
  Double total_bf_weapon = 0;
  Double total_after_weapon = 0;

  for (auto &weapon : m_weapons_list) {
    total_after_weapon = total_bf_weapon + weapon->GetDropProbability();
    if (total_bf_weapon < num && num <= total_after_weapon) {
      MSG_DEBUG("bonus","Weapon choosed: %s", weapon->GetName().c_str());
      return *weapon;
    }
    total_bf_weapon = total_after_weapon;
  }
  ASSERT(false);
}

Weapon* WeaponsList::GetWeapon(Weapon::Weapon_type type) const
{
  auto it = std::find_if(m_weapons_list.begin(), m_weapons_list.end(), [&] (const auto &w) { return w->GetType() == type; });
  ASSERT (it != m_weapons_list.end());
  return it->get();
}

const WeaponLauncher* WeaponsList::GetWeaponLauncher(Weapon::Weapon_type type) const
{
  auto it = std::find_if(m_weapons_list.begin(), m_weapons_list.end(), [&] (const auto &w) { return w->GetType() == type; });
  ASSERT (it != m_weapons_list.end());
  auto wl = dynamic_cast<WeaponLauncher *>(it->get());
  ASSERT (wl != nullptr);
  return wl;
}

//-----------------------------------------------------------------------------
