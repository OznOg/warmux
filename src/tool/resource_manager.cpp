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
 * Resource Manager: Load resources (images/sprites) suitable for SDL
 *                    Load directly or from refernece in xml resource profile
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de)
 *             Initial version
 *
 * TODO:       Keep reference to resources, better exceptions
 *****************************************************************************/

#include "tool/resource_manager.h"
#include <libxml/xmlreader.h>
#include <string>
#include <iostream>

#include "tool/xml_document.h"
#include "tool/string_tools.h"
#include "game/config.h"
#include "graphic/sprite.h"
#include "graphic/polygon_generator.h"
#include "interface/mouse_cursor.h"

ResourceManager::ProfileMap ResourceManager::profiles;

ResourceManager::ResourceManager() : base_path("")
{
}

ResourceManager::~ResourceManager()
{
  xmlCleanupParser();
}

Double ResourceManager::LoadDouble(const std::shared_ptr<Profile> profile, const std::string& resource_name) const
{
  Double tmp = ZERO;
  const xmlNode* elem = profile->GetElement("Double", resource_name);
  if (!elem)
    Error("ResourceManager: can't find Double resource \""+resource_name+"\" in profile "+profile->filename);
  if (!profile->doc->ReadDoubleAttr(elem, "value", tmp))
    Error("ResourceManager: Double resource \""+resource_name+"\" has no value field in profile "+profile->filename);
  return tmp;
}

Color ResourceManager::LoadColor(const std::shared_ptr<Profile> profile, const std::string& resource_name) const
{
  const xmlNode* elem = profile->GetElement("color", resource_name);
  if (!elem)
    Error("ResourceManager: can't find color resource \""+resource_name+"\" in profile "+profile->filename);

  uint chanel_color[4];
  std::string tmp[4] = { "r", "g", "b", "a" };
  for (int i = 0; i < 4; i++) {
    if (!profile->doc->ReadUintAttr(elem, tmp[i], chanel_color[i]))
      Error("ResourceManager: color resource \""+resource_name+"\" has no "+tmp[i]+" field in profile "+profile->filename);
  }
  return Color(chanel_color[0], chanel_color[1], chanel_color[2], chanel_color[3]);
}

Point2i ResourceManager::LoadPoint2i(const std::shared_ptr<Profile> profile, const std::string& resource_name) const
{
  const xmlNode* elem = profile->GetElement("point", resource_name);
  if (!elem)
    Error("ResourceManager: can't find point resource \""+resource_name+"\" in profile "+profile->filename);

  uint point[2];
  std::string tmp[2] = { "x", "y" };
  for (int i = 0; i < 2; i++) {
    if (!profile->doc->ReadUintAttr(elem, tmp[i], point[i]))
      Error("ResourceManager: point resource \""+resource_name+"\" has no "+tmp[i]+" field in profile "+profile->filename);
  }
  return Point2i(point[0], point[1]);
}

Point2d ResourceManager::LoadPoint2d(const std::shared_ptr<Profile> profile, const std::string& resource_name) const
{
  const xmlNode* elem = profile->GetElement("point", resource_name);
  if (!elem)
    Error("ResourceManager: can't find point resource \""+resource_name+"\" in profile "+profile->filename);

  Double point[2];
  std::string tmp[2] = { "x", "y" };
  for (int i = 0; i < 2; i++) {
    if (!profile->doc->ReadDoubleAttr(elem, tmp[i], point[i]))
      Error("ResourceManager: point resource \""+resource_name+"\" has no "+tmp[i]+" field in profile "+profile->filename);
  }
  return Point2d(point[0], point[1]);
}

