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
 * Teleportation.
 *****************************************************************************/

#ifndef TELEPORTATION_H
#define TELEPORTATION_H

#include "weapon.h"

class WeaponConfig;

class Teleportation : public Weapon
{
  private:
    bool target_chosen;
    Point2i src, dst;
    bool done;
  protected:
    bool p_Shoot() override;
    void p_Select() override;
    void Refresh() override;
    virtual bool ShouldBeVisible();
  public:
    Teleportation();
    void ChooseTarget(Point2i mouse_pos) override;
    void StartShooting() override {}
    void StopShooting() override {}
    WeaponConfig& cfg();

    void UpdateTranslationStrings() override;
    std::string GetWeaponWinString(const char *TeamName, uint items_count ) const override;
};

#endif /* TELEPORTATION_H */
