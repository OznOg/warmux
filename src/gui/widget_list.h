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
 * Widget list : store all widgets displayed on one screen
 * It is a fake widget
 *****************************************************************************/
#ifndef WIDGET_LIST_H
#define WIDGET_LIST_H

#include <list>
#include "widget.h"
#include "container.h"

class WidgetList : public Widget
{
public:
  typedef std::list<Widget*>::iterator wit;
  typedef std::list<Widget*>::const_iterator cwit;
  typedef std::list<Widget*>::reverse_iterator rwit;
  typedef std::list<Widget*>::const_reverse_iterator crwit;

private:
  Widget* selected_widget;

protected:
  std::list<Widget*> widget_list;
  virtual void DelFirstWidget(); // usefull only for message_box

public:
  WidgetList();
  WidgetList(const Point2i &size);
  WidgetList(std::shared_ptr<Profile> profile,
             const xmlNode * widgetListNode);
  ~WidgetList() override;

  // Highlight and background
  void SetVisible(bool _visible) override;
  void SetHighlighted(bool focus) override;
  void SetBackgroundColor(const Color &background_color) override;
  virtual void SetHighlightBgColor(const Color &highlight_bg_color);
  virtual void SetSelfBackgroundColor(const Color &background_color);
  virtual void SetSelfHighlightBgColor(const Color &highlight_bg_color);

  bool Update(const Point2i &mousePosition,
                      const Point2i &lastMousePosition) override;
  void Draw(const Point2i &mousePosition) override;
  // set need_redrawing to true for all sub widgets;
  void NeedRedrawing() override;

  // methods specialized from Widget to manage the list of widgets
  bool SendKey(const SDL_keysym &key) override;
  Widget* Click(const Point2i &mousePosition, uint button) override;
  Widget* ClickUp(const Point2i &mousePosition, uint button) override;

  // to add a widget
  virtual void AddWidget(Widget* widget);
  virtual void RemoveWidget(Widget* w);
  virtual size_t WidgetCount() const { return widget_list.size(); }
  virtual void Empty() { widget_list.clear(); }
  virtual void Clear();

  // Navigate between widget with keyboard
  virtual void SetFocusOnNextWidget();
  virtual void SetFocusOnPreviousWidget();
  Widget * GetCurrentKeyboardSelectedWidget() const { return selected_widget; };

  // to implement WidgetBrowser
  Widget* GetFirstWidget() const override;
  Widget* GetLastWidget() const override;
  Widget* GetNextWidget(const Widget *w, bool loop) const override;
  Widget* GetPreviousWidget(const Widget *w, bool loop) const override;
  bool IsWidgetBrowser() const override { return true; };

  virtual void SetFocusOn(Widget* widget, bool force_mouse_position = false);
  void Pack() override;
};

#endif // WIDGET_LIST_H
