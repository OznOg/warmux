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
 * Contains AICommand classes which can be used to let the AI send actions.
 *****************************************************************************/

#ifndef AI_COMMAND_H
#define AI_COMMAND_H

#include <WARMUX_types.h>
#include <queue>
#include "weapon/weapon.h"

class AICommand
{
public:
  /**
   * Tries to execute the command or tries to continue the execution of the command.
   * If the command requires an increment in game time then this method returns false.
   * In such a case this method should be called again when the game time has been increased.
   * A return value of true indicates that the command has been executed and that the object should not be used further.
   */
  virtual bool Execute() = 0;
  virtual ~AICommand() {};
};

class DoNothingForeverCommand : public AICommand
{
public:
  bool Execute() override { return false; };
};

/**
 * Increases the weapon angle until it's higher or equal then a specifc value.
 * Does nothing if the specified angle is larger then the current angle.
 */
class IncreaseAngleCommand : public AICommand
{
 const float target_angle;
  const bool slowly;
  bool is_increasing;
public:
  IncreaseAngleCommand(float target_angle, bool slowly);
  bool Execute() override;
};


/**
 * Decreases the weapon angle until it's higher or equal then a specifc value.
 * Does nothing if the specified angle is smaller then the current angle.
 */
class DecreaseAngleCommand : public AICommand
{
  const float target_angle;
  const bool slowly;
  bool is_decreasing;
public:
  DecreaseAngleCommand(float target_angle, bool slowly);
  bool Execute() override;
};

class CommandList : public AICommand
{
  std::queue<AICommand *> commands;
public:
  CommandList() {};
  ~CommandList() override;
  bool Execute() override;
  void Add(AICommand * command) { commands.push(command); }
  int Size() { return commands.size(); }
};

class SetWeaponAngleCommand : public AICommand
{
  CommandList commands;
  const float target_angle;
public:
  SetWeaponAngleCommand(float angle);
  bool Execute() override;
};

class StartShootingCommand : public AICommand
{
public:
  bool Execute() override;
};

class StopShootingCommand : public AICommand
{
public:
  bool Execute() override;
};

class StartMovingCommand : public AICommand
{
  LRDirection direction;
  bool slowly;
public:
  StartMovingCommand(LRDirection direction, bool slowly);
  bool Execute() override;
};

class StopMovingCommand : public AICommand
{
  LRDirection direction;
  bool slowly;
public:
  StopMovingCommand(LRDirection direction, bool slowly);
  bool Execute() override;
};

class SetDirectionCommand : public AICommand
{
  LRDirection direction;
  bool turning;
  CommandList * commands;
public:
  SetDirectionCommand(LRDirection direction);
  ~SetDirectionCommand() override;
  bool Execute() override;
};

/*unused?
class LoadAndShootCommand : public AICommand
{
  private:
    const float target_strength;
    bool is_shooting;
  public:
    LoadAndShootCommand(float strength);
    virtual bool Execute();
};*/

class DoNothingCommand : public AICommand
{
  const uint duration;
  uint start_time;
public:
  DoNothingCommand(const uint duration_in_ms);
  bool Execute() override;
};

class SelectWeaponCommand : public AICommand
{
  Weapon::Weapon_type weapon;
public:
  SelectWeaponCommand(Weapon::Weapon_type weapon);
  bool Execute() override;
};

class SelectCharacterCommand : public AICommand
{
  const Character * character;
public:
  SelectCharacterCommand(const Character * character);
  bool Execute() override;
};

class WaitForStrengthCommand : public AICommand
{
  float target_strength;
public:
  WaitForStrengthCommand(float target_strength);
  bool Execute() override;
};

class SetTimeoutCommand : public AICommand
{
  int timeout;
public:
  SetTimeoutCommand(int timeout);
  bool Execute() override;
};

#endif
