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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

template<class... WeaponType>
void WeaponsList::allocate() {
  auto unused = { (m_weapons_list.emplace_back(new WeaponType), 0)... };
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
          Grapple, Blowtorch, Syringe>();

  Init(weapons_xml);
}

void WeaponsList::Init(const xmlNode* weapons_xml) const
{
  for (auto &it : m_weapons_list)
    it->LoadXml(weapons_xml);
}

bool WeaponsList::Save(XmlWriter& writer, xmlNode* weapons_xml) const
{
  for (auto &it : m_weapons_list) {
    if (!it->SaveXml(writer, weapons_xml))
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
