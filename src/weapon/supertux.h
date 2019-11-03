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
 * Weapon Supertux : Look ! it's the famous flying magic pinguin !
 *****************************************************************************/

#ifndef SUPERTUX_H
#define SUPERTUX_H

#include "weapon/weapon_launcher.h"
#include "particles/particle.h"

class SuperTux;

class SuperTuxWeaponConfig;

class TuxLauncher : public WeaponLauncher
{
private:
  SuperTux * current_tux;
  uint tux_death_time;

public:
  TuxLauncher();

  void UpdateTranslationStrings() override;
  std::string GetWeaponWinString(const char *TeamName, uint items_count ) const override;

  void SignalEndOfProjectile() override;
  bool IsReady() const override { return !IsOnCooldownFromShot() && WeaponLauncher::IsReady(); }
  bool IsOnCooldownFromShot() const override { return (current_tux || tux_death_time); }

  void StartShooting() override;
  void StopShooting() override;

  bool IsPreventingLRMovement() override { return IsOnCooldownFromShot(); }
  bool IsPreventingJumps() override { return IsOnCooldownFromShot(); }
  bool IsPreventingWeaponAngleChanges() override { return IsOnCooldownFromShot(); }

protected:
  WeaponProjectile * GetProjectileInstance() override;
  bool p_Shoot() override;
  void Refresh() override;
  bool ShouldBeDrawn() override { return !(current_tux || tux_death_time); }
private:
  SuperTuxWeaponConfig& cfg();
};

#endif
