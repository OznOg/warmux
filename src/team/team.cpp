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
 * A team
 *****************************************************************************/
#include <sstream>
#include <iostream>

#include <WARMUX_action.h>
#include <WARMUX_debug.h>
#include <WARMUX_file_tools.h>
#include <WARMUX_point.h>

#include "ai/ai_stupid_player.h"
#include "character/character.h"
#include "character/body_list.h"
#include "game/config.h"
#include "game/game.h"
#include "game/game_mode.h"
#include "graphic/sprite.h"
#include "include/constant.h"
#include "interface/cursor.h"
#include "map/camera.h"
#include "map/map.h"
#include "network/network.h"
#include "sound/jukebox.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"
#include "weapon/weapons_list.h"
#include <algorithm>

std::unique_ptr<Team> Team::LoadTeam(const std::string &teams_dir, const std::string &id, std::string& error)
{
  std::string nomfich = teams_dir+id+ PATH_SEPARATOR "team.xml";
  std::string name;
  XmlReader   doc;

  // Load XML
  if (!doc.Load(nomfich)) {
    error = "Unable to load file of team data";
    return nullptr;
  }

  if (!XmlReader::ReadString(doc.GetRoot(), "name", name)) {
    error = "Invalid file structure: cannot find a name for team";
    return nullptr;
  }

  auto res = GetResourceManager().LoadXMLProfile(nomfich, true);
  if (!res) {
    error = "Invalid file structure: cannot load resources";
    return nullptr;
  }

  // The constructor will unload res
  return std::make_unique<Team>(doc, res, name, id);
}

Team::Team(XmlReader& doc, std::shared_ptr<Profile> res,
           const std::string& name, const std::string &id)
  : m_id(id)
  , m_name(name)
  , m_player_name("")
  , remote(false)
  , ai(nullptr)
  , ai_name(NO_AI_NAME)
  , active_weapon(nullptr)
  , abandoned(false)
  , team_color(white_color)
  , group(0)
  , energy(this)
{
  // Load team color
  team_color = res->LoadColor("teamcolor");

  // Load flag
  flag = res->LoadImage("flag");
  mini_flag = flag.RotoZoom(ZERO, ONE_HALF, ONE_HALF);
  death_flag = res->LoadImage("death_flag");
  big_flag = res->LoadImage("big_flag");

  // Get sound profile
  if (!XmlReader::ReadString(doc.GetRoot(), "sound_profile", m_sound_profile))
    m_sound_profile = "default";

  // read body id and default character name for each character
  xmlNodeArray nodes = XmlReader::GetNamedChildren(XmlReader::GetMarker(doc.GetRoot(), "team"), "character");
  xmlNodeArray::const_iterator it = nodes.begin();
  do {
    std::string character_name = "Unknown Soldier";
    std::string body_id = "";
    XmlReader::ReadStringAttr(*it, "name", character_name);
    XmlReader::ReadStringAttr(*it, "body", body_id);

    default_characters_names.push_back(character_name);
    bodies_ids.push_back(body_id);
    ++it;
  } while (it != nodes.end());

  active_character = characters.end();
  nb_characters = GameMode::GetInstance()->nb_characters;
}

void Team::AddOnePlayingCharacter(const std::string& character_name, Body *body)
{
  auto new_char = std::make_unique<Character>(*this, character_name, body,
                                              GameMode::GetInstance()->character_cfg,
                                              Config::GetInstance()->GetObjectConfig("character", ""));

  if (!new_char->PutRandomly(false, GetWorld().GetDistanceBetweenCharacters())) {
    // We haven't found any place to put the characters!!
    if (!new_char->PutRandomly(false, GetWorld().GetDistanceBetweenCharacters() / 2)) {
      std::cerr << std::endl;
      std::cerr << "Error: player " << character_name.c_str() << " will be probably misplaced!" << std::endl;
      std::cerr << std::endl;

      // Put it with no space...
      new_char->PutRandomly(false, 0);
    }
  }
  new_char->Init();

  characters.emplace_back(std::move(new_char));

  MSG_DEBUG("team", "Add %s in team %s", character_name.c_str(), m_name.c_str());
}

bool Team::AddPlayingCharacters(const std::vector<std::string> names)
{
  // Check that we have enough information
  if (names.size() < nb_characters || bodies_ids.size() < nb_characters)
    return false;

  characters.clear();

  // Time to effectively create the characters
  BodyList bl;
  for (uint i = 0; i < nb_characters; i++) {
    Body *body = bl.GetBody(bodies_ids[i]);
    if (!body) {
      std::cerr << Format(_("Error: can't find the body \"%s\" for the team \"%s\"."),
                          bodies_ids[i].c_str(),
                          m_name.c_str())
                << std::endl;
      return false;
    }

    // Create a new character and add him to the team
    AddOnePlayingCharacter(names[i], body);
  }
  active_character = characters.begin();

  return characters.size() == nb_characters;
}

