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
 *  Refresh du vent
 *****************************************************************************/

#include "wind.h"
//----------------------------------------------------------------------------
#include "../include/action_handler.h"
#include "../tool/random.h"
#include "../map/map.h"
#include "../map/maps_list.h"
#include "../game/time.h"
#include "../tool/xml_document.h"
#include "../graphic/graphism.h"
#include "camera.h"
//----------------------------------------------------------------------------

const uint MAX_WIND_OBJECTS = 200;

const uint BARRE_LARG = 80;
const uint BARRE_HAUT = 10;
const double force = 5; // Force maximale du vent en m/(sec*sec)
const uint barre_speed = 20;

//----------------------------------------------------------------------------
namespace Wormux {
Wind wind;
//-----------------------------------------------------------------------------

WindParticle::WindParticle() : PhysicalObj("WindParticle", 0.0)
{
  m_type = objUNBREAKABLE;
  m_wind_factor = 1;
  m_air_resist_factor = 0.2;
}

//-----------------------------------------------------------------------------

void WindParticle::Init()
{
  double mass, wind_factor ;

  sprite = CL_Sprite("wind_particle", TerrainActif().res);
  sprite.set_frame ( RandomLong(0, sprite.get_frame_count()-1) );

  SetXY(RandomLong(0, monde.GetWidth()), RandomLong(0, monde.GetHeight()));

  //Mass = mass_mean + or - 25%
  mass = TerrainActif().wind.particle_mass;
  mass *= (1.0 + RandomLong(-100, 100)/400.0);
  SetMass (mass);
  SetSize (sprite.get_width(), sprite.get_height());
  wind_factor = TerrainActif().wind.particle_wind_factor ;
  wind_factor *= (1.0 + RandomLong(-100, 100)/400.0);  
  SetWindFactor(wind_factor);
  StartMoving();

  // Fixe le rectangle de test
  int dx = 0 ;
  int dy = 0 ;
  SetTestRect (dx, dx, dy, dy);

  m_allow_negative_y = true;
}

//-----------------------------------------------------------------------------

void WindParticle::Reset()
{ 
}

//-----------------------------------------------------------------------------

void WindParticle::Refresh()
{
  sprite.update();
  UpdatePosition();
  if (m_alive == GHOST)
    {
      int x = GetX();
      int y = GetY();

      if(x >= (int)monde.GetWidth())
	{
	  x = 1 - sprite.get_width() ;
	  y = RandomLong(0, monde.GetHeight()) ;
	}
      else
	{
	  if(x <= 0)
	    {
	      x = monde.GetWidth() - 1 ;
	      y = RandomLong(0, monde.GetHeight()) ;
	    }
	}

      if(y >= (int)monde.GetHeight())
	{
	  y = 1 - sprite.get_height() ;
	  x = RandomLong(0, monde.GetWidth()) ;
	}
      else
	{
	  if(y <= 0)
	    {
	      y = monde.GetHeight() - 1 ;
	      x = RandomLong(0, monde.GetWidth()) ;
	    }
	}

      m_alive = ALIVE;
      SetXY(x,y);
    }
}

//-----------------------------------------------------------------------------

void WindParticle::Draw()
{
  if(TerrainActif().wind.need_flip)
  {
    DoubleVector speed;
    GetSpeedXY(speed);
    float scale_x,scale_y;
    sprite.get_scale(scale_x,scale_y);
    if(speed.x<0 && scale_x>0
    || speed.x>0 && scale_x<0)
    {
      scale_x=-scale_x;
      sprite.set_scale(scale_x,scale_y);
    }
  }
  sprite.draw(GetX(),GetY());
}

//-----------------------------------------------------------------------------

void WindParticle::Resize(double size)
{
  size=0.5+size/2.0;
  sprite.set_scale(size,size);
  sprite.set_alpha(size);
}

//-----------------------------------------------------------------------------


Wind::Wind()
{
  m_val = m_nv_val = 0;
  barre.InitPos (10, 10, BARRE_LARG, BARRE_HAUT);
  barre.InitVal (0, -100, 100);
  barre.border_color = CL_Color::white;
  barre.background_color = CL_Color(255*6/10,255*6/10,255*6/10);
  barre.value_color = CL_Color(255*3/10,255*3/10,255);
  barre.AjouteMarqueur (100, CL_Color::white);
  barre.SetReferenceValue (true, 0);

  wind_particle_array = new WindParticle[MAX_WIND_OBJECTS];
}

//-----------------------------------------------------------------------------

Wind::~Wind()
{
  delete []wind_particle_array;
}

//-----------------------------------------------------------------------------

void Wind::Init()
{
}

//-----------------------------------------------------------------------------

void Wind::Reset()
{
  m_last_move = 0;
  m_last_part_mvt = 0;
  m_val = m_nv_val = 0;
  barre.Actu (m_val);

  for(uint i=0;i<TerrainActif().wind.nbr_sprite; i++)
  {
    wind_particle_array[i].Init();
    wind_particle_array[i].Resize((double)i/(double)TerrainActif().wind.nbr_sprite);
  }
}

//-----------------------------------------------------------------------------

double Wind::GetStrength() const
{
  return m_nv_val*force/100.0;
}

//-----------------------------------------------------------------------------

void Wind::ChooseRandomVal()
{
  int val = RandomLong(-100, 100);
  action_handler.NewAction (ActionInt(ACTION_WIND, val));
}

//-----------------------------------------------------------------------------

void Wind::SetVal(long val)
{
  m_nv_val = val;
}

//-----------------------------------------------------------------------------

void Wind::DrawParticles()
{
  //  TerrainActif().wind.nbr_sprite = 1 ;

  for(uint i=0;i<TerrainActif().wind.nbr_sprite; i++)
    wind_particle_array[i].Draw();
}

//-----------------------------------------------------------------------------

void Wind::Refresh()
{
  if(m_last_move + barre_speed < temps.Lit())
  {
    if(m_val>m_nv_val)
      --m_val;
    else
    if(m_val<m_nv_val)
      ++m_val;
    m_last_move = temps.Lit();
    barre.Actu(m_val); 
  }

  for(uint i=0;i<TerrainActif().wind.nbr_sprite; i++)
    {
      wind_particle_array[i].Refresh();
    }
}

//-----------------------------------------------------------------------------

void Wind::Draw()
{
  barre.Draw();
}

//-----------------------------------------------------------------------------
} //namespace Wormux
//-----------------------------------------------------------------------------
