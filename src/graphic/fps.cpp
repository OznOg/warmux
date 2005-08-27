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
 * Calculate frame per second.
 *****************************************************************************/

#include "fps.h"
//-----------------------------------------------------------------------------
#include "video.h"
#include "graphism.h"
#include <ClanLib/core.h>
#include <sstream>
#include <iomanip>
using namespace Wormux;
//-----------------------------------------------------------------------------
const uint NBR_VAL = 4; // nombre de valeurs utilis�es pour calculer la moyenne
//-----------------------------------------------------------------------------
ImageParSeconde image_par_seconde;
//-----------------------------------------------------------------------------
  
ImageParSeconde::ImageParSeconde()
{
  affiche = true;
  Reset();
}

//-----------------------------------------------------------------------------

void ImageParSeconde::Reset()
{
  moyenne = -1;
  nbr_img.clear();
  for (uint i=0; i<=NBR_VAL; ++i) nbr_img.push_back (0);
  temps_seconde = CL_System::get_time()+1000;
  nbr_val_valides = -1;
}

//-----------------------------------------------------------------------------

void ImageParSeconde::AjouteUneImage()
{
  ++nbr_img.front();
}

//-----------------------------------------------------------------------------

void ImageParSeconde::Refresh()
{
  uint nv_temps = CL_System::get_time();

  // Pas encore l'heure de recalculer : exit !
  if (nv_temps <= temps_seconde) return;

  // On d�cale !
  while (temps_seconde < nv_temps)
  {
    temps_seconde += 1000;
    nbr_img.pop_back();
    nbr_img.push_front(0);
    if (nbr_val_valides < (int)nbr_img.size()-1) nbr_val_valides++;
  }

  // Recalcule la moyenne
  if (0 < nbr_val_valides)
  {
    moyenne = 0;
    std::list<uint>::const_iterator it=nbr_img.begin();
    ++it;
    for (int i=1; i<=nbr_val_valides; ++i, ++it) moyenne += *it;
    moyenne /= nbr_val_valides;
  }
}

//-----------------------------------------------------------------------------

void ImageParSeconde::Draw()
{
  if (!affiche) return;
  if (moyenne < 0) return;

  std::ostringstream ss;

  ss << std::setprecision(3) << moyenne << " i/s";
  police_mix.WriteRight (video.GetWidth(), 0, ss.str());
}

//-----------------------------------------------------------------------------