MouseCursor ResourceManager::LoadMouseCursor(const std::shared_ptr<Profile> profile, const std::string& resource_name,
                                             Mouse::pointer_t _pointer_id) const
{
  const xmlNode* elem = profile->GetElement("mouse_cursor", resource_name);
  if (!elem)
    Error("ResourceManager: can't find mouse cursor resource \""+resource_name+"\" in profile "+profile->filename);
  std::string filename;
  if (!profile->doc->ReadStringAttr(elem, "file", filename))
    Error("ResourceManager: mouse cursor resource \""+resource_name+"\" has no file field in profile "+profile->filename);

  uint point[2];
  std::string tmp[2] = { "x", "y" };
  for (int i = 0; i < 2; i++) {
    if (!profile->doc->ReadUintAttr(elem, tmp[i], point[i]))
      Error("ResourceManager: mouse cursor resource \""+resource_name+"\" has no "+tmp[i]+" field in profile "+profile->filename);
  }
  Point2i pos(point[0], point[1]);

  MouseCursor mouse_cursor(_pointer_id, profile->relative_path+filename, pos);
  return mouse_cursor;
}

Surface ResourceManager::LoadImage(const std::string& filename,
                                   bool alpha, bool set_colorkey, Uint32 colorkey) const
{
  Surface surface(filename.c_str());

  if (set_colorkey)
    surface.SetColorKey(SDL_SRCCOLORKEY, colorkey);

  return (alpha) ? surface.DisplayFormatAlpha() : surface.DisplayFormat();
}

std::shared_ptr<Profile> ResourceManager::LoadXMLProfile(const std::string& xml_filename, bool is_absolute_path) const
{
  /* Nex test may create a empt week_ptr, but this is harmless as it will be
   * filled hereafter in this case. */
  if (auto sp = profiles[xml_filename].lock()) {
    MSG_DEBUG("xml.cached_load", "Returning cached %s\n", xml_filename.c_str());
    return sp;
  }

  MSG_DEBUG("xml.load", "Loading uncached %s\n", xml_filename.c_str());
  auto doc = std::make_unique<XmlReader>();
  std::string filename, path;
  if (!is_absolute_path) {
    path = base_path;
    filename = path + xml_filename;
  } else {
    ASSERT(xml_filename.rfind(PATH_SEPARATOR) != xml_filename.npos);
    path = xml_filename.substr(0, xml_filename.rfind(PATH_SEPARATOR)+1);
    filename = xml_filename;
  }

   // Load the XML
  if (!doc->Load(filename)) {
    // TODO raise an "can't load file" exception
    Error("ResourceManager: can't load profile "+filename);
    return nullptr;
  }

  auto profile = std::make_shared<Profile>(path, xml_filename, std::move(doc));
  profiles[xml_filename] = profile;
  return profile;
}

std::string ResourceManager::LoadImageFilename(const std::shared_ptr<Profile> profile, const std::string& resource_name) const
{
  const xmlNode* elem = profile->GetElement("surface", resource_name);
  if (!elem)
    Error("ResourceManager: can't find image resource \""+resource_name+"\" in profile "+profile->filename);

  std::string filename;
  if (!profile->doc->ReadStringAttr(elem, "file", filename))
    Error("ResourceManager: image resource \""+resource_name+"\" has no file field in profile "+profile->filename);

  return profile->relative_path+filename;
}

Surface ResourceManager::LoadImage(const std::shared_ptr<Profile> profile, const std::string& resource_name, bool alpha) const
{
  std::string    filename = LoadImageFilename(profile, resource_name);
  Surface        image    = LoadImage(filename, alpha);
  const xmlNode *elem     = profile->GetElement("surface", resource_name);
  std::string    str;

  if (XmlReader::ReadStringAttr(elem, "size", str)) {
    int x, y;
    Rectanglei source_rect(0,0, image.GetWidth(), image.GetHeight());

    if (sscanf(str.c_str(), "%i,%i", &x, &y) != 2)
      Error("ResourceManager: can't load image resource \""+resource_name+"\", malformed size attribute " + str);
    source_rect.SetSizeX(x);
    source_rect.SetSizeY(y);

    if (XmlReader::ReadStringAttr(elem, "pos", str)) {
      if (sscanf(str.c_str(), "%i,%i", &x, &y) != 2)
        Error("ResourceManager: can't load image resource \""+resource_name+"\", malformed position attribute " + str);

      source_rect.SetPositionX(x);
      source_rect.SetPositionY(y);
    }

    return image.Crop(source_rect);
  }
  else {
    return image;
  }

  // TODO load more properties in xml : alpha, colorkey....
  //      By now force alpha and no colorkey
}

