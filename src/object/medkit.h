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
 * Medkit
 *****************************************************************************/

#ifndef MEDKIT_H
#define MEDKIT_H
//-----------------------------------------------------------------------------
#include <WARMUX_base.h>
#include "object/objbox.h"
#include "tool/config_element.h"

class Team;
class Character;

//-----------------------------------------------------------------------------
struct MedkitSettings
{
  int nbr_health = 41;
  int start_points = 24;
};

class Medkit : public ObjBox
{
  static std::weak_ptr<Sprite> g_icon;
  MedkitSettings settings;

  std::shared_ptr<Sprite> icon;

  void ApplyMedkit(Team &team, Character &character) const;
public:
  Medkit(const MedkitSettings &settings);

  void ApplyBonus(Character *) override;
  const Surface* GetIcon() const override;
};

//-----------------------------------------------------------------------------
#endif /* MEDKIT_H */
