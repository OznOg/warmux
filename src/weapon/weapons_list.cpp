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

template<class W>
std::unique_ptr<Weapon> _LoadXml(const xmlNode*  weapon)
{
  auto w = std::make_unique<W>();
  const xmlNode* elem = XmlReader::GetMarker(weapon, w->m_id);
  if (!elem) {
    std::cout << Format(_("No element <%s> found in the xml config file!"),
                        w->m_id.c_str())
              << std::endl;
    return nullptr;
  }

  XmlReader::ReadInt(elem, "available_after_turn", w->m_available_after_turn);
  XmlReader::ReadInt(elem, "nb_ammo", w->m_initial_nb_ammo);
  XmlReader::ReadInt(elem, "unit_per_ammo", w->m_initial_nb_unit_per_ammo);
  XmlReader::ReadInt(elem, "ammo_per_drop", w->ammo_per_drop);
  XmlReader::ReadDouble(elem, "drop_probability", w->drop_probability);
  if (w->m_initial_nb_ammo == INFINITE_AMMO && w->drop_probability.IsNotZero()) {
    std::cerr << Format(_("The weapon %s has infinite ammo, but bonus boxes might contain ammo for it!"), w->m_id.c_str());
    std::cerr << std::endl;
  }

  // max strength
  // if max_strength = 0, no strength_bar !
  XmlReader::ReadDouble(elem, "max_strength",w-> max_strength);

  // change weapon after ? (for the grapple = true)
  XmlReader::ReadBool(elem, "change_weapon", w->m_can_change_weapon);

  // Disable crosshair ?
  XmlReader::ReadBool(elem, "display_crosshair", w->m_display_crosshair);
  // angle of weapon when drawing
  // if (min_angle == max_angle) no cross_hair !
  // between -90 to 90 degrees
  int min_angle_deg = 0, max_angle_deg = 0;
  XmlReader::ReadInt(elem, "min_angle", min_angle_deg);
  XmlReader::ReadInt(elem, "max_angle", max_angle_deg);
  w->min_angle = min_angle_deg * PI / 180;
  w->max_angle = max_angle_deg * PI / 180;
  if (EqualsZero(w->min_angle - w->max_angle))
    w->m_display_crosshair = false;
#if 0
  if (m_weapon_fire) {
    uint num = 32 * (max_angle - min_angle) / TWO_PI;
    m_weapon_fire->EnableCaches(true, num, min_angle, max_angle);
  }
#endif

  // Load extra parameters if existing
  if (w->extra_params)
      bindExplosiveWeaponConfig(*w->extra_params)->LoadXml(elem);

  if (w->drawable && w->origin == Weapon::weapon_origin_HAND)
    w->m_image->SetRotation_HotSpot(w->position);

  return w;
}

template<class... WeaponType>
void WeaponsList::allocate(const xmlNode* weapons_xml) {
  auto unused = { (m_weapons_list.emplace_back(_LoadXml<WeaponType>(weapons_xml)), 0)... };
  (void)unused;
}

WeaponsList::WeaponsList(const xmlNode* weapons_xml)
{
  allocate<AnvilLauncher, TuxLauncher, GnuLauncher,
          PolecatLauncher, BounceBallLauncher, Bazooka, AutomaticBazooka,
          GrenadeLauncher, DiscoGrenadeLauncher, ClusterLauncher, FootBombLauncher,
          RiotBomb, Cluzooka, SubMachineGun, Gun, Shotgun, SnipeRifle, RailGun,
          Dynamite, FlameThrower, Mine, Baseball, AirAttack, Slap, Teleportation,
          Parachute, Suicide, SkipTurn, JetPack, Airhammer, Construct, LowGrav,
          Grapple, Blowtorch, Syringe>(weapons_xml);
}

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

bool WeaponsList::GetWeaponBySort(Weapon::category_t sort, Weapon::Weapon_type &type)
{
  bool open = ActiveMap()->LoadedData()->IsOpened();

  auto it  = m_weapons_list.begin();
  if (ActiveTeam().GetWeapon().Category() == sort) {
      /* find the current position */
      it = std::find_if(m_weapons_list.begin(),
                     m_weapons_list.end(),
                     [] (const auto &w) { return &ActiveTeam().GetWeapon() == w.get(); });
      it++;
  }
  auto start_it = it;
  it = std::find_if(it, m_weapons_list.end(), [&](const auto &w) {
              return w->Category() == sort
                      && ActiveTeam().ReadNbAmmos(w->GetType()) != 0
                      && (open  || w->CanBeUsedOnClosedMap());
          });

  /* Ok, a weapon was found let's return it */
  if (it != m_weapons_list.end()) {
      type = (*it)->GetType();
      return true;
  }

  if (start_it == m_weapons_list.begin()) {
      return false;
  } 

  /* we didn't find a valid weapon after the current one ; lets wrap:
   * restart from the begining and try to find the first one matching
   * our criteria */
  it = std::find_if(m_weapons_list.begin(), start_it, [&](const auto &w) {
              return w->Category() == sort
                      && ActiveTeam().ReadNbAmmos(w->GetType()) != 0
                      && (open  || w->CanBeUsedOnClosedMap());
          });

  /* Ok, a weapon was found let's return it */
  if (it != m_weapons_list.end()) {
      type = (*it)->GetType();
      return true;
  }

  /* we definitly found nothing... */
  return false;
}

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
