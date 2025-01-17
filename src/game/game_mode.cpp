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
 * Warmux configuration : all main configuration variables have there default
 * value here. They should all be modifiable using the xml config file
 *****************************************************************************/

#include <iostream>
#include <cstdio>
#include <WARMUX_file_tools.h>
#include "game/config.h"
#include "game/game.h"
#include "game/game_mode.h"
#include "object/medkit.h"
#include "object/bonus_box.h"
#include "weapon/weapons_list.h"
#include "weapon/all.h"
#include "tool/math_tools.h"
#include "graphic/sprite.h"

GameMode::GameMode() = default;

std::unique_ptr<ConfigElementList> GameMode::BindMembers() const {
    // FIXME ConfigElements should be able to take const refrencs so that I do
    // not need const cast here... but this is an other story..
   return const_cast<GameMode *>(this)->BindMembers();
}

std::unique_ptr<ConfigElementList> GameMode::BindMembers() {
  auto main_settings = std::make_unique<ConfigElementList>();
  main_settings->emplace_back(new StringConfigElement("rules", &rules, "none"));
  main_settings->emplace_back(new BoolConfigElement("auto_change_character", &auto_change_character, true));
  main_settings->emplace_back(new StringConfigElement("allow_character_selection", &txt, "always"));
  main_settings->emplace_back(new UintConfigElement("duration_turn", &duration_turn, 60));
  main_settings->emplace_back(new UintConfigElement("duration_move_player", &duration_move_player, 3));
  main_settings->emplace_back(new UintConfigElement("duration_exchange_player", &duration_exchange_player, 2));
  main_settings->emplace_back(new UintConfigElement("duration_before_death_mode", &duration_before_death_mode, 20 * 60));
  main_settings->emplace_back(new UintConfigElement("damage_per_turn_during_death_mode", &damage_per_turn_during_death_mode, 5));
  main_settings->emplace_back(new UintConfigElement("max_teams", &max_teams, 8));
  main_settings->emplace_back(new UintConfigElement("nb_characters", &nb_characters, 6));
  main_settings->emplace_back(new IntConfigElement("gravity", &gravity, 30));
  main_settings->emplace_back(new IntConfigElement("safe_fall", &safe_fall, 10));
  main_settings->emplace_back(new UintConfigElement("damage_per_fall_unit", &damage_per_fall_unit, 7));

  auto char_settings = std::make_unique<ConfigElementList>();
  char_settings->emplace_back(new UintConfigElement("walking_pause", &character_cfg.walking_pause, 50));

  auto energy = std::make_unique<ConfigElementList>();
  energy->emplace_back(new UintConfigElement("initial", &character_cfg.init_energy, 100, 1, 200, true));
  energy->emplace_back(new UintConfigElement("maximum", &character_cfg.max_energy, 200, 1, 200, true));
  char_settings->LinkList(std::move(energy), "energy");
  char_settings->LinkList(bindExplosiveWeaponConfig(death_explosion_cfg), "death_explosion");

  auto jump = std::make_unique<ConfigElementList>();
  jump->emplace_back(new IntConfigElement("strength", &character_cfg.jump_strength, 8, true));
  jump->emplace_back(new AngleConfigElement("angle", &character_cfg.jump_angle, -60, true));
  char_settings->LinkList(std::move(jump), "jump");

  auto super_jump = std::make_unique<ConfigElementList>();
  super_jump->emplace_back(new IntConfigElement("strength", &character_cfg.super_jump_strength, 11, true));
  super_jump->emplace_back(new AngleConfigElement("angle", &character_cfg.super_jump_angle, -80, true));
  char_settings->LinkList(std::move(super_jump), "super_jump");

  auto back_jump = std::make_unique<ConfigElementList>();
  back_jump->emplace_back(new IntConfigElement("strength", &character_cfg.back_jump_strength, 9, true));
  back_jump->emplace_back(new AngleConfigElement("angle", &character_cfg.back_jump_angle, -100, true));
  char_settings->LinkList(std::move(back_jump), "back_jump");
  main_settings->LinkList(std::move(char_settings), "character");

  auto barrel = std::make_unique<ConfigElementList>();
  barrel->LinkList(bindExplosiveWeaponConfig(barrel_explosion_cfg), "explosion");
  main_settings->LinkList(std::move(barrel), "barrel");

  auto bonus_box = std::make_unique<ConfigElementList>();
  bonus_box->LinkList(bindExplosiveWeaponConfig(bonus_box_explosion_cfg), "explosion");
  main_settings->LinkList(std::move(bonus_box), "bonus_box");

  auto med_kit = std::make_unique<ConfigElementList>();
  med_kit->emplace_back(new IntConfigElement("life_points", &medkit_cfg.start_points, 41));
  med_kit->emplace_back(new IntConfigElement("energy_boost", &medkit_cfg.nbr_health, 24));
  main_settings->LinkList(std::move(med_kit), "medkit");
  main_settings->LinkList(bindExplosiveWeaponConfig(mines_explosion_cfg), "minelauncher");

  return main_settings;
}

