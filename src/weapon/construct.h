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
 * Add a structure to the ground
 *****************************************************************************/

#ifndef CONSTRUCT_H
#define CONSTRUCT_H
#include "weapon.h"
#include <WARMUX_base.h>
#include <WARMUX_point.h>
#include "graphic/sprite.h"

class Sprite;
struct WeaponConfig;

class Construct : public Weapon
{
private:
  bool target_chosen;
  std::unique_ptr<Sprite> construct_spr;
  Double angle;
  Point2i dst;

  void Up() const;
  void Down() const;

protected:
  bool p_Shoot() override;
  void Refresh() override { };
  bool ShouldBeDrawn() override { return false; };

public:
  Construct();
  void Draw() override;
  void ChooseTarget(Point2i mouse_pos) override;

  bool IsPreventingWeaponAngleChanges() override { return true; };
  void HandleKeyPressed_Down(bool /*slowly*/) override { Down(); };
  void HandleKeyPressed_Up(bool /*slowly*/) override { Up(); };
  void HandleMouseWheelUp(bool) override { Up(); };
  void HandleMouseWheelDown(bool) override { Down(); };

  // Implemeting a method that would otherwise have required RTTI
  void SetAngle(Double _angle) override { angle = _angle; }; // to be used by network

  void UpdateTranslationStrings() override;
  std::string GetWeaponWinString(const char *TeamName, uint items_count) const override;
  WeaponConfig& cfg();
};

#endif
