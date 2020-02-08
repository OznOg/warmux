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

