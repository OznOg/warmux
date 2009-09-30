/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
  bool move_left_pressed;
  bool move_right_pressed;

public:
  TuxLauncher();
  virtual bool IsInUse() const;

  virtual void UpdateTranslationStrings();
  virtual std::string GetWeaponWinString(const char *TeamName, uint items_count ) const;

  virtual void SignalEndOfProjectile();

  virtual void StartMovingLeft() {move_left_pressed = true;};
  virtual void StopMovingLeft() {move_left_pressed = false;};

  virtual void StartMovingRight() {move_right_pressed = true;};
  virtual void StopMovingRight() {move_right_pressed = false;};

  void StartShooting();
  void StopShooting();

  virtual void HandleKeyPressed_MoveRight(bool slowly);
  virtual void HandleKeyReleased_MoveRight(bool slowly);

  virtual void HandleKeyPressed_MoveLeft(bool slowly);
  virtual void HandleKeyReleased_MoveLeft(bool slowly);

  virtual void HandleKeyPressed_Up(bool slowly);
  virtual void HandleKeyReleased_Up(bool slowly);

  virtual void HandleKeyPressed_Down(bool slowly);
  virtual void HandleKeyReleased_Down(bool slowly);

  virtual void HandleKeyPressed_Jump();
  virtual void HandleKeyReleased_Jump();

  virtual void HandleKeyPressed_HighJump();
  virtual void HandleKeyReleased_HighJump();

  virtual void HandleKeyPressed_BackJump();
  virtual void HandleKeyReleased_BackJump();

  void RefreshFromNetwork(double angle, Point2d pos);
  void ExplosionFromNetwork(Point2d tux_pos);

protected:
  WeaponProjectile * GetProjectileInstance();
  virtual bool p_Shoot();
  virtual void Refresh();
private:
  SuperTuxWeaponConfig& cfg();
};

#endif