bool Team::LoadCharacters()
{
  ASSERT(characters.size() == 0);
  ASSERT(bodies_ids.size() >= nb_characters);
  ASSERT(nb_characters <= 10);

  // handle custom names for characters
  std::vector<std::string> *characters_name = &default_characters_names;
  if (custom_characters_names.size() >= nb_characters) {
    characters_name = &custom_characters_names;
  }

  return AddPlayingCharacters(*characters_name);
}

uint Team::ReadEnergy() const
{
  uint total_energy = 0;

  for (auto &c : characters) {
    if (!c->IsDead())
      total_energy += c->GetEnergy();
  }

  return total_energy;
}

void Team::SelectCharacter(Character * c)
{
  ASSERT(c != nullptr);

  if (!c->IsActiveCharacter()) {
    ActiveCharacter().StopPlaying();

    active_character = characters.begin();
    while (c != active_character->get() && active_character != characters.end())
      active_character++;

    ASSERT(active_character != characters.end());
  }

  // StartPlaying (if needed) even if c was already ActiveCharacter() thanks to
  // the team change...
  c->StartPlaying();
}

void Team::NextCharacter(bool newturn)
{
  if (0 == NbAliveCharacter()) {
      Abandon();
      return;
  }

  ActiveCharacter().StopPlaying();

  // we change character:
  // - if user asked so
  // - if it's a new turn and game mode requests a change of character
  if (!newturn || GameMode::GetInstance()->auto_change_character) {

    do {
      ++active_character;
      if (active_character == characters.end())
        active_character = characters.begin();
    } while (ActiveCharacter().IsDead());
  }
  ActiveCharacter().StartPlaying();

  Camera::GetInstance()->CenterOnActiveCharacter();

  MSG_DEBUG("team", "%s (%d, %d)is now the active character",
            ActiveCharacter().GetName().c_str(),
            ActiveCharacter().GetX(),
            ActiveCharacter().GetY());
}

void Team::PreviousCharacter()
{
  ASSERT(0 < NbAliveCharacter());
  ActiveCharacter().StopPlaying();
  do {
    if (active_character == characters.begin())
      active_character = characters.end();
    --active_character;
  } while (ActiveCharacter().IsDead());

  ActiveCharacter().StartPlaying();

  Camera::GetInstance()->FollowObject(&ActiveCharacter());
  MSG_DEBUG("team", "%s (%d, %d)is now the active character",
            ActiveCharacter().GetName().c_str(),
            ActiveCharacter().GetX(),
            ActiveCharacter().GetY());
}

int Team::NbAliveCharacter() const
{
  uint nbr = 0;

  for (auto &c : characters)
    if (!c->IsDead())
        nbr++;

  return nbr;
}

// Prepare a new team turn
void Team::PrepareTurn()
{
  current_turn++;

  // Get a living character if possible
  if (ActiveCharacter().IsDead()) {
    NextCharacter();
  }

  Camera::GetInstance()->FollowObject(&ActiveCharacter(),true);
  CharacterCursor::GetInstance()->FollowActiveCharacter();

  // Updating weapon ammos (some weapons are not available from the beginning)
  for (const auto &w : weapons_list->GetList()) {
    if (w->AvailableAfterTurn() == (int)current_turn) {
      // this weapon is available now
      weaponStatus[ w->GetType() ].m_nb_ammos += w->ReadInitialNbAmmo();
      weaponStatus[ w->GetType() ].m_nb_units += w->ReadInitialNbUnit();
    }
  }

  // Active last weapon use if EnoughAmmo
  if (AccessWeapon().EnoughAmmo())
    AccessWeapon().Select();
  else { // try to find another weapon !!
    active_weapon = weapons_list->GetWeapon(Weapon::WEAPON_BAZOOKA);
    AccessWeapon().Select();
  }

  // Sound the bell, so the local players know when it is their turn
  if (IsLocalHuman())
    JukeBox::GetInstance()->Play("default", "start_turn");
  if (ai != nullptr)
    ai->PrepareTurn();
}

void Team::SetWeapon(Weapon::Weapon_type type)
{
  ASSERT (type >= Weapon::FIRST && type <= Weapon::LAST);
  AccessWeapon().Deselect();
  active_weapon = weapons_list->GetWeapon(type);
  AccessWeapon().Select();
}

