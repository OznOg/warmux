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
 * It is a fake widget.
 *****************************************************************************/
#include <iostream>
#include <SDL_keyboard.h>
#include "graphic/video.h"
#include "gui/widget_list.h"
#include "gui/widget.h"
#include "interface/mouse.h"

WidgetList::WidgetList()
  : selected_widget(nullptr)
{
}

WidgetList::WidgetList(const Point2i &size)
  : Widget(size)
  , selected_widget(nullptr)
{
}

WidgetList::WidgetList(std::shared_ptr<Profile> profile, const xmlNode * widgetListNode)
  : Widget(profile, widgetListNode)
  , selected_widget(nullptr)
{
}

WidgetList::~WidgetList()
{
  // Do not use Clear/Empty methods, they might be implemented
  // for other purposes
  for (auto & w : widget_list)
    delete w;

  widget_list.clear();
}

void WidgetList::Clear()
{
  for (auto & w : widget_list)
    delete w;

  Empty();
}

void WidgetList::DelFirstWidget()
{
  delete widget_list.front();
  widget_list.pop_front();
}

void WidgetList::AddWidget(Widget* w)
{
  ASSERT(w!=nullptr);
  widget_list.push_back(w);
  w->SetContainer(this);
}

void WidgetList::RemoveWidget(Widget* w)
{
  ASSERT(w!=nullptr);
  widget_list.remove(w);
  w->SetContainer(nullptr);
  delete w;
}

void WidgetList::SetVisible(bool v)
{
  Widget::SetVisible(v);
  for (auto & w : widget_list)
    w->SetVisible(v);
}

void WidgetList::SetHighlighted(bool focus)
{
  Widget::SetHighlighted(focus);
  for (auto & w : widget_list)
    w->SetHighlighted(focus);
}

void WidgetList::SetFocusOn(Widget* widget, bool force_mouse_position)
{
  if (widget == selected_widget)
    return;

  // Previous selection ?
  if (selected_widget != nullptr) {
    selected_widget->SetFocus(false);
  }

  selected_widget = widget;

  if (selected_widget) {
    selected_widget->SetFocus(true);

    if (force_mouse_position &&
        !selected_widget->Contains(Mouse::GetInstance()->GetPosition())) {

      Mouse::GetInstance()->SetPosition(selected_widget->GetPosition() +
                                        selected_widget->GetSize()/2);
    }
  }
}

Widget* WidgetList::GetFirstWidget() const
{
  Widget *first = nullptr;

  MSG_DEBUG("widgetlist", "%p::GetFirstWidget()", this);

  for (auto it : widget_list) {
    if (it->IsWidgetBrowser()) {
      MSG_DBG_RTTI("widgetlist", "%s:%p is a widget browser!\n",
                   typeid(*it).name(), it);

      first = it->GetFirstWidget();
      if (first != nullptr)
        return first;
    } else {
      MSG_DBG_RTTI("widgetlist", "%s:%p is NOT a widget browser!\n",
                   typeid(*it).name(), it);

      return it;
    }
  }

  return nullptr;
}

Widget* WidgetList::GetLastWidget() const
{
  Widget *last = nullptr;

  for (auto it = widget_list.rbegin(); it != widget_list.rend(); it++) {
    if ((*it)->IsWidgetBrowser()) {
      last = (*it)->GetLastWidget();
      if (last)
        return last;
    } else {
      return (*it);
    }
  }

  return nullptr;
}

Widget* WidgetList::GetNextWidget(const Widget *w, bool loop) const
{
  Widget *r = nullptr;

  ASSERT(!w || !w->IsWidgetBrowser());

  MSG_DBG_RTTI("widgetlist", "%p::GetNextWidget(%s:%p)",
               this, typeid(w).name(), w);

  if (widget_list.size() == 0) {
    return nullptr;
  }

  if (w == nullptr) {
    r = GetFirstWidget();
    MSG_DBG_RTTI("widgetlist", "%p::GetNextWidget(%s:%p) ==> %s%p",
                 this, typeid(w).name(), w, typeid(r).name(), r);
    return r;
  }

  std::list<Widget*>::const_iterator it;
  for (it = widget_list.begin(); it != widget_list.end(); it++) {

    MSG_DBG_RTTI("widgetlist", "iterate on %s:%p", typeid(*it).name(), (*it));

    if (w == (*it)) {
      MSG_DBG_RTTI("widgetlist", "we have found %s:%p", typeid(*it).name(), (*it));

      it++;
      if (it != widget_list.end())
        r = (*it);
      else if (loop)
        r = GetFirstWidget();
      else
        r = (Widget*)w;
      break;
    }

    if ((*it)->IsWidgetBrowser()) {
      MSG_DBG_RTTI("widgetlist", "%s:%p is a widget browser!\n",
                   typeid(*it).name(), (*it));

      r = (*it)->GetNextWidget(w, false);

      if (r && r == w && it != widget_list.end()) {
        MSG_DBG_RTTI("widgetlist", "r == w %s:%p", typeid(r).name(), (r));
        it++;
        if (it != widget_list.end()) {
          r = (*it);
          MSG_DBG_RTTI("widgetlist", "r ==>  %s:%p", typeid(r).name(), (r));
          if (r->IsWidgetBrowser()) {
            r = r->GetFirstWidget();
          }
        } else if (loop) {
          r = GetFirstWidget();
        }
      }
      if (r)
        break;
    } else {
      MSG_DBG_RTTI("widgetlist", "%s:%p is NOT a widget browser!\n",
                   typeid(*it).name(), (*it));
    }
  }

  ASSERT(!r || !r->IsWidgetBrowser());

  MSG_DBG_RTTI("widgetlist", "%p::GetNextWidget(%s:%p) ==> %s%p",
               this, typeid(w).name(), w, typeid(r).name(), r);

  return r;
}

