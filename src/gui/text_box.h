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
 * TextBox widget
 *****************************************************************************/

#ifndef TEXT_BOX_H
#define TEXT_BOX_H

#include "label.h"

// Forward declaration
struct SDL_keysym;

class TextBox : public Label
{
protected:
  uint max_nb_chars;
  std::string::size_type cursor_pos;
  virtual void BasicSetText(std::string const & new_txt);

public:
  TextBox(const std::string &label,
          uint width,
          Font::font_size_t fsize = Font::FONT_SMALL,
          Font::font_style_t fstyle = Font::FONT_BOLD);
  TextBox(std::shared_ptr<Profile> profile,
          const xmlNode * textBoxNode);
  ~TextBox() override { };

  bool LoadXMLConfiguration() override;

  void SetText(std::string const & new_txt);
  void SetMaxNbChars(uint nb_chars) { max_nb_chars = nb_chars; }

  // From widget
  bool SendKey(const SDL_keysym & key) override;
  void Draw(const Point2i & mousePosition) override;
  Widget *ClickUp(const Point2i &, uint) override;
};

class PasswordBox : public TextBox
{
  std::string clear_text;
  void BasicSetText(std::string const & new_txt) override;

public:
  PasswordBox(const std::string & label,
              uint max_width,
              Font::font_size_t fsize = Font::FONT_SMALL,
              Font::font_style_t fstyle = Font::FONT_BOLD)
    : TextBox("", max_width, fsize, fstyle)
  {
    BasicSetText(label);
  }
  PasswordBox(std::shared_ptr<Profile> profile, const xmlNode * passwordBoxNode)
    : TextBox(profile, passwordBoxNode) { }

  bool SendKey(const SDL_keysym & key) override;
  const std::string & GetPassword() const { return clear_text; };
};

#endif

