/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Damage statistics for a character
 *****************************************************************************/

#ifndef _DAMAGE_STATS_H
#define _DAMAGE_STATS_H

class Character;

class DamageStatistics
{
  const Character& owner;

  int  damage_other_teams;
  int  damage_friendly_fire; // damage same team but not itself
  int  damage_itself;
  int  max_damage;
  int  current_total_damage;

public:
  DamageStatistics(const Character& _owner);
  DamageStatistics(const DamageStatistics& adamage_stats,
		   const Character& _owner);

  void MadeDamage(const int Dmg, const Character &other);
  void HandleMostDamage();

  int  GetMostDamage() const { return max_damage; }
  int  GetFriendlyFireDamage() const { return damage_friendly_fire; }
  int  GetItselfDamage() const { return damage_itself; }
  int  GetOthersDamage() const { return damage_other_teams; }
};

#endif
