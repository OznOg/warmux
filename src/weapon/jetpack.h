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
 * Jet Pack :-)
 *****************************************************************************/

#ifndef JETPACK_H
#define JETPACK_H

#include "weapon.h"

class JetPack : public Weapon
{
  private:
    bool m_flying;

    SoundSample flying_sound;

    // Jetpack fuel.
    uint m_last_fuel_down;
    bool active;

    bool IsInAir();

  public:
    JetPack();
    void Reset();

    void StartShooting() override;
    void StopShooting() override {};

    bool IsPreventingLRMovement() override;
    bool IsPreventingJumps() override;
    bool IsPreventingWeaponAngleChanges() override;

    void UpdateTranslationStrings() override;
    std::string GetWeaponWinString(const char *TeamName, uint items_count ) const override;


  protected:
    void Refresh() override;
    void p_Select() override;
    void p_Deselect() override;
    bool p_Shoot() override;
    bool ShouldBeDrawn() override { return false; };
    bool ShouldAmmoUnitsBeDrawn() const override;

  private:
    void GoUp();
    void StartFlying();
    void StopFlying();
};

#endif /* JETPACK_H */
