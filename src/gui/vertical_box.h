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
 * Vertical Box
 *****************************************************************************/

#ifndef GUI_VERTICAL_BOX_H
#define GUI_VERTICAL_BOX_H

#include "gui/box.h"

class VBox : public Box
{
protected:
  bool force_widget_size;

public:
  VBox(uint width, bool draw_border = true, bool shadowed = true,
       bool force_widget_size = true);
  VBox(std::shared_ptr<Profile> profile,
       const xmlNode * verticalBoxNode);
  ~VBox() override { }

  void Pack() override;
  bool LoadXMLConfiguration(void) override;
};

#endif

