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
 ******************************************************************************/

#ifndef WEAPON_STRENGTH_BAR_H
#define WEAPON_STRENGTH_BAR_H

#include "gui/progress_bar.h"
#include "tool/resource_manager.h"

class Sprite;
class PolygonItem;
class DecoratedBox;

class WeaponStrengthBar : public ProgressBar
{
  std::shared_ptr<Profile> profile;
  bool visible;
  std::unique_ptr<DecoratedBox> m_box;
  std::unique_ptr<Sprite> last_fire;
  PolygonItem *m_item_last_fire;
  void FetchData();

public:
  WeaponStrengthBar(std::shared_ptr<Profile> profile);
  ~WeaponStrengthBar() override;
  void DrawXY(const Point2i &pos) const override;
  void InitPos (uint x, uint y, uint larg, uint haut) override;
  Color ComputeValueColor(int val) const;
  bool IsVisible() const { return visible; }
};

#endif // WEAPON_STRENGTH_BAR_H
