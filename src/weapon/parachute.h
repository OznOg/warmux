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
 * Parachute
 *****************************************************************************/

#ifndef PARACHUTE_H
#define PARACHUTE_H

#include "weapon.h"

class Sprite;
class ParachuteConfig;

//-----------------------------------------------------------------------------

class Parachute : public Weapon
{
  private:
    bool open;
    bool closing;

    Sprite* img;
  protected:
    bool m_used_this_turn;
    void p_Select() override;
    void p_Deselect() override;
    void Refresh() override;
    bool p_Shoot() override;
  public:
    Parachute();
    ~Parachute() override;
    void Draw() override;
    bool ShouldBeDrawn() override { return false; };

    void StartShooting() override;

    void UpdateTranslationStrings() override;
    std::string GetWeaponWinString(const char *TeamName, uint items_count ) const override;

    ParachuteConfig& cfg();
};

#endif /* PARACHUTE_H */
