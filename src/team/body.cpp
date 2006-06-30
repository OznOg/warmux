/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
#include "body.h"
#include <sstream>
#include <iostream>
#include <map>
#include "clothe.h"
#include "member.h"
#include "teams_list.h"
#include "../game/time.h"
#include "../tool/resource_manager.h"
#include "../tool/xml_document.h"
#include "../tool/random.h"

Body::Body(xmlpp::Element* xml, Profile* res)
{
  current_clothe = NULL;
  current_mvt = NULL;
  walk_events = 0;
  animation_number = 0;

  // Load members
  xmlpp::Node::NodeList nodes = xml -> get_children("sprite");
  xmlpp::Node::NodeList::iterator it=nodes.begin();

  while(it != nodes.end())
   {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it);
    std::string name;
    LitDocXml::LitAttrString( elem, "name", name);

    Member* member = new Member(elem, res);
    members_lst[name] = member;

    it++;
  }

  // Add a special weapon member to the body
  members_lst["weapon"] = new WeaponMember();

  // Load clothes
  xmlpp::Node::NodeList nodes2 = xml -> get_children("clothe");
  xmlpp::Node::NodeList::iterator it2=nodes2.begin();

  while(it2 != nodes2.end())
  {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it2);
    std::string name;
    LitDocXml::LitAttrString( elem, "name", name);

    Clothe* clothe = new Clothe(elem, members_lst);
    clothes_lst[name] = clothe;

    it2++;
  }

  // Load movements
  xmlpp::Node::NodeList nodes3 = xml -> get_children("movement");
  xmlpp::Node::NodeList::iterator it3=nodes3.begin();

  while(it3 != nodes3.end())
  {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it3);
    std::string name;
    LitDocXml::LitAttrString( elem, "name", name);
    if(strncmp(name.c_str(),"animation", 9)==0)
      animation_number++;

    Movement* mvt = new Movement(elem);
    mvt_lst[name] = mvt;

    it3++;
  }
}

Body::Body(Body *_body)
{
  current_clothe = NULL;
  current_mvt = NULL;
  walk_events = 0;
  animation_number = _body->animation_number;

  // Clean the members list
  std::map<std::string, Member*>::iterator it1 = _body->members_lst.begin();
  while(it1 != _body->members_lst.end())
  {
    std::pair<std::string,Member*> p;
    p.first = it1->first;
    p.second = it1->second;
    members_lst.insert(p);
    it1++;
  }

  // Clean the clothes list
  std::map<std::string, Clothe*>::iterator it2 = _body->clothes_lst.begin();
  while(it2 != _body->clothes_lst.end())
  {
    std::pair<std::string,Clothe*> p;
    p.first = it2->first;
    p.second = it2->second;
    clothes_lst.insert(p);
    it2++;
  }

  // Clean the movements list
  std::map<std::string, Movement*>::iterator it3 = _body->mvt_lst.begin();
  while(it3 != _body->mvt_lst.end())
  {
    std::pair<std::string,Movement*> p;
    p.first = it3->first;
    p.second = it3->second;
    mvt_lst.insert(p);
    it3++;
  }
}

Body::~Body()
{
  // Pointers inside those lists are freed from the body_list
  members_lst.clear();
  clothes_lst.clear();
  mvt_lst.clear();
}

void Body::ResetMovement()
{
  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
  if(current_clothe->layers[layer]->name != "weapon")
    current_clothe->layers[layer]->ResetMovement();
}

