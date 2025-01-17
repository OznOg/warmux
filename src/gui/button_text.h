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
 * Button with text.
 *****************************************************************************/

#ifndef BUTTON_TEXT_H
#define BUTTON_TEXT_H

#include "button.h"
#include "graphic/font.h"

class Text;

class ButtonText : public Button
{
Text * text;

public:
  ButtonText(const std::shared_ptr<Profile> res_profile,
             const std::string & resource_id,
             const std::string & new_text,
             Font::font_size_t font_size,
             Font::font_style_t font_style);
  ~ButtonText() override;

  void Draw(const Point2i & mousePosition) override;
};

#endif
