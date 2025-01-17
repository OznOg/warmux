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
 * Character of a team.
 *****************************************************************************/
#include "character/body.h"
#include "character/character.h"
#include "character/clothe.h"
#include "character/member.h"
#include "character/movement.h"
#include "game/game_time.h"
#include "graphic/sprite.h"
#include "interface/mouse.h"
#include "network/randomsync.h"
#include "particles/body_member.h"
#include "particles/teleport_member.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/resource_manager.h"
#include "tool/string_tools.h"
#include <WARMUX_debug.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>

Body::Body(const xmlNode *     xml,
           std::string  main_folder):
  current_clothe(nullptr),
  current_mvt(nullptr),
  current_loop(0),
  current_frame(0),
  previous_clothe(nullptr),
  previous_mvt(nullptr),
  mvt_locked(false),
  weapon_member(new WeaponMember()),
  last_refresh(0),
  walking(false),
  main_rotation_rad(0),
  direction(DIRECTION_RIGHT),
  animation_number(0),
  need_rebuild(false),
  need_refreshsprites(true),
  owner(nullptr),
  mainXmlNode(xml),
  mainFolder(std::move(main_folder))
{
  // Add a special weapon member to the body
  members_lst["weapon"] = std::unique_ptr<Member>(weapon_member);
}

Body::Body(const Body & _body):
  current_clothe(nullptr),
  current_mvt(nullptr),
  current_loop(0),
  current_frame(0),
  previous_clothe(nullptr),
  previous_mvt(nullptr),
  mvt_locked(false),
  weapon_member(new WeaponMember()),
  last_refresh(0),
  walking(false),
  main_rotation_rad(0),
  direction(DIRECTION_RIGHT),
  animation_number(_body.animation_number),
  need_rebuild(true),
  owner(nullptr)
{
  // Add a special weapon member to the body
  members_lst["weapon"] = std::unique_ptr<Member>(weapon_member);

  // Make a copy of members
  auto it1 = _body.members_lst.begin();

  while (it1 != _body.members_lst.end()) {
    if (it1->second->GetName() != "weapon") {
      members_lst.emplace(it1->first, new Member(*it1->second));
    }
    ++it1;
  }

  // Make a copy of clothes
  auto it2 = _body.clothes_lst.begin();
  while (it2 != _body.clothes_lst.end()) {
    clothes_lst.emplace(it2->first, std::make_unique<Clothe>(it2->second.get(), members_lst));
    ++it2;
  }

  // Movements are shared
  mvt_lst = _body.mvt_lst;
}

/* Here to keep forward declarations of private stuff */
Body::~Body() = default;

void Body::Init() {
  const xmlNode * skeletons = XmlReader::GetMarker(mainXmlNode, "skeletons");
  ASSERT(skeletons);

  xmlNodeArray nodes = XmlReader::GetNamedChildren(skeletons, "sprite");

  LoadMembers(nodes, mainFolder);
  LoadClothes(nodes, mainXmlNode);
  LoadMovements(nodes, mainXmlNode);
}

void Body::LoadMembers(xmlNodeArray &      nodes,
                       const std::string & main_folder)
{
  MSG_DEBUG("body", "Found " SIZET_FORMAT "u sprites", nodes.size());
  std::string                  name;
  xmlNodeArray::const_iterator it = nodes.begin();

  for ( ; it != nodes.end(); ++it) {
    XmlReader::ReadStringAttr(*it, "name", name);
    MSG_DEBUG("body", "Loading member %s", name.c_str());

    if (members_lst.find(name) != members_lst.end()) {
      std::cerr << "Warning !! The member \""<< name << "\" is defined twice in the xml file" << std::endl;
      ASSERT(false);
    } else {
      members_lst[name] = std::make_unique<Member>(*it, main_folder);
    }
  }
}

void Body::LoadClothes(xmlNodeArray &  nodes,
                       const xmlNode * xml)
{
  const xmlNode * clothes = XmlReader::GetMarker(xml, "clothes");
  ASSERT(clothes);

  nodes = XmlReader::GetNamedChildren(clothes, "clothe");
  MSG_DEBUG("body", "Found " SIZET_FORMAT "u clothes", nodes.size());
  std::string name;
  xmlNodeArray::const_iterator it = nodes.begin();

  for ( ; it != nodes.end(); ++it) {
    XmlReader::ReadStringAttr(*it, "name", name);
    //Clothe* clothe = new Clothe(*it, members_lst);

    if (clothes_lst.find(name) != clothes_lst.end()) {
      std::cerr << "Warning !! The clothe \""<< name << "\" is defined twice in the xml file" << std::endl;
    } else {
      clothes_lst[name] = std::make_unique<Clothe>(*it, members_lst);
    }
  }
}

