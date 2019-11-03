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
 * Mine : Detect if character is close and explode after a shot time.
 * Sometime the mine didn't explode randomly.
 *****************************************************************************/
#ifndef MINE_H
#define MINE_H

#include <WARMUX_singleton.h>
#include <WARMUX_base.h>
#include "weapon/weapon_cfg.h"
#include "weapon/weapon_launcher.h"

class Mine;
class MineConfig;

class ObjMine : public WeaponProjectile
{
  SoundSample timeout_sound;

  // this is a fake mine ?
  bool fake;

  // Is this mine active ?
  bool is_active;

  // Activation des mines ?
  bool animation;
  uint attente;
  uint escape_time;

protected:
  void FakeExplosion();
public:
  ObjMine(MineConfig &cfg,
          WeaponLauncher * p_launcher = NULL);

  void StartTimeout();
  void Detection();
  bool IsImmobile() const override;
  // Damage handling
  void SetEnergyDelta(int /*delta*/, Character * /*dealer*/) override;

  void Draw() override;
  void Refresh() override;
};

class MineConfig : public Singleton<MineConfig>, public ExplosiveWeaponConfig
{
protected:
  friend class Singleton<MineConfig>;
public:
  uint escape_time;
  Double detection_range;
  Double speed_detection;
private:
  MineConfig();
};

class Mine : public WeaponLauncher
{
private:
  void Add (int x, int y);
protected:
  WeaponProjectile * GetProjectileInstance() override;
  bool p_Shoot() override;
  bool ShouldBeDrawn() override;
public:
  void UpdateTranslationStrings() override;
  std::string GetWeaponWinString(const char *TeamName, uint items_count ) const override;
  Mine();
  MineConfig& cfg();
};

#endif /* MINE_H */
