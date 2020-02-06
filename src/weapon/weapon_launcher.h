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
 * WeaponLauncher: generic weapon to launch a projectile
 *****************************************************************************/

#ifndef WEAPON_LAUNCHER_H
#define WEAPON_LAUNCHER_H

#include "weapon.h"
#include "object/physical_obj.h"

class Sprite;
class WeaponLauncher;
struct ExplosiveWeaponConfig;

class WeaponProjectile : public PhysicalObj
{
  uint timeout_start;

protected:
  Sprite *image;
  bool explode_colliding_character; // before timeout.
  bool explode_with_timeout;
  bool explode_with_collision;
  bool can_drown;
  bool camera_follow_closely;

  ExplosiveWeaponConfig& cfg;

public:
  WeaponLauncher * launcher;
  int m_timeout_modifier;

  WeaponProjectile(const std::string &nom,
                   ExplosiveWeaponConfig& cfg,
                   WeaponLauncher * p_launcher);
  ~WeaponProjectile() override;

  void Draw() override;
  // Call by the object list class to refresh the weapon's state
  void Refresh() override;
  virtual void Shoot(Double strength);
  bool IsImmobile() const override;
  void SetEnergyDelta(int /*delta*/, Character * /*dealer*/) override {
      // Don't call Explosion here, we're already in an explosion
      m_energy = 0;
  }

  void IncrementTimeOut();
  void DecrementTimeOut();
  void SetTimeOut(int timeout);
  int GetTotalTimeout() const;
  void ResetTimeOut() { m_timeout_modifier = 0; };
  bool change_timeout_allowed() const;

protected:
  void StartTimeout();
  uint GetMSSinceTimeoutStart() const;
  void Collision();

  void SignalGroundCollision(const Point2d& speed_before, const Double& contactAngle) override;
  void SignalObjectCollision(const Point2d& my_speed_before,
                                     PhysicalObj * obj,
                                     const Point2d& obj_speed) override;
  void SignalOutOfMap() override;
  virtual void SignalTimeout();
  virtual void SignalExplosion();
  void SignalDrowning() override;
  void SignalGoingOutOfWater() override;
  void SignalGhostState (bool was_dead) override;

  virtual void ShootSound();
  virtual void Explosion();
  // Implement it in subclass to randomize fire
  virtual void RandomizeShoot(Double &/*angle*/,Double &/*strength*/) { };
  virtual void DoExplosion();
};

class WeaponBullet : public WeaponProjectile
{
public:
  WeaponBullet(const std::string &name,
               ExplosiveWeaponConfig& cfg,
               WeaponLauncher * p_launcher);
  ~WeaponBullet() override{};
  void Refresh() override;
  bool IsBullet() const override { return true; }
protected:
  void SignalGroundCollision(const Point2d& speed_before, const Double& contactAngle) override;
  void SignalObjectCollision(const Point2d& my_speed_before,
                                     PhysicalObj * obj,
                                     const Point2d& obj_speed) override;
  void SignalOutOfMap() override;
  void DoExplosion() override;
};


class WeaponLauncher : public Weapon
{
public:
  bool ignore_timeout_signal;
  bool ignore_collision_signal;
  bool ignore_explosion_signal;
  bool ignore_ghost_state_signal;
  bool ignore_drowning_signal;
  bool ignore_going_out_of_water_signal;
protected:
  std::unique_ptr<WeaponProjectile> projectile;
  uint nb_active_projectile;
  bool m_allow_change_timeout;
  int missed_shots;
  bool announce_missed_shots;
protected:
  bool p_Shoot() override;
  void p_Select() override;
  virtual std::unique_ptr<WeaponProjectile> GetProjectileInstance() = 0;
  virtual void ReloadLauncher();
private:
  void DirectExplosion();
  void SetTimeoutForAllPlayers(int timeout);
public:
  WeaponLauncher(Weapon_type type,
                 const std::string &id,
                 EmptyWeaponConfig * params,
                 bool drawable = true);

  // Methods to access data of the projectile
  int GetDamage() const;
  Double GetWindFactor() const { return projectile->GetWindFactor(); }
  Double GetMass() const { return projectile->GetMass(); }

  void Draw() override;

  std::string GetWeaponWinString(const char *TeamName, uint items_count) const override = 0;

  // Handle of projectile events
  // Signal the end of a projectile for any reason possible
  virtual void SignalEndOfProjectile() { DecActiveProjectile(); };
  // Signal that a projectile explosion
  virtual void SignalProjectileExplosion() { SignalEndOfProjectile(); };
  // Signal that a projectile fired by this weapon has hit something (ground, character etc)
  virtual void SignalProjectileCollision() { SignalEndOfProjectile(); };
  // Signal a projectile is drowning
  virtual void SignalProjectileDrowning() { SignalEndOfProjectile(); };
  // Signal a projectile is fishing out of water (supertux)
  virtual void SignalProjectileGoingOutOfWater() {};
  // Signal a ghost state
  virtual void SignalProjectileGhostState() { SignalEndOfProjectile(); };
  // Signal a projectile timeout (for exemple: grenade, disco grenade ... etc.)
  virtual void SignalProjectileTimeout() { SignalEndOfProjectile(); };

  void SetTimeout(int timeout) { projectile->SetTimeOut(timeout); }
  int GetTimeout() { return projectile->GetTotalTimeout(); }

  // Keep the total amount of active projectile
  void IncActiveProjectile() { ++nb_active_projectile; };
  void DecActiveProjectile() { --nb_active_projectile; };

  virtual void IncMissedShots();

  // Handle mouse events
  void HandleMouseWheelUp(bool) override { SetTimeoutForAllPlayers(GetTimeout() + 1); }
  void HandleMouseWheelDown(bool) override { SetTimeoutForAllPlayers(GetTimeout() - 1); }

  // Handle special keys
  void HandleKeyReleased_Num1() override { SetTimeoutForAllPlayers(1); }
  void HandleKeyReleased_Num2() override { SetTimeoutForAllPlayers(2); }
  void HandleKeyReleased_Num3() override { SetTimeoutForAllPlayers(3); }
  void HandleKeyReleased_Num4() override { SetTimeoutForAllPlayers(4); }
  void HandleKeyReleased_Num5() override { SetTimeoutForAllPlayers(5); }
  void HandleKeyReleased_Num6() override { SetTimeoutForAllPlayers(6); }
  void HandleKeyReleased_Num7() override { SetTimeoutForAllPlayers(7); }
  void HandleKeyReleased_Num8() override { SetTimeoutForAllPlayers(8); }
  void HandleKeyReleased_Num9() override { SetTimeoutForAllPlayers(9); }
  void HandleKeyReleased_Less() override { SetTimeoutForAllPlayers(GetTimeout() - 1); }
  void HandleKeyReleased_More() override { SetTimeoutForAllPlayers(GetTimeout() + 1); }

  ExplosiveWeaponConfig& cfg() const;

  // Implemeting a method that would otherwise have required RTTI
  void SetProjectileTimeOut(int timeout) override { projectile->SetTimeOut(timeout); }
};

#endif /* WEAPON_LAUNCHER_H */
