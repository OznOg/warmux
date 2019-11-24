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
 *                   Load directly or from refernece in xml resource profile
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de)
 *             Initial version
 *
 * TODO:       Keep reference to resources, better exceptions
 *****************************************************************************/

#ifndef _RESOURCE_MANAGER_H
#define _RESOURCE_MANAGER_H

#include <string>
#include <map>
#include <WARMUX_base.h>
#include <WARMUX_singleton.h>
#include "graphic/surface.h"
#include "interface/mouse.h"
#include "tool/xml_document.h"

#include <memory>

#ifdef WIN32
   // Protects against macro definition of LoadImage when this header is included last.
#  undef LoadImage
#endif

class Sprite;
class MouseCursor;

class Profile
{
protected:
  int ref_count = 1;
  std::string name;

public:
  Profile(const std::string& name, std::unique_ptr<XmlReader> &&doc) : name(name), doc(std::move(doc)) { }
  std::unique_ptr<XmlReader> doc; //TODO move to private
  std::string filename;
  std::string relative_path;

  XmlReader * GetXMLDocument(void) const { return this->doc.get(); }
};

class ResourceManager : public Singleton<ResourceManager>
{
  ResourceManager();
  ~ResourceManager() override;
  friend class Singleton<ResourceManager>;
  typedef std::map<std::string, std::weak_ptr<Profile>> ProfileMap;
  static ProfileMap profiles;
  std::string base_path;

public:
  void SetDataPath(const std::string& path) { base_path = path; }
  Surface LoadImage(const std::string& ressource_str, bool alpha = false,
                    bool set_colorkey = false, Uint32 colorkey = 0) const;

  std::shared_ptr<Profile> LoadXMLProfile(const std::string& xml_filename, bool is_absolute_path) const;

  MouseCursor LoadMouseCursor(const std::shared_ptr<Profile> profile, const std::string& resource_name, Mouse::pointer_t pointer_id) const;
  Color LoadColor(const std::shared_ptr<Profile> profile, const std::string& resource_name) const;
  int LoadInt(const std::shared_ptr<Profile> profile, const std::string& resource_name) const;
  Double LoadDouble(const std::shared_ptr<Profile> profile, const std::string& resource_name) const;
  Point2i LoadPoint2i(const std::shared_ptr<Profile> profile, const std::string& resource_name) const;
  Point2d LoadPoint2d(const std::shared_ptr<Profile> profile, const std::string& resource_name) const;
  std::string LoadImageFilename(const std::shared_ptr<Profile> profile, const std::string& resource_name) const;
  Surface LoadImage(const std::shared_ptr<Profile> profile, const std::string& resource_name, bool alpha = true) const;

  Sprite *LoadSprite(const std::shared_ptr<Profile> profile, const std::string& resource_name) const;

  // the following method is usefull if you have direct access to the xml file
  Sprite *LoadSprite(const xmlNode* sprite_elem, const std::string& resource_name, const std::string& main_folder) const;

  const xmlNode*  GetElement(const std::shared_ptr<Profile> profile, const std::string& ressource_type,
                             const std::string& ressource_name) const;
};

inline ResourceManager& GetResourceManager() { return ResourceManager::GetRef(); }

#define LOAD_RES_IMAGE(name) GetResourceManager().LoadImage(res, name)
#define LOAD_RES_SPRITE(name) GetResourceManager().LoadSprite(res, name)
#define LOAD_RES_COLOR(name) GetResourceManager().LoadColor(res, name)
#define LOAD_RES_POINT(name) GetResourceManager().LoadPoint2i(res, name)

#endif /* _RESOURCE_MANAGER_H */
