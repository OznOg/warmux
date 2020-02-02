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
 * List of objects that are displayed.
 *****************************************************************************/

#include "object/objects_list.h"
//-----------------------------------------------------------------------------
#include "object/barrel.h"
#include "include/app.h"
#include "map/map.h"
#include "map/maps_list.h"
#include "map/camera.h"
#include <WARMUX_debug.h>
#include <WARMUX_rectangle.h>
#include "game/game_time.h"
#include "game/game_mode.h"
#include "weapon/mine.h"
#include <vector>
#include <iostream>

void ObjectsList::PlaceMines()
{
  MSG_DEBUG("lst_objects","Placing mines");
  for (uint i = 0; i < ActiveMap()->LoadedData().GetNbMine(); ++i)
  {
    auto obj = std::make_unique<ObjMine>(GameMode::GetInstance()->mines_explosion_cfg);
    Double detection_range_factor = 1.5;
    if (obj->PutRandomly(false, GameMode::GetInstance()->mines_explosion_cfg.detection_range * PIXEL_PER_METER * detection_range_factor)) {
      // detection range is in meter
      emplace_back(std::move(obj));
    }
  }
}

void ObjectsList::PlaceBarrels()
{
  MSG_DEBUG("lst_objects","Placing barrels");
  for (uint i = 0; i < ActiveMap()->LoadedData().GetNbBarrel(); ++i)
  {
    auto obj = std::make_unique<PetrolBarrel>();

    if (obj->PutRandomly(false, 20.0))
      emplace_back(std::move(obj));
  }
}


//-----------------------------------------------------------------------------
void ObjectsList::Refresh()
{
  auto object = begin();

  while (object != end())
  {
    (*object)->UpdatePosition();

    if (!(*object)->IsGhost()) {
      // Update position may lead to a Ghost object, we
      // must not to refresh in that case
      (*object)->Refresh();
    }

    if ((*object)->IsGhost()) {
      // Stop following this object, remove from overlapse reference then delete it.
      Camera::GetInstance()->StopFollowingObj(object->get());
      RemoveOverlappedObjectReference(*object->get());
      object = erase(object);
    } else {
      object++;
    }
  }
}

//-----------------------------------------------------------------------------
void ObjectsList::Draw()
{
  for (auto & it : *this) {
    ASSERT(it != nullptr);

    if (!it->IsGhost())
      it->Draw();
  }
}

//-----------------------------------------------------------------------------
bool ObjectsList::AllReady() const
{
  for (auto &obj : *this) {
    if (!obj->IsGhost() && !obj->IsImmobile()) {
      MSG_DEBUG("lst_objects", "\"%s\" is not ready ( IsImmobile()==false )",
                obj->GetName().c_str());
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------

void ObjectsList::RemoveOverlappedObjectReference(const PhysicalObj &obj)
{
  for (auto it = overlapped_objects.begin(); it != overlapped_objects.end();) {

    if ((*it)->GetOverlappingObject() == &obj) {
      MSG_DEBUG("lst_objects", "removing overlapse reference of \"%s\" (%p) in \"%s\"",
                obj.GetName().c_str(), &obj, (*it)->GetName().c_str());
      (*it)->SetOverlappingObject(nullptr);
      it = overlapped_objects.erase(it);

    } else if (*it == &obj) {
      MSG_DEBUG("lst_objects", "removing overlapse object of \"%s\" (%p)",
                obj.GetName().c_str(), &obj);
      it = overlapped_objects.erase(it);
    } else {
      ++it;
    }
  }
}

void ObjectsList::AddOverlappedObject(PhysicalObj * obj)
{
  MSG_DEBUG("lst_objects", "adding overlapsed object \"%s\" %p",
            obj->GetName().c_str(), obj);

  overlapped_objects.push_back(obj);
}

void ObjectsList::RemoveOverlappedObject(PhysicalObj * obj)
{
  MSG_DEBUG("lst_objects", "removing overlapsed object \"%s\" %p",
            obj->GetName().c_str(), obj);

  overlapped_objects.remove(obj);
}
