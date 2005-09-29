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
 * Arme bazooka : projette une roquette avec un angle et une force donn�e.
 *****************************************************************************/

#include "../weapon/auto_bazooka.h"
//-----------------------------------------------------------------------------
#include "../game/time.h"
#include "../game/config.h"
#include "../interface/mouse.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../interface/interface.h"
#include "../graphic/video.h"
#include "../tool/math_tools.h"
#include "../game/game_loop.h"
#include "../tool/i18n.h"
#include "../map/camera.h"
#include "../weapon/weapon_tools.h"
#include "../interface/game_msg.h"
#include "../object/objects_list.h"
#include "../game/game_mode.h"
#include "../map/wind.h"
//-----------------------------------------------------------------------------
namespace Wormux {

AutomaticBazooka auto_bazooka;

//Temps en seconde � partir duquel la roquette se dirige vers la cible
const uint TPS_AV_ATTIRANCE = 1;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

RoquetteTeteCherche::RoquetteTeteCherche() 
  : WeaponProjectile("roquette tete chercheuse")
{
  m_allow_negative_y = true;
  touche_ver_objet = true;
  m_attire = false;
}

//-----------------------------------------------------------------------------

void RoquetteTeteCherche::Tire (double force, 
				int x, int y,
				uint cible_x, uint cible_y)
{
  SetAirResistFactor(auto_bazooka.cfg().air_resist_factor);

  PrepareTir();

  // Fixe la position de d�part
  SetXY (x,y);

  //On choisit la cible pour la roquette:
  m_cible.x = cible_x;
  m_cible.y = cible_y;
  
  //La roquette n'est pas attir�e par la cible d�s le d�part:
  m_attire = false;
  
  double angle = ActiveTeam().crosshair.GetAngleRad();
  SetSpeed (force, angle);

  temps_debut_tir = Wormux::temps.Lit();
  angle_local=angle;
}

//-----------------------------------------------------------------------------

void RoquetteTeteCherche::Init()
{
  image = CL_Sprite("roquette", &graphisme.weapons);
  SetSize (image.get_width(), image.get_height());

  SetMass (auto_bazooka.cfg().mass);
  SetWindFactor(0.1);
  SetAirResistFactor(auto_bazooka.cfg().air_resist_factor);

  // Fixe le rectangle de test
  int dx = image.get_width()/2-1;
  int dy = image.get_height()/2-1;
  SetTestRect (dx, dx, dy, dy);
}

//-----------------------------------------------------------------------------

void RoquetteTeteCherche::Refresh()
{
  double angle, tmp ;

  if (!is_active) return;

  if (TestImpact()) { SignalCollision(); return; }

  if (!m_attire)
    {
      //La roquette tourne sur elle-m�me
      angle_local += M_PI / 8;
      if(angle_local > M_PI) angle_local = - M_PI;
      angle = angle_local;
      
      image.set_angle (angle *180/M_PI);
      
      //2 sec apr�s avoir �t� tir�e, la roquette se dirige vers la cible:
      tmp = Wormux::temps.Lit() - temps_debut_tir;
      if(tmp>1000 * TPS_AV_ATTIRANCE)
	{
	  m_attire = true;

	  SetSpeed(0,0);
	  angle = CalculeAngle (GetPos(), m_cible);
	  image.set_angle (angle *180/M_PI);
	  SetExternForce(200, angle);
	}
    }
}
//-----------------------------------------------------------------------------

void RoquetteTeteCherche::SignalCollision()
{ 
  if (IsGhost())
  {
    game_messages.Add (_("The automatic rocket has left the battlefield..."));
  }
  is_active = false; 
}

//-----------------------------------------------------------------------------

void RoquetteTeteCherche::Reset()
{}

//-----------------------------------------------------------------------------

// Choisit les coordonn�es de la cible 	 
void RoquetteTeteCherche::SetTarget (int x, int y) 	 
{ 	 
  m_cible.x = x; 	 
  m_cible.y = y; 	 
} 	 


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

AutomaticBazooka::AutomaticBazooka() : Weapon(WEAPON_AUTOMATIC_BAZOOKA, "automatic_bazooka")
{  
  m_name = _("Automatic bazooka");

  m_is_active = false;
  cible.choisie = false;
  extra_params = new ExplosiveWeaponConfig();
}

//-----------------------------------------------------------------------------

void AutomaticBazooka::Draw()
{
  Weapon::Draw();
  DrawTarget();
}

//-----------------------------------------------------------------------------

bool AutomaticBazooka::p_Shoot ()
{
  if (m_strength == max_strength)
  {
    m_strength = 0;
    ExplosionDirecte();
    return true;
  }

  // Initialise le roquette
  int x,y;
  PosXY (x,y);
  roquette.Tire (m_strength, x,y, cible.pos.x,cible.pos.y);
  lst_objets.AjouteObjet (&roquette,true);

#ifdef CL
  jukebox.PlayProfile(ActiveTeam().GetSoundProfile(), "fire");
#else
  jukebox.Play(ActiveTeam().GetSoundProfile(), "fire");
#endif

  return true;
}

//-----------------------------------------------------------------------------

// Le bazooka explose car il a �t� pouss� � bout !
void AutomaticBazooka::ExplosionDirecte()
{
  CL_Point pos = ActiveCharacter().GetCenter();
  AppliqueExplosion (pos, pos, impact, cfg(), NULL);
}

//-----------------------------------------------------------------------------

void AutomaticBazooka::Explosion()
{
  m_is_active = false;
  
  lst_objets.RetireObjet (&roquette);

  // On fait un trou ?
  if (roquette.IsGhost()) return;

  // Applique les degats et le souffle aux vers
  CL_Point pos = roquette.GetCenter();
  AppliqueExplosion (pos, pos, impact, cfg(), NULL);
}

//-----------------------------------------------------------------------------

void AutomaticBazooka::Refresh()
{
  if(cible.choisie)
    cible.image.draw (cible.pos.x,cible.pos.y);

  if (m_is_active)
  {
    if (!roquette.is_active) Explosion();
  } 
}

//-----------------------------------------------------------------------------

void AutomaticBazooka::p_Init()
{
  roquette.Init();
  impact = CL_Surface("bazooka_impact", &graphisme.weapons);
  cible.image = CL_Surface("baz_cible", &graphisme.weapons);
}

//-----------------------------------------------------------------------------

void AutomaticBazooka::p_Select()
{
  cible.choisie = false;
}

//-----------------------------------------------------------------------------

void AutomaticBazooka::ChooseTarget()
{
  cible.pos = mouse.GetPosMonde();
  DrawTarget();
  cible.choisie = true;
}

//-----------------------------------------------------------------------------

void AutomaticBazooka::DrawTarget()
{
  if(!cible.choisie) { return; }
  cible.image.draw (cible.pos.x - (cible.image.get_width() / 2),
  			cible.pos.y - (cible.image.get_height() / 2));
}

//-----------------------------------------------------------------------------

bool AutomaticBazooka::IsReady() const
{
  return (EnoughAmmo() && cible.choisie);  
}

//-----------------------------------------------------------------------------

ExplosiveWeaponConfig& AutomaticBazooka::cfg()
{ return static_cast<ExplosiveWeaponConfig&>(*extra_params); }

//-----------------------------------------------------------------------------

} // namespace Wormux
