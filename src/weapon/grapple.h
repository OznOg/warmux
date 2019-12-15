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
 * Grapple
 *****************************************************************************/

#ifndef GRAPPLE_H
#define GRAPPLE_H
//-----------------------------------------------------------------------------
#include "weapon.h"
#include <WARMUX_base.h>
#include <list>
//-----------------------------------------------------------------------------

struct GrappleConfig;

class Grapple : public Weapon
{
  private:
    struct rope_node_t
    {
      Point2i pos;
      Double angle;
    };

    uint last_mvt;
    Double last_broken_node_angle;

    // Rope launching data.
    bool attached;
    Sprite* m_hook_sprite;
    Sprite* m_node_sprite;

    SoundSample cable_sound;
    bool move_left_pressed;
    bool move_right_pressed;
    bool move_up_pressed;
    bool move_down_pressed;

  protected:
    void Refresh() override;
    void p_Deselect() override { DetachRope(); };
    bool p_Shoot() override;

    void GoUp();
    void GoDown();
    void GoLeft();
    void GoRight();
    void StopUp();
    void StopDown();
    void StopLeft();
    void StopRight();

    bool WillBeAttached();
    bool TryAttachRope();
    bool TryAddNode();
    void TryRemoveNodes();

  public:
    enum {
      ATTACH_ROPE,
      ATTACH_NODE,
      DETACH_NODE,
      SET_ROPE_SIZE,
      UPDATE_PLAYER_POSITION
    } grapple_movement_t;

    std::list<rope_node_t> rope_nodes;
    Point2i m_fixation_point;
    bool go_left, go_right;
    Double delta_len ;

    Grapple();
    ~Grapple() override;
    void Draw() override;
    void NotifyMove(bool collision) override;

    GrappleConfig& cfg();

    // Attaching and dettaching nodes rope
    // This is public because of network
    void AttachRope(const Point2i& contact_point);
    void DetachRope();

    void AttachNode(const Point2i& contact_point, Double angle);
    void DetachNode();
    void SetRopeSize(Double length) const;

    void UpdateTranslationStrings() override;
    std::string GetWeaponWinString(const char *TeamName, uint items_count) const override;

    void StartMovingLeft() override;
    void StopMovingLeft() override;

    void StartMovingRight() override;
    void StopMovingRight() override;

    void StartMovingUp() override;
    void StopMovingUp() override;

    void StartMovingDown() override;
    void StopMovingDown() override;

    void StartShooting() override;
    void StopShooting() override;

    bool IsPreventingLRMovement() override;
    bool IsPreventingWeaponAngleChanges() override;
    // Keys management
    void HandleKeyPressed_Up(bool /*slowly*/) override;
    void HandleKeyReleased_Up(bool /*slowly*/) override;

    void HandleKeyPressed_Down(bool /*slowly*/) override;
    void HandleKeyReleased_Down(bool /*slowly*/) override;

    void HandleKeyPressed_MoveRight(bool /*slowly*/) override;
    void HandleKeyReleased_MoveRight(bool /*slowly*/) override;

    void HandleKeyPressed_MoveLeft(bool /*slowly*/) override;
    void HandleKeyReleased_MoveLeft(bool /*slowly*/) override;

    void PrintDebugRope();
};

//-----------------------------------------------------------------------------
#endif /* GRAPPLE_H */
