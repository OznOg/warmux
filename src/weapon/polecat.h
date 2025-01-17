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
 * Polecat : send a polecat to the enemy. Close character get sick with the mefitic odor.
 *****************************************************************************/

#ifndef POLECAT_H
#define POLECAT_H

#include "weapon/weapon_launcher.h"
#include <WARMUX_base.h>

class Polecat;

class PolecatLauncher : public WeaponLauncher
{
private:
  Polecat *current_polecat;
  uint polecat_death_time;

public:
  PolecatLauncher();

  void SignalEndOfProjectile() override;
  void SignalProjectileCollision() override { };
  void SignalProjectileDrowning() override { };

  bool IsOnCooldownFromShot() const override;
  bool IsReady() const override;

  void StopShooting() override;

  bool IsPreventingLRMovement() override;
  bool IsPreventingJumps() override;
  bool IsPreventingWeaponAngleChanges() override;

  void UpdateTranslationStrings() override;
  std::string GetWeaponWinString(const char *TeamName, uint items_count ) const override;
protected:
  bool p_Shoot() override;
  void Refresh() override;
  std::unique_ptr<WeaponProjectile> GetProjectileInstance() override;
};

#endif /* POLECAT_H */
