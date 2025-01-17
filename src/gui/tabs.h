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
 * Tabs
 *****************************************************************************/

#ifndef GUI_TABS_H
#define GUI_TABS_H

#include <vector>
#include <WARMUX_base.h>
#include "graphic/font.h"
#include "gui/widget.h"

class Button;
class Box;
class Text;
class Tab;

class MultiTabs : public Widget
{
private:
  std::vector<Tab> tabs;

  uint max_visible_tabs;

  uint current_tab;
  uint first_tab;
  uint nb_visible_tabs;
  uint tab_header_width;
  uint tab_header_height;
  Font::font_size_t fsize;

  Button* prev_tab_bt;
  Button* next_tab_bt;

  void PrevTab();
  void NextTab();

  void DrawHeader(const Point2i &mousePosition) const;

public:
  MultiTabs(const Point2i& size, Font::font_size_t fsize = Font::FONT_MEDIUM);
  ~MultiTabs() override;

  void AddNewTab(const std::string& id, const std::string& title, Widget* w);
  const std::string& GetCurrentTabId() const;

  uint GetHeaderHeight() const { return tab_header_height; }

  void SelectTab(uint current);

  // from widget
  void NeedRedrawing() override;
  void Draw(const Point2i &mousePosition) override;
  bool Update(const Point2i &mousePosition,
                      const Point2i &lastMousePosition) override;
  void Pack() override;

  bool SendKey(const SDL_keysym&) override;
  Widget* Click(const Point2i &mousePosition, uint button) override;
  Widget* ClickUp(const Point2i &mousePosition, uint button) override;

  void SetMaxVisibleTabs(uint max) { max_visible_tabs = max; }
};

#endif // GUI_TABS_H