Character* Team::FindByIndex(uint index)
{
  ASSERT(index < characters.size());

  for (auto &c : characters) {
      if (index == 0)
          return c.get();
    index--;
  }
  return nullptr;
}

void Team::LoadGamingData(WeaponsList &weapons)
{
  weapons_list = &weapons;
  current_turn = 0;

  // Reset ammos
  weaponStatus.clear();
  auto &l_weapons_list = weapons_list->GetList() ;

  weaponStatus.assign(l_weapons_list.size(), { 0, 0 });

  for (const auto &w : l_weapons_list) {
    if (w->AvailableAfterTurn() == 0) {
      // this weapon is available now
      weaponStatus[ w->GetType() ] = { w->ReadInitialNbAmmo(), w->ReadInitialNbUnit() };
    } else {
      // this weapon will be available later
    }
  }

  // Disable non-working weapons in network games
  if(Network::GetInstance()->IsConnected()) {
    //m_nb_ammos[Weapon::WEAPON_GRAPPLE] = 0;
  }

  active_weapon = weapons_list->GetWeapon(Weapon::WEAPON_DYNAMITE);

  abandoned = false;
  LoadCharacters();
}

void Team::UnloadGamingData()
{
  // Clear list of characters
  characters.clear();

  if (ai) {
    delete ai;
    ai = nullptr;
  }
  weapons_list = nullptr;
}

void Team::LoadAI()
{
  ASSERT(IsLocalAI());
  if (ai)
    delete ai;

  float accuracy;
  if (ai_name == DEFAULT_AI_NAME) accuracy = 0.9f;
  if (ai_name == DUMB_AI_NAME)    accuracy = 0.8f;
  if (ai_name == STRONG_AI_NAME)  accuracy = 0.95f;
  ai = new AIStupidPlayer(this, accuracy);
}

void Team::RefreshAI()
{
  if (ai != nullptr)
    ai->Refresh();
}

bool Team::IsActiveTeam() const
{
  return this == &ActiveTeam();
}

void Team::SetDefaultPlayingConfig()
{
  SetPlayerName("");
  ClearCustomCharactersNames();
  SetNbCharacters(GameMode::GetInstance()->nb_characters);
  SetRemote(false);
  SetAIName(NO_AI_NAME);
}

void Team::SetCustomCharactersNamesFromAction(Action *a)
{
  uint nb_custom_names = a->PopInt();
  if (nb_custom_names == 0)
    return;

  std::vector<std::string> custom_names;
  std::string name;
  for (uint i = 0; i < nb_custom_names; i++) {
    name = a->PopString();
    custom_names.push_back(name);
  }
  SetCustomCharactersNames(custom_names);
}

void Team::PushCustomCharactersNamesIntoAction(Action *a) const
{
  uint nb_custom_names = custom_characters_names.size();
  a->Push(nb_custom_names);
  for (uint i = 0; i < nb_custom_names; i++) {
    a->Push(custom_characters_names[i]);
  }
}

bool Team::GetWeaponBySort(bool open_map, Weapon::category_t sort, Weapon::Weapon_type &type)
{
  auto &m_weapons_list = weapons_list->GetList();
  auto it = m_weapons_list.begin();
  if (active_weapon->Category() == sort) {
      /* find the current position */
      it = std::find_if(m_weapons_list.begin(),
                     m_weapons_list.end(),
                     [this] (const auto &w) { return active_weapon == w.get(); });
      it++;
  }
  auto start_it = it;
  it = std::find_if(it, m_weapons_list.end(), [&](const auto &w) {
              return w->Category() == sort
                      && ReadNbAmmos(w->GetType()) != 0
                      && (open_map  || w->CanBeUsedOnClosedMap());
          });

  /* Ok, a weapon was found let's return it */
  if (it != m_weapons_list.end()) {
      type = (*it)->GetType();
      return true;
  }

  if (start_it == m_weapons_list.begin()) {
      return false;
  } 

  /* we didn't find a valid weapon after the current one ; lets wrap:
   * restart from the begining and try to find the first one matching
   * our criteria */
  it = std::find_if(m_weapons_list.begin(), start_it, [&](const auto &w) {
              return w->Category() == sort
                      && ActiveTeam().ReadNbAmmos(w->GetType()) != 0
                      && (open_map  || w->CanBeUsedOnClosedMap());
          });

  /* Ok, a weapon was found let's return it */
  if (it != m_weapons_list.end()) {
      type = (*it)->GetType();
      return true;
  }

  /* we definitly found nothing... */
  return false;
}

