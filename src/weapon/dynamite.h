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
 * Weapon dynamite : When fired, explode after a short laps of time. Then make a
 * big hole, eject character and made them lost energy.
 * Like a dynamite after all :)
 *****************************************************************************/

#ifndef DYNAMITE_H
#define DYNAMITE_H

#include "weapon/weapon_launcher.h"
#include <WARMUX_base.h>

// L'arme dynamite
class Dynamite : public WeaponLauncher
{
  protected:
    bool p_Shoot() override;
    std::unique_ptr<WeaponProjectile> GetProjectileInstance() override;
    bool ShouldBeDrawn() override;
  public:
    Dynamite();
    void UpdateTranslationStrings() override;
    std::string GetWeaponWinString(const char *TeamName, uint items_count) const override;
};
#endif /* DYNAMITE_H */
