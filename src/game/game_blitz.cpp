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
 * Specialization of Game methods for the blitz mode.
 *****************************************************************************/

#include "character/character.h"
#include "game/game_blitz.h"
#include "game/game_mode.h"
#include "game/game_time.h"
#include "include/action_handler.h"
#include "interface/cursor.h"
#include "interface/interface.h"
#include "interface/game_msg.h"
#include "map/camera.h"
#include "map/map.h"
#include "map/maps_list.h"
#include "map/wind.h"
#include "network/network.h"
#include "sound/jukebox.h"
#include "team/macro.h"
#include "team/team.h"
#include "team/teams_list.h"

// Should be read from game mode
GameBlitz::GameBlitz()
  : counter(0)
{ }

void GameBlitz::EndOfGame()
{
  SetState(END_TURN);
  GameMessages::GetInstance()->Add(_("And the winner is..."), white_color);

  counter = GameMode::GetInstance()->duration_exchange_player + 2;
  while (counter)
    MainLoop();
}

GameBlitz::time_iterator GameBlitz::GetCurrentTeam()
{
  auto cur = times.find(ActiveTeam().GetGroup());
  ASSERT(cur != times.end());
  return cur;
}

GameBlitz::time_iterator GameBlitz::KillGroup(GameBlitz::time_iterator cur)
{
  TeamGroup& group = TeamsList::GetInstance()->GetGroupList()[cur->first];
  for (auto & it : group) {
    FOR_EACH_LIVING_CHARACTER(it, character)
      character->Die(nullptr);
  }
  GameMessages::GetInstance()->Add(Format(_("Group %u was fragged down."), cur->first),
                                   TeamGroup::Colors[cur->first]);
  cur->second = 0;
  times.erase(cur);
  return times.end();
}

bool GameBlitz::Run()
{
  // Make sure map is empty
  times.clear();
  const TeamsList::GroupList& glist = TeamsList::GetInstance()->GetGroupList();
  for (const auto & git : glist)
    times[git.first] = GameMode::GetInstance()->duration_turn;

  counter = 0;
  return Game::Run();
}

void GameBlitz::RefreshClock()
{
  GameTime * global_time = GameTime::GetInstance();

  if (1000 < global_time->Read() - last_clock_update) {
    last_clock_update = global_time->Read();

    if (counter) {
      counter--;
    } else {
      auto cur = GetCurrentTeam();

      uint duration = cur->second;

      switch (state) {

      case PLAYING:
        if (duration <= 1) {
          JukeBox::GetInstance()->Play("default", "end_turn");
          cur = KillGroup(cur);
          SetState(END_TURN);
        } else {
          duration--;

          if (duration == 12) {
            countdown_sample.Play("default", "countdown-end_turn");
          }

          if (duration > 10) {
            interface->UpdateTimer(duration, false, false);
          } else {
            interface->UpdateTimer(duration, true, false);
          }
        }
        break;

      case HAS_PLAYED:
        if (duration <= 1) {
          cur = KillGroup(cur);
        } else {
          duration--;
          interface->UpdateTimer(duration, false, false);
        }
        SetState(END_TURN);
        break;

      case END_TURN:
        if (IsAnythingMoving() && duration<2) {
          duration = 1;
          break;
        }

        if (IsGameFinished())
          break;

        if (give_objbox && GetWorld().IsOpen()) {
          NewBox();
          give_objbox = false;
          break;
        }
        else {
          SetState(PLAYING);
          break;
        }
      } // switch

      if (cur != times.end())
        cur->second = duration;
    }// if !counter
  }
}

uint GameBlitz::GetRemainingTime() const
{
  return times.find(ActiveTeam().GetGroup())->second;
}

// Beginning of a new turn
void GameBlitz::__SetState_PLAYING()
{
  MSG_DEBUG("game.statechange", "Playing");

  Wind::GetRef().ChooseRandomVal();

  SetCharacterChosen(false);

  // Prepare each character for a new turn
  FOR_ALL_LIVING_CHARACTERS(team, character)
    character->PrepareTurn();

  // Select the next team
  ASSERT (!IsGameFinished());
  GetTeamsList().NextTeam();

  // initialize counter
  interface->UpdateTimer(GetCurrentTeam()->second, false, true);
  interface->EnableDisplayTimer(true);
  last_clock_update = GameTime::GetInstance()->Read();

  give_objbox = true; //hack: make it so that no more than one objbox per turn
}

void GameBlitz::__SetState_HAS_PLAYED()
{
  MSG_DEBUG("game.statechange", "Has played, now can move");
  last_clock_update = GameTime::GetInstance()->Read();

  CharacterCursor::GetInstance()->Hide();
}

void GameBlitz::__SetState_END_TURN()
{
  MSG_DEBUG("game.statechange", "End of turn");
  ActiveTeam().AccessWeapon().SignalTurnEnd();
  ActiveTeam().AccessWeapon().Deselect();
  CharacterCursor::GetInstance()->Hide();
  last_clock_update = GameTime::GetInstance()->Read();
  // Ensure the clock sprite isn't NULL:
  interface->UpdateTimer(GameMode::GetInstance()->duration_exchange_player,
                                        false,
                                        true);

  // Applying Disease damage and Death mode.
  ApplyDiseaseDamage();
}

bool GameBlitz::IsGameFinished() const
{
  uint num = 0;

  for (auto time : times) {
    if (time.second) //!= 0 && it->first->NbAliveCharacter())
      num++;
  }

  // If more than one group with time left > 0 and alive character, game not finished
  return (num < 2);
}