template<class W>
std::unique_ptr<Weapon> _LoadXml(const xmlNode*  weapon)
{
  auto w = std::make_unique<W>();
  const xmlNode* elem = XmlReader::GetMarker(weapon, w->m_id);
  if (!elem) {
    std::cout << Format(_("No element <%s> found in the xml config file!"),
                        w->m_id.c_str())
              << std::endl;
    return nullptr;
  }

  XmlReader::ReadInt(elem, "available_after_turn", w->m_available_after_turn);
  XmlReader::ReadInt(elem, "nb_ammo", w->m_initial_nb_ammo);
  XmlReader::ReadInt(elem, "unit_per_ammo", w->m_initial_nb_unit_per_ammo);
  XmlReader::ReadInt(elem, "ammo_per_drop", w->ammo_per_drop);
  XmlReader::ReadDouble(elem, "drop_probability", w->drop_probability);
  if (w->m_initial_nb_ammo == INFINITE_AMMO && w->drop_probability.IsNotZero()) {
    std::cerr << Format(_("The weapon %s has infinite ammo, but bonus boxes might contain ammo for it!"), w->m_id.c_str());
    std::cerr << std::endl;
  }

  // max strength
  // if max_strength = 0, no strength_bar !
  XmlReader::ReadDouble(elem, "max_strength",w-> max_strength);

  // change weapon after ? (for the grapple = true)
  XmlReader::ReadBool(elem, "change_weapon", w->m_can_change_weapon);

  // Disable crosshair ?
  XmlReader::ReadBool(elem, "display_crosshair", w->m_display_crosshair);
  // angle of weapon when drawing
  // if (min_angle == max_angle) no cross_hair !
  // between -90 to 90 degrees
  int min_angle_deg = 0, max_angle_deg = 0;
  XmlReader::ReadInt(elem, "min_angle", min_angle_deg);
  XmlReader::ReadInt(elem, "max_angle", max_angle_deg);
  w->min_angle = min_angle_deg * PI / 180;
  w->max_angle = max_angle_deg * PI / 180;
  if (EqualsZero(w->min_angle - w->max_angle))
    w->m_display_crosshair = false;
#if 0
  if (m_weapon_fire) {
    uint num = 32 * (max_angle - min_angle) / TWO_PI;
    m_weapon_fire->EnableCaches(true, num, min_angle, max_angle);
  }
#endif

  // Load extra parameters if existing
  if (w->extra_params)
      bindExplosiveWeaponConfig(*w->extra_params)->LoadXml(elem);

  if (w->drawable && w->origin == Weapon::weapon_origin_HAND)
    w->m_image->SetRotation_HotSpot(w->position);

  return w;
}

template<class... WeaponType>
void _LoadWeapon(WeaponsList &m_weapons_list, const xmlNode* weapons_xml) {
  auto unused = { (m_weapons_list.GetList().emplace_back(_LoadXml<WeaponType>(weapons_xml)), 0)... };
  (void)unused;
}

