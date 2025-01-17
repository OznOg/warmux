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
 * Sprite cache.
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de)
 *             Initial version
 *****************************************************************************/

#ifndef _SPRITE_CACHE_H
#define _SPRITE_CACHE_H

#include <vector>
#include <assert.h>
#include <WARMUX_base.h>
#include "graphic/surface.h"

class Sprite;

#if 0 //def ANDROID
#  define RotoZoomC(a, x, y) RotoZoom(a, x, y).DisplayFormatColorKey(128)
#else
#  define RotoZoomC(a, x, y) RotoZoom(a, x, y)
#endif

class SpriteSubframeCache
{
  std::vector<Surface> rotated;
  Double min = 0, max = 0;

  Double RestrictAngle(Double angle) const
  {
    while (angle < min)
      angle += TWO_PI;
    while (angle >= max)
      angle -= TWO_PI;
    ASSERT(angle>=min && angle<max);
    return angle;
  }

public:
  Surface              surface;

  SpriteSubframeCache() = default;
  SpriteSubframeCache(const Surface& surf) : surface(surf) { };

  Surface GetSurfaceForAngle(Double angle);
  void SetCache(uint num, const Double& mini, const Double& maxi);
};

class SpriteFrameCache
{
public:
  uint delay;

  SpriteSubframeCache normal;
  SpriteSubframeCache flipped;

  SpriteFrameCache(uint d = 100) : delay(d) {}
  SpriteFrameCache(const Surface& surf, uint d = 100)
    : delay(d)
    , normal(surf)
  { }

  void SetCaches(bool flipped, uint rotation_num, Double min, Double max);
};

class SpriteCache : private std::vector<SpriteFrameCache>
{
  using Cont = std::vector<SpriteFrameCache>;
  uint rotation_cache_size = 0;
  bool have_flipping_cache = false;

public:
  using Cont::size;
  using Cont::operator[];

  //SpriteCache(Sprite&) ;

  void AddFrame(const Surface& surf, uint delay=100) { emplace_back(surf, delay); }
  void EnableCaches(bool flipped, uint rotation_num, const Double& min, const Double& max);

  //operator SpriteFrameCache& [](uint index) { return frames.at(index); }
  void SetDelay(uint delay)
  {
    for (auto &s : *this)
      s.delay = delay;
  }

  void FixParameters(const Double& rotation_rad,
                     const Double& scale_x, const Double& scale_y,
                     bool force_color_key);
  bool HasRotationCache() const { return rotation_cache_size; }
  bool HasFlippedCache() const { return have_flipping_cache; }
};

#endif /* _SPRITE_CACHE_H */