void Body::ApplyMovement(Movement* mvt, uint frame)
{
  // Move each member following the movement description
  // We do it using the order of the squeleton, as the movement of each
  // member affect the child members as well
  std::vector<junction>::iterator member = squel_lst.begin();
  for(;member != squel_lst.end();
       member++)
  {
    assert( frame < mvt->frames.size() );
    if(mvt->frames[frame].find(member->member->type) != mvt->frames[frame].end())
    {
      // This member needs to be moved :
      member_mvt mb_mvt = mvt->frames[frame].find(member->member->type)->second;
      if(mb_mvt.follow_crosshair && ActiveCharacter().body == this && ActiveTeam().AccessWeapon().UseCrossHair())
      {
        // Use the movement of the crosshair
        int angle = ActiveTeam().crosshair.GetAngle(); // returns -180 < angle < 180
        if(angle < 0)
          angle += 360; // so now 0 < angle < 360;
        if(ActiveCharacter().GetDirection() == -1)
          angle = 180 - angle;

        mb_mvt.angle += angle ;
      }

      if(mb_mvt.follow_speed)
      {
        // Use the movement of the character
        int angle = (int)(owner->GetSpeedAngle()/M_PI*180.0);
        if(angle < 0)
          angle += 360; // so now 0 < angle < 360;
        if(owner->GetDirection() == -1)
          angle = 180 - angle;

        mb_mvt.angle += angle;
      }

      if(mb_mvt.follow_direction)
      {
        // Use the direction of the character
        if(owner->GetDirection() == -1)
          mb_mvt.angle += 180;
      }


      member->member->ApplyMovement(mb_mvt, squel_lst);
    }
  }
}

void Body::ApplySqueleton(const Point2f& pos)
{
  // Move each member following the squeleton
  std::vector<junction>::iterator member = squel_lst.begin();
  // The first member is the body, we set it to pos:
  member->member->pos = pos;
  member->member->angle = 0;
  member++;

  for(;member != squel_lst.end();
       member++)
  {
    // Place the other members depending of the parent member:
    member->member->ApplySqueleton(member->parent);
  }
}

void Body::Draw(const Point2i& _pos)
{
  Point2f pos;
  pos.x = (float) _pos.x;
  pos.y = (float) _pos.y;

  // Increase frame number if needed
  if(walk_events > 0 || current_mvt->type!="walk")
  if(Time::GetInstance()->Read() > last_refresh + current_mvt->speed)
  {
    current_frame += (Time::GetInstance()->Read()-last_refresh) / current_mvt->speed;
    last_refresh += ((Time::GetInstance()->Read()-last_refresh) / current_mvt->speed) * current_mvt->speed;
    if(current_frame != current_frame % current_mvt->frames.size())
    {
      if(play_once_clothe_sauv)
        SetClothe(play_once_clothe_sauv->name);
      if(play_once_mvt_sauv)
      {
        SetMovement(play_once_mvt_sauv->type);
        current_frame = play_once_frame_sauv;
      }
    }
    current_frame %= current_mvt->frames.size();
  }

  ResetMovement();
  ApplySqueleton(pos);
  ApplyMovement(current_mvt, current_frame);

  // Rotate each sprite, because the next part need to know the height of the sprite
  // once he is rotated
  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
  if(current_clothe->layers[layer]->name != "weapon")
    current_clothe->layers[layer]->RotateSprite();

  // Move the members to get the lowest member at the bottom of the skin rectangle
  member_mvt body_mvt;
  float y_max = 0;
  for(int lay=0;lay < (int)current_clothe->layers.size() ;lay++)
  if(current_clothe->layers[lay]->name != "weapon")
  {
    Member* member = current_clothe->layers[lay];
    if(member->pos.y + member->spr->GetHeightMax() + member->spr->GetRotationPoint().y > y_max)
      y_max = member->pos.y + member->spr->GetHeightMax() + member->spr->GetRotationPoint().y;
  }
  body_mvt.pos.y = pos.y + (float)GetSize().y - y_max + current_mvt->test_bottom;

  // And center the "body" horizontally in the object:
  body_mvt.pos.x = GetSize().x / 2.0 - squel_lst.front().member->spr->GetWidth() / 2.0;
  squel_lst.front().member->ApplyMovement(body_mvt, squel_lst);

  // update the weapon position
  if(direction == 1)
    weapon_pos = Point2i((int)current_weapon_member->pos.x,(int)current_weapon_member->pos.y);
  else
    weapon_pos = Point2i(2 * (int)pos.x + GetSize().x - (int)current_weapon_member->pos.x,(int)current_weapon_member->pos.y);

  // Finally draw each layer one by one
  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
    current_clothe->layers[layer]->Draw((int)pos.x + GetSize().x/2, direction);
}