// Load data options from the selected game_mode
bool GameMode::LoadXml(XmlReader &doc)
{
  const xmlNode *elem = doc.GetRoot();

  auto settings = BindMembers();
  // Load all elements
  settings->LoadXml(elem);

  if (txt == "always")
    allow_character_selection = ALWAYS;
  else if (txt == "never")
    allow_character_selection = NEVER;
  else if (txt == "before_first_action")
    allow_character_selection = BEFORE_FIRST_ACTION;
  else if (txt == "within_team")
    allow_character_selection = WITHIN_TEAM;
  else
    fprintf(stderr, "%s is not a valid option for \"allow_character_selection\"\n", txt.c_str());

  //=== Weapons ===
  elem = XmlReader::GetMarker(elem, "weapons");
  if (!elem)
    return false;
  
  weapons_list = std::make_unique<WeaponsList>();

  _LoadWeapon<AnvilLauncher, TuxLauncher, GnuLauncher,
          PolecatLauncher, BounceBallLauncher, Bazooka, AutomaticBazooka,
          GrenadeLauncher, DiscoGrenadeLauncher, ClusterLauncher, FootBombLauncher,
          RiotBomb, Cluzooka, SubMachineGun, Gun, Shotgun, SnipeRifle, RailGun,
          Dynamite, FlameThrower, Mine, Baseball, AirAttack, Slap, Teleportation,
          Parachute, Suicide, SkipTurn, JetPack, Airhammer, Construct, LowGrav,
          Grapple, Blowtorch, Syringe>(*weapons_list, elem);

  return bool(weapons_list);
}

bool GameMode::Load()
{
  Config * config = Config::GetInstance();

  XmlReader doc;
  return doc.Load(config->GetModeFilename()) && LoadXml(doc);
}

// Load the game mode from strings (probably from network)
bool GameMode::LoadFromString(const std::string& game_mode_name,
                              const std::string& game_mode_contents,
                              const std::string& game_mode_objects_contents)
{
  m_current = game_mode_name;
  MSG_DEBUG("game_mode", "Loading %s from network: ", m_current.c_str());

  if (!XmlReader().LoadFromString(game_mode_objects_contents))
    return false;

  XmlReader doc;
  return doc.LoadFromString(game_mode_contents) && LoadXml(doc);
}

bool GameMode::ExportToString(std::string& mode,
                              std::string& mode_objects) const
{
  XmlReader objdoc;

  objdoc.Load(Config::GetInstance()->GetObjectsFilename());
  mode_objects = objdoc.ExportToString();

  auto out = SaveXml();
  mode = out->SaveToString();
  return !mode_objects.empty() && !mode.empty();
}

bool GameMode::AllowCharacterSelection() const
{
  switch (allow_character_selection) {
  case GameMode::ALWAYS: break;

  case GameMode::BEFORE_FIRST_ACTION:
    return (Game::GetInstance()->ReadState() == Game::PLAYING) &&
            !Game::GetInstance()->IsCharacterAlreadyChosen();

  case GameMode::NEVER:
  case GameMode::WITHIN_TEAM:
  default:
    return false;
  }

  return true;
}

std::unique_ptr<XmlWriter> GameMode::SaveXml() const
{
  auto out = std::make_unique<XmlWriter>("game_mode", "1.0", "utf-8");
  xmlNode *node = out->GetRoot();
  auto settings = BindMembers();
  settings->SaveXml(*out, node);

  node = XmlWriter::AddNode(node, "weapons");
  weapons_list->Save(*out, node);

  return out;
}

bool GameMode::ExportToFile(const std::string& game_mode_name)
{
  Config * config = Config::GetInstance();
  std::string filename = std::string("game_mode" PATH_SEPARATOR)
                       + game_mode_name + std::string(".xml");

  auto out = SaveXml();
  if (!out)
    return false;

  std::string fullname = config->GetPersonalDataDir() + filename;
  return out->Save(fullname);
}