void WidgetList::SetFocusOnNextWidget()
{
  // No widget => exit
  if (widget_list.size() == 0) {
    selected_widget = nullptr;
    return;
  }

  MSG_DBG_RTTI("widgetlist", "before %s:%p",
               typeid(selected_widget).name(), selected_widget);

  Widget* w = GetNextWidget(selected_widget, true);
  SetFocusOn(w, true);
}

Widget* WidgetList::GetPreviousWidget(const Widget *w, bool loop) const
{
  Widget *r = nullptr;

  if (widget_list.size() == 0) {
    return nullptr;
  }

  if (w == nullptr) {
    r = GetLastWidget();
    return r;
  }

  for (auto it = widget_list.rbegin();
       it != widget_list.rend();
       it++) {
    if (w == (*it)) {
      it++;
      if (it != widget_list.rend())
        r = (*it);
      else if (loop)
        r = (*widget_list.rbegin());
      else
        r = nullptr;
      break;
    }
  }

  return r;
}

void WidgetList::SetFocusOnPreviousWidget()
{
  // No widget => exit
  if (widget_list.size() == 0) {
    selected_widget = nullptr;
    return;
  }

  Widget* w = GetPreviousWidget(selected_widget, true);
  SetFocusOn(w, true);
}

bool WidgetList::SendKey(const SDL_keysym &key)
{
  if (selected_widget != nullptr)
    return selected_widget->SendKey(key);

  return false;
}

bool WidgetList::Update(const Point2i& mousePosition,
                        const Point2i& lastMousePosition)
{
  Rectanglei clip;
  Rectanglei wlr = GetClip(clip);
  if (!wlr.GetSizeX() || !wlr.GetSizeY())
      return false;

  // Redraw the background
  bool updated = false;
  if (need_redrawing)
    RedrawBackground(wlr);

  for (std::list<Widget*>::const_iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    Rectanglei r((*w)->GetPosition(), (*w)->GetSize());
    r.Clip(wlr);

    if (r.GetSizeX() && r.GetSizeY()) {
      SwapWindowClip(r);
      updated |= (*w)->Update(mousePosition, lastMousePosition);
      SwapWindowClip(r);
    }
  }

  if (updated)
    RedrawForeground();

  // Restore initial clip rectangle
  UnsetClip(clip);
  need_redrawing = false;
  return updated;
}

void WidgetList::Draw(const Point2i &mousePosition)
{
  Rectanglei clip;
  Rectanglei wlr = GetClip(clip);
  if (!wlr.GetSizeX() || !wlr.GetSizeY())
      return;

  for (std::list<Widget*>::const_iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    Rectanglei r((*w)->GetPosition(), (*w)->GetSize());
    r.Clip(wlr);

    if (r.GetSizeX() && r.GetSizeY()) {
      Rectanglei wr = r;
      SwapWindowClip(r);
      (*w)->RedrawBackground(wr);
      (*w)->Draw(mousePosition);
      (*w)->RedrawForeground();
      SwapWindowClip(r);
    }
  }

  // Restore initial clip rectangle
  UnsetClip(clip);
}

Widget* WidgetList::ClickUp(const Point2i &mousePosition, uint button)
{
  for (auto & w : widget_list) {
    if (w->Contains(mousePosition)) {
      Widget* child = w->ClickUp(mousePosition,button);

      if (child)
        SetFocusOn(child);

      return child;
    }
  }

  return nullptr;
}

Widget* WidgetList::Click(const Point2i &mousePosition, uint button)
{
  for (auto & w : widget_list)
    if (w->Contains(mousePosition))
      w->Click(mousePosition,button);

  return nullptr;
}

void WidgetList::NeedRedrawing()
{
  need_redrawing = true;

  for (auto & w : widget_list)
    w->NeedRedrawing();
}

void WidgetList::Pack()
{
  for (auto & w : widget_list)
    w->Pack();
}

void WidgetList::SetSelfBackgroundColor(const Color &background_color)
{
  Widget::SetBackgroundColor(background_color);
}

void WidgetList::SetSelfHighlightBgColor(const Color &highlight_bg_color)
{
  Widget::SetHighlightBgColor(highlight_bg_color);
}

void WidgetList::SetBackgroundColor(const Color &background_color)
{
  Widget::SetBackgroundColor(background_color);
  for (auto & w : widget_list)
    w->SetBackgroundColor(background_color);
}

void WidgetList::SetHighlightBgColor(const Color &highlight_bg_color)
{
  Widget::SetHighlightBgColor(highlight_bg_color);
  for (auto & w : widget_list)
    w->SetHighlightBgColor(highlight_bg_color);
}
