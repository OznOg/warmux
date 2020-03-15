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
 *  Constants.
 *****************************************************************************/

#include "constant.h"

const std::string Constants::WARMUX_VERSION = PACKAGE_VERSION;

const std::string Constants::ENV_DATADIR = "WARMUX_DATADIR";
const std::string Constants::ENV_LOCALEDIR = "WARMUX_LOCALEDIR";
const std::string Constants::ENV_FONT_PATH = "WARMUX_FONT_PATH";

const std::string Constants::WEB_SITE = "www.wormux.org";
const std::string Constants::EMAIL = "warmux-dev .AT. gna .DOT. org";

// Size min/max of the map (pixels)
const Point2i Constants::MAP_MIN_SIZE = Point2i(100, 200);
const int Constants::MAP_MAX_SIZE = 6000*6000;

Constants::Constants()
{
  AUTHORS.emplace_back("Lawrence AZZOUG");
  AUTHORS.emplace_back("Frédéric BERTOLUS");
  AUTHORS.emplace_back("Anthony CARRÉ");
  AUTHORS.emplace_back("Laurent DEFERT SIMONNEAU");
  AUTHORS.emplace_back("Jean-Christophe DUBERGA");
  AUTHORS.emplace_back("Matthieu FERTRÉ");
  AUTHORS.emplace_back("Christophe GISQUET");
  AUTHORS.emplace_back("Sebastien GONZALVE");
  AUTHORS.emplace_back("Reiner HERRMANN");
  AUTHORS.emplace_back("Florian KÖBERLE");
  AUTHORS.emplace_back("Renaud LOTTIAUX");
  AUTHORS.emplace_back("Yannig PERRÉ");
  AUTHORS.emplace_back("Olivie SERRES");
  AUTHORS.emplace_back("Victor STINNER");
  AUTHORS.emplace_back("Mikko VARTIAINEN");
}