void Body::LoadMovements(xmlNodeArray &  nodes,
                         const xmlNode * xml)
{
  //// Load movements alias
  const xmlNode * aliases = XmlReader::GetMarker(xml, "aliases");
  ASSERT(aliases);

  nodes = XmlReader::GetNamedChildren(aliases, "alias");
  MSG_DEBUG("body", "Found " SIZET_FORMAT "u aliases", nodes.size());

  std::map<std::string, std::string> mvt_alias;
  xmlNodeArray::const_iterator       it = nodes.begin();
  std::string                        mvt;
  std::string                        corresp;

  for (; it != nodes.end(); ++it) {
    XmlReader::ReadStringAttr(*it, "movement", mvt);
    XmlReader::ReadStringAttr(*it, "correspond_to", corresp);
    mvt_alias.insert(std::make_pair(mvt, corresp));
    MSG_DEBUG("body", "  %s -> %s", mvt.c_str(), corresp.c_str());
  }

  //// Load movements
  const xmlNode * movements = XmlReader::GetMarker(xml, "movements");
  ASSERT(movements);

  nodes = XmlReader::GetNamedChildren(movements, "movement");
  MSG_DEBUG("body", "Found " SIZET_FORMAT "u movements", nodes.size());
  std::string name;

  for (it = nodes.begin(); it != nodes.end(); ++it) {
    XmlReader::ReadStringAttr(*it, "name", name);
    if (0 == strncmp(name.c_str(),"animation", 9)) {
      animation_number++;
    }

    if (mvt_lst.find(name) != mvt_lst.end()) {
      std::cerr << "Warning !! The movement \""<< name << "\" is defined twice in the xml file" << std::endl;
    } else {
      mvt_lst[name] = std::make_shared<Movement>(*it);
    }

    auto iter = mvt_alias.begin();
    for ( ; iter != mvt_alias.end(); ++iter) {
      if (iter->second == name) {
        auto mvt = std::make_shared<Movement>(*it);
        mvt->SetType(iter->first);
        mvt_lst[iter->first] = mvt;
      }
    }

  }

  auto mvtBlack     = mvt_lst.find("black");
  auto clothesBlack = clothes_lst.find("black");

  if ((mvtBlack == mvt_lst.end() && clothesBlack != clothes_lst.end())
     || (mvtBlack != mvt_lst.end() && clothesBlack == clothes_lst.end())) {
    std::cerr << R"(Error: The movement "black" or the clothe "black" is not defined!)" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Body::ResetMovement() const
{
  for (auto &layer : current_clothe->GetLayers()) {
    layer->ResetMovement();
  }
}

void Body::ApplyMovement(Movement * mvt,
                         uint       frame)
{

#ifdef DEBUG
  if (mvt->GetType() != "breathe")
    MSG_DEBUG("body_anim", " %s uses %s-%s:%u",
              owner->GetName().c_str(),
              current_clothe->GetName().c_str(),
              mvt->GetType().c_str(),
              frame);
#endif

  auto member = skel_lst.begin();
  bool                              useCrossHair;
  Movement::member_def              movMember = mvt->GetFrames()[frame];

  // Move each member following the movement description
  // We do it using the order of the skeleton, as the movement of each
  // member affects the child members as well
  for (; member != skel_lst.end(); ++member) {
    ASSERT(frame < mvt->GetFrames().size());

    Member *mb = (*member)->member;
    auto itMember = std::find(movMember.begin(), movMember.end(), mb->GetType());

    if (itMember != movMember.end()) {

      // This member needs to be moved :
      member_mvt& mb_mvt = *itMember;

      useCrossHair = ActiveTeam().AccessWeapon().UseCrossHair();

      if (mb_mvt.follow_crosshair &&
          &ActiveCharacter().GetBody() == this &&
          useCrossHair) {
        ProcessFollowCrosshair(mb_mvt);
      } else if (mb_mvt.follow_half_crosshair &&
          &ActiveCharacter().GetBody() == this &&
          useCrossHair) {
        ProcessFollowHalfCrosshair(mb_mvt);
      } else if (mb_mvt.follow_speed) {
        ProcessFollowSpeed(mb_mvt);
      } else if (mb_mvt.follow_direction) {
        ProcessFollowDirection(mb_mvt);
      }

      mb->ApplyMovement(mb_mvt);

      // This movement needs to know the position of the member before
      // being applied so it does a second ApplyMovement after being used
      if (mb_mvt.follow_cursor_square_limit &&
          Mouse::GetInstance()->GetVisibility() == Mouse::MOUSE_VISIBLE) {
        ProcessFollowCursor(mb_mvt, mb);
      }

    }
  }
}

void Body::ProcessFollowCrosshair(member_mvt & mb_mvt)
{
  // Use the movement of the crosshair
  Double angle = owner->GetFiringAngle(); /* Get -2 * PI < angle =< 2 * PI*/
  if (ZERO > angle) {
    angle += TWO_PI; // so now 0 < angle < 2 * PI;
  }

  if (DIRECTION_LEFT == ActiveCharacter().GetDirection()) {
    angle = PI - angle;
  }

  mb_mvt.SetAngle(mb_mvt.GetAngle() + angle);
}

void Body::ProcessFollowHalfCrosshair(member_mvt & mb_mvt)
{
  // Use the movement of the crosshair
  Double angle_rad = owner->GetFiringAngle(); // returns -180 < angle < 180
  if (DIRECTION_RIGHT == ActiveCharacter().GetDirection())
    angle_rad *= ONE_HALF; // -90 < angle < 90
  else
  if (angle_rad > HALF_PI)
    angle_rad = HALF_PI - ONE_HALF * angle_rad;//formerly in deg to 45 + (90 - angle) / 2;
  else
    angle_rad = -HALF_PI - ONE_HALF * angle_rad;//formerly in deg to -45 + (-90 - angle) / 2;

  if (angle_rad < 0) {
    angle_rad += TWO_PI; // so now 0 < angle < 2 * PI;
  }

  mb_mvt.SetAngle(mb_mvt.GetAngle() + angle_rad);
}

void Body::ProcessFollowSpeed(member_mvt & mb_mvt)
{
  // Use the movement of the character
  Double angle_rad = owner->GetSpeedAngle();

  if (angle_rad < 0) {
    angle_rad += TWO_PI; // so now 0 < angle < 2 * PI;
  }

  if (owner->GetDirection() == DIRECTION_LEFT) {
    angle_rad = PI - angle_rad;
  }

  mb_mvt.SetAngle(mb_mvt.GetAngle() + angle_rad);
}

void Body::ProcessFollowDirection(member_mvt & mb_mvt)
{
  // Use the direction of the character
  if (DIRECTION_LEFT == owner->GetDirection()) {
    mb_mvt.SetAngle(mb_mvt.GetAngle() + PI);
  }
}

void Body::ProcessFollowCursor(const member_mvt& mb_mvt, Member* member)
{
  Point2i v = owner->GetPosition() + member->GetPos();
  Point2i zero(0, 0);
  v += member->GetAnchorPos();

  if (DIRECTION_LEFT == owner->GetDirection()) {
    v.x = (((int)owner->GetPosition().x)<<1) + (GetSize().x>>1) - v.x;
    //v.x -= member->GetSprite().GetWidth();
  }
  v = Mouse::GetInstance()->GetWorldPosition() - v;

  if (v.SquareDistance(zero) < mb_mvt.follow_cursor_square_limit) {
    Double angle = v.ComputeAngle(zero);

    if (owner->GetDirection() == DIRECTION_RIGHT) {
      angle -= PI;
    } else {
      angle = -angle;
    }

    member_mvt angle_mvt;
    angle_mvt.SetAngle(angle);
    member->ApplyMovement(angle_mvt);
  }
}

void Body::ApplySqueleton()
{
  // Move each member following the skeleton
  auto member = skel_lst.begin();

  // The first member is the body, we set it to pos:
  (*member)->member->SetPos(Point2i(0, 0));
  (*member)->member->SetAngle(ZERO);
  member++;

  for ( ; member != skel_lst.end();
       ++member) {
    // Place the other members depending on the parent member:
    (*member)->member->ApplySqueleton((*member)->parent);
  }
}

void Body::Build()
{
  // Increase frame number if needed
  uint last_frame = current_frame;

  if (walking || current_mvt->GetType() != "walk") {

    if (GameTime::GetInstance()->Read() > last_refresh + current_mvt->GetFrameDuration()) {

      // Compute the new frame number
      current_frame += (GameTime::GetInstance()->Read()-last_refresh) / current_mvt->GetFrameDuration();
      last_refresh += (current_frame-last_frame) * current_mvt->GetFrameDuration();

      // This is the end of the animation
      if (current_frame >= current_mvt->GetFrames().size()) {

        current_frame = 0;
        current_loop++;

        mvt_locked = false;

        // Number of loops
        if (current_mvt->GetNbLoops() && current_loop >= current_mvt->GetNbLoops()) {

          // animation is finished - set it to the very last frame
          current_loop = current_mvt->GetNbLoops() -1;
          current_frame = current_mvt->GetFrames().size() -1;

          if (previous_clothe) {
            SetClothe(previous_clothe->GetName());
            previous_clothe = nullptr;
          }
          if (previous_mvt) {
            SetMovement(previous_mvt->GetType());
            previous_mvt = nullptr;
          }
        }
      }
    }
  }

  if (last_frame == current_frame && !need_rebuild) {
    return;
  }
  need_refreshsprites = true;

  ResetMovement();
  ApplySqueleton();
  ApplyMovement(current_mvt, current_frame);

  int y_max = 0;
  const std::vector<Member*>& layers = current_clothe->GetNonWeaponLayers();
  for (auto &member : layers) {
    if (!member->IsGoingThroughGround()) {
      // Rotate sprite, because the next part need to know the height
      // of the sprite once it is rotated
      member->RotateSprite();

      // Move the members to get the lowest member at the bottom
      // of the skin rectangle
      int val = member->GetPos().y + member->GetSprite().GetHeightMax()
              + member->GetSprite().GetRotationPoint().y;
      if (val > y_max)
        y_max = val;
    }
  }

  member_mvt body_mvt;
  body_mvt.pos.y = GetSize().y - y_max + current_mvt->GetTestBottom();
  body_mvt.pos.x = (GetSize().x - skel_lst.front()->member->GetSprite().GetWidth())>>1;
  body_mvt.SetAngle(main_rotation_rad);
  skel_lst.front()->member->ApplyMovement(body_mvt);

  need_rebuild = false;
}

void Body::RefreshSprites()
{
  if (need_refreshsprites) {
    const std::vector<Member*>& layers = current_clothe->GetNonWeaponLayers();
    for (auto &layer : layers)
      layer->RefreshSprite(direction);

    need_refreshsprites = false;
  } else {
    const std::vector<Member*>& must = current_clothe->MustRefreshMembers();
    for (auto &layer : must)
      layer->RefreshSprite(direction);
  }
}

std::string Body::GetFrameLoop() const
{
  char str[32];
  snprintf(str, 32, "%u/%u-%u/%u",
           current_loop+1, current_mvt->GetNbLoops(),
           current_frame+1, (uint)current_mvt->GetFrames().size());

  return std::string(str);
}

void Body::GetRelativeHandPosition(Point2i& result) const
{
  if (direction == DIRECTION_RIGHT) {
    result = weapon_member->GetPos();
  } else {
    result.x = GetSize().x - weapon_member->GetPos().x;
    result.y = weapon_member->GetPos().y;
  }
}

void Body::DrawWeaponMember(const Point2i & _pos)
{
  weapon_member->Draw(_pos, _pos.x + GetSize().x/2, direction);
}

void Body::Draw(const Point2i & _pos)
{
  ASSERT(!need_rebuild);
  int draw_weapon_member = 0;

  // Finally draw each layer one by one
  const std::vector<Member*>& layers = current_clothe->GetLayers();
  for (auto &member : layers) {
    if (member == weapon_member) {
      // We draw the weapon member only if currently drawing the active character
      if (owner->IsActiveCharacter()) {
        assert(draw_weapon_member == 0);
        DrawWeaponMember(_pos);
        draw_weapon_member++;
      }
    } else {
      member->Draw(_pos, _pos.x + GetSize().x/2, direction);
    }
  }

  // if we are drawing the active character but current clothe does not contain a weapon member,
  // we should draw it else weapon ammos number will be not displayed (see bug #11479)
  if (owner->IsActiveCharacter() && draw_weapon_member == 0) {
    DrawWeaponMember(_pos);
  }

  // if this assertion fails then the body has been modified in this _draw_ method!!!
  ASSERT(!need_rebuild);
}

void Body::AddChildMembers(Member * parent)
{
  const Member::AttachTypeMap& attached = parent->GetAttachedTypes();
  auto child = attached.begin();

  // Add child members of the parent member to the skeleton
  // and continue recursively with child members
  const std::vector<Member*>& layers = current_clothe->GetLayers();
  for ( ; child != attached.end(); ++child) {

    // Find if the current clothe uses this member:
    for (auto &member : layers) {
      if (member->GetType() == child->first) {
        // This child member is attached to his parent
        auto * body = new junction();
        body->member = member;
        body->parent = parent;
        skel_lst.emplace_back(body);

        // continue recursively
        if (!member->GetAttachedTypes().empty()) {
          AddChildMembers(member);
        }
      }
    }
  }
}

void Body::BuildSqueleton()
{
  // Find each member used by the current clothe
  // and set the parent member of each member
  skel_lst.clear();

  // Find the "body" member as it is the top of the skeleton
  const std::vector<Member*>& layers = current_clothe->GetLayers();
  for (auto &member : layers) {
    if (member->GetType() == "body") {

      // TODO lami : overwrite junction constructor
      auto * body = new junction();
      body->member = member;
      body->parent = nullptr;
      skel_lst.emplace_back(body);
      break;
    }
  }

  if (skel_lst.empty()) {
    std::cerr << "Unable to find the \"body\" member in the current clothe" << std::endl;
    ASSERT(false);
  }

  AddChildMembers(skel_lst.front()->member);

  // Now that the skeleton is built, inform each member
  for (auto & i : skel_lst)
    i->member->BuildAttachMemberMap(skel_lst);

  need_refreshsprites = true;
}

void Body::SetClothe(const std::string & name)
{
  MSG_DEBUG("body", " %s use clothe %s", owner->GetName().c_str(), name.c_str());
  if (current_clothe && current_clothe->GetName() == name) {
    return;
  }

  auto itClothes = clothes_lst.find(name);

  if (itClothes != clothes_lst.end()) {
    current_clothe = itClothes->second.get();
    BuildSqueleton();
    main_rotation_rad = 0;
    need_rebuild      = true;
    previous_clothe   = nullptr;
  } else {
    MSG_DEBUG("body", "Clothe not found");
  }

  assert(current_clothe);
}

void Body::SetMovement(const std::string & name)
{
  MSG_DEBUG("body", " %s use movement %s", owner->GetName().c_str(), name.c_str());
  if (mvt_locked || (current_mvt && current_mvt->GetType() == name)) {
    return;
  }

  // Dirty trick to get the "black" movement to be played fully
  if (current_clothe && current_clothe->GetName() == "black" && GetMovement() == "black") {
    return;
  }
  auto itMvt = mvt_lst.find(name);

  if (itMvt != mvt_lst.end()) {
    current_mvt       = itMvt->second.get();
    current_frame     = 0;
    current_loop      = 0;
    last_refresh      = GameTime::GetInstance()->Read();
    main_rotation_rad = 0;
    need_rebuild      = true;
    previous_mvt      = nullptr;
  } else {
    MSG_DEBUG("body", "Movement not found");
  }

  assert(current_mvt);
}

void Body::PlayAnimation()
{
  std::ostringstream name;
  MSG_DEBUG("random.get", "Body::PlayAnimation()");
  name << "animation" << RandomSync().GetInt(0, animation_number - 1);
  //name << "animation0";
  SetClotheOnce(name.str());
  SetMovementOnce(name.str());
}

void Body::SetClotheOnce(const std::string & name)
{
  MSG_DEBUG("body", " %s use clothe %s once", owner->GetName().c_str(), name.c_str());
  if (current_clothe && current_clothe->GetName() == name) {
    return;
  }

  auto itClothes = clothes_lst.find(name);

  if (itClothes != clothes_lst.end()) {
    if (!previous_clothe) {
      previous_clothe = current_clothe;
    }
    current_clothe = itClothes->second.get();
    BuildSqueleton();
    main_rotation_rad = 0;
    need_rebuild = true;
  } else {
    MSG_DEBUG("body", "Clothe not found");
  }

  assert(current_clothe);
}

void Body::SetMovementOnce(const std::string & name)
{
  MSG_DEBUG("body", " %s use movement %s once", owner->GetName().c_str(), name.c_str());
  if (mvt_locked || (current_mvt && current_mvt->GetType() == name)) {
    return;
  }

  // Dirty trick to get the "black" movement to be played fully
  if(current_clothe && current_clothe->GetName() == "black"  && name != "black") {
    return;
  }

  auto itMvt = mvt_lst.find(name);

  if (itMvt != mvt_lst.end()) {
    if (!previous_mvt) {
      previous_mvt = current_mvt;
    }
    current_mvt = itMvt->second.get();
    current_frame = 0;
    current_loop = 0;
    last_refresh = GameTime::GetInstance()->Read();
    main_rotation_rad = 0;
    need_rebuild = true;
    if (name.compare(0, 9, "animation"))
      mvt_locked = true;
  } else {
    MSG_DEBUG("body", "Movement not found");
  }

  assert(current_mvt);
}

void Body::GetTestRect(uint & l,
                       uint & r,
                       uint & t,
                       uint & b) const
{
  if(DIRECTION_RIGHT == direction) {
    l = current_mvt->GetTestLeft();
    r = current_mvt->GetTestRight();
  } else {
    r = current_mvt->GetTestLeft();
    l = current_mvt->GetTestRight();
  }
  t = current_mvt->GetTestTop();
  b = current_mvt->GetTestBottom();
}

void Body::StartWalking()
{
  ASSERT(!walking);
  walking = true;
  last_refresh = GameTime::GetInstance()->Read();
}

void Body::StopWalking()
{
  ASSERT(walking);
  walking = false;

  if (current_mvt->GetType() == "walk") {
    SetMovement("breathe");
    SetFrame(0);
  }
}

uint Body::GetMovementDuration() const
{
  return current_mvt->GetFrames().size() * current_mvt->GetFrameDuration();
}

uint Body::GetFrameCount() const
{
  return current_mvt->GetFrames().size();
}

void Body::SetFrame(uint no)
{
#ifdef DEBUG
  if (no >= current_mvt->GetFrames().size()) {
    fprintf(stderr, "%s:%d Clothe: %s\n", __PRETTY_FUNCTION__, __LINE__, current_clothe->GetName().c_str());
    fprintf(stderr, "%s:%d Movement: %s\n", __PRETTY_FUNCTION__, __LINE__, current_mvt->GetType().c_str());
    fprintf(stderr, "%s:%d Frame requested: %d - Max nb frames: %d\n", __PRETTY_FUNCTION__, __LINE__,
            no,(int) current_mvt->GetFrames().size());
  }
#endif
  // no == current_frame occurs very infrequently (once when stopping walking)
  // that it is probably more costly to check for it every time
  ASSERT(no < current_mvt->GetFrames().size());
  current_frame = no;
  current_loop  = 0;
  need_rebuild  = true;
}

void Body::MakeParticles(const Point2i & pos)
{
  Build();

  const std::vector<Member*>& layers = current_clothe->GetNonWeaponLayers();
  for (auto &member : layers) {
    ParticleEngine::AddNow(std::make_unique<BodyMemberParticle>(member->GetSprite(), member->GetPos()+pos));
  }
}

void Body::MakeTeleportParticles(const Point2i& pos, const Point2i& dst)
{
  Build();

  const std::vector<Member*>& layers = current_clothe->GetNonWeaponLayers();
  for (auto &member : layers) {
    ParticleEngine::AddNow(std::make_unique<TeleportMemberParticle>(member->GetSprite(),
                                                      member->GetPos()+pos, member->GetPos()+dst));
  }
}

void Body::SetRotation(Double angle)
{
  MSG_DEBUG("body", "%s -> new angle: %.3f", owner->GetName().c_str(), angle.tofloat());
  // angle == main_rotation_rad is infrequent, but often enough
  if (main_rotation_rad != angle) {
    main_rotation_rad = angle;
    need_rebuild = true;
  }
}

const std::string& Body::GetMovement() const
{
  return current_mvt->GetType();
}

const std::string& Body::GetClothe() const
{
  return current_clothe->GetName();
}

void Body::DebugState() const
{
  MSG_DEBUG("body.state", "clothe: %s\tmovement: %s\t%i", current_clothe->GetName().c_str(),
            current_mvt->GetType().c_str(), current_frame);
  MSG_DEBUG("body.state", "(played once)clothe: %s\tmovement: %s",
            (previous_clothe?previous_clothe->GetName().c_str():"(NULL)"),
            (previous_mvt?previous_mvt->GetType().c_str():"(NULL)"));
  MSG_DEBUG("body.state", "need rebuild = %i",need_rebuild);
}