void Body::AddChildMembers(Member* parent)
{
  // Add child members of the parent member to the squeleton
  // and continue recursively with child members
  for(std::map<std::string, v_attached>::iterator child = parent->attached_members.begin();
      child != parent->attached_members.end();
      child++)
  {
    // Find if the current clothe uses this member:
    for(uint lay = 0; lay < current_clothe->layers.size(); lay++)
    {
      if(current_clothe->layers[lay]->type == child->first)
      {
        // This child member is attached to his parent
        junction body;
        body.member = current_clothe->layers[lay];
        body.parent = parent;
        squel_lst.push_back(body);

        // continue recursively
        AddChildMembers(current_clothe->layers[lay]);
      }
      if(current_clothe->layers[lay]->name == "weapon")
        current_weapon_member = current_clothe->layers[lay];
    }
  }
}

void Body::BuildSqueleton()
{
  // Find each member used by the current clothe
  // and set the parent member of each member
  squel_lst.clear();

  // Find the "body" member as its the top of the squeleton
  for(uint lay = 0; lay < current_clothe->layers.size(); lay++)
  if(current_clothe->layers[lay]->type == "body")
  {
    junction body;
    body.member = current_clothe->layers[lay];
    body.parent = NULL;
    squel_lst.push_back(body);
  }

  if(squel_lst.size() == 0)
  {
    std::cerr << "Unable to find the \"body\" member in the current clothe" << std::endl;
    assert(false);
  }

  AddChildMembers(squel_lst.front().member);
}

void Body::SetClothe(std::string name)
{
  if(current_clothe && current_clothe->name == name) return;

  if(clothes_lst.find(name) != clothes_lst.end())
  {
    current_clothe = clothes_lst.find(name)->second;
    BuildSqueleton();
  }

  play_once_clothe_sauv = NULL;

  assert(current_clothe != NULL);
}

void Body::SetMovement(std::string name)
{
  if(current_mvt && current_mvt->type == name) return;

  if(mvt_lst.find(name) != mvt_lst.end())
  {
    current_mvt = mvt_lst.find(name)->second;
    current_frame = 0;
    last_refresh = Time::GetInstance()->Read();
  }

  play_once_mvt_sauv = NULL;

  assert(current_mvt != NULL);
}

void Body::PlayAnimation()
{
  std::ostringstream name;
  name << "animation" << randomObj.GetLong(0, animation_number - 1);
  SetClotheOnce(name.str());
  SetMovementOnce(name.str());
}

void Body::SetClotheOnce(std::string name)
{
  if(current_clothe && current_clothe->name == name) return;

  if(clothes_lst.find(name) != clothes_lst.end())
  {
    play_once_clothe_sauv = current_clothe;
    current_clothe = clothes_lst.find(name)->second;
    BuildSqueleton();
  }

  assert(current_clothe != NULL);
}

void Body::SetMovementOnce(std::string name)
{
  if(current_mvt && current_mvt->type == name) return;

  if(mvt_lst.find(name) != mvt_lst.end())
  {
    play_once_mvt_sauv = current_mvt;
    current_mvt = mvt_lst.find(name)->second;
    play_once_frame_sauv = current_frame;
    current_frame = 0;
    last_refresh = Time::GetInstance()->Read();
  }

  assert(current_mvt != NULL);
}

void Body::GetTestRect(uint &l, uint&r, uint &t, uint &b)
{
  if(direction == 1)
  {
    l = current_mvt->test_left;
    r = current_mvt->test_right;
  }
  else
  {
    r = current_mvt->test_left;
    l = current_mvt->test_right;
  }
  t = current_mvt->test_top;
  b = current_mvt->test_bottom;
}

void Body::SetDirection(int dir)
{
  direction=dir;
}

const int Body::GetDirection()
{
  return direction;
}

const Point2i Body::GetHandPosition()
{
  return weapon_pos;
}

void Body::StartWalk()
{
  walk_events++;
  if(walk_events == 1)
    last_refresh = Time::GetInstance()->Read();
}

void Body::StopWalk()
{
  if(walk_events > 0)
    walk_events--;
}

void Body::ResetWalk()
{
  walk_events = 0;
}

uint Body::GetMovementDuration()
{
  return current_mvt->frames.size() * current_mvt->speed;
}

uint Body::GetFrameCount()
{
  return current_mvt->frames.size();
}

void Body::SetFrame(uint no)
{
  assert(no < current_mvt->frames.size());
  current_frame = no;
}

const std::string& Body::GetMovement() { return current_mvt->type; }
const std::string& Body::GetClothe() { return current_clothe->name; }
