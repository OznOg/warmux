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
#include "graphic/sprite.h"
#include "interface/mouse.h"
#include "tool/xml_document.h"

#include <memory>

#ifdef WIN32
   // Protects against macro definition of LoadImage when this header is included last.
#  undef LoadImage
#endif

class Sprite;
class MouseCursor;

// FIXME move this into a xml_sprite.hpp header so that it doesn't mess up here
static inline std::unique_ptr<Sprite> LoadSprite(const xmlNode* elem_sprite, const std::string& resource_name,
                                const std::string& main_folder)
{
  const xmlNode* elem_image = XmlReader::GetMarker(elem_sprite, "image");

  if (!elem_image)
      Error("ResourceManager: can't load (sprite) resource " + resource_name);

  std::string image_filename;
  if (!XmlReader::ReadStringAttr(elem_image, "file", image_filename))
      Error("ResourceManager: can't load (sprite) resource " + resource_name);

  // TODO load more properties in xml : alpha, colorkey....
  //      By now force alpha and no colorkey

  bool alpha = true;
  std::unique_ptr<Sprite> sprite = nullptr;

  const xmlNode* elem_grid = XmlReader::GetMarker(elem_image, "grid");

  if (!elem_grid) {
      ASSERT(resource_name != "barrel");
      // No grid element, Load the Sprite like a normal image
      Surface surface = LoadImage(main_folder + image_filename, alpha);
      sprite = std::make_unique<Sprite>();
      sprite->Init(surface, surface.GetSize(), 1, 1);
  } else {
      Point2i frameSize, offset;
      int nb_frames_x = 1;
      int nb_frames_y = 1;
      std::string str;

      // size is required
      if (!XmlReader::ReadStringAttr(elem_grid, "size", str))
          Error("ResourceManager: can't load sprite resource \""+resource_name+"\", no attribute size");
      if (sscanf(str.c_str(), "%i,%i", &frameSize.x, &frameSize.y) != 2)
          Error("ResourceManager: can't load sprite resource \""+resource_name+"\", malformed size attribute " + str);

      //array is not required, default is 1,1
      if (XmlReader::ReadStringAttr(elem_grid, "array", str)) {
          if (sscanf(str.c_str(), "%i,%i", &nb_frames_x, &nb_frames_y) != 2)
              Error("ResourceManager: can't load (sprite) resource "+resource_name+"\", malformed array attribute " + str);
          if (nb_frames_x <= 0)
              nb_frames_x = 1;
          if (nb_frames_y <= 0)
              nb_frames_y = 1;
      }

      Surface surface = LoadImage(main_folder + image_filename, alpha);
      sprite = std::make_unique<Sprite>();
      sprite->Init(surface, frameSize, nb_frames_x, nb_frames_y);
  }

  ASSERT(sprite != nullptr);

  const xmlNode* elem = XmlReader::GetMarker(elem_sprite, "animation");
  if (elem != nullptr) {
      std::string str;
      // Set the frame speed
      if (XmlReader::ReadStringAttr(elem, "speed", str))
          sprite->SetFrameSpeed(atoi(str.c_str()));

      if (XmlReader::ReadStringAttr(elem, "loop_mode", str)) {
          bool loop_value;
          if (str2bool(str, loop_value))
              sprite->animation.SetLoopMode(loop_value);
          else if (str == "pingpong")
              sprite->animation.SetPingPongMode(true);
          else
              Error("ResourceManager: unrecognized xml option loop_mode=\"" +str+ "\" in resource " + resource_name);
      }

      if (XmlReader::ReadStringAttr(elem, "loop_wait", str)) {
          sprite->animation.SetLoopWait(atoi(str.c_str()));
      }

      if (XmlReader::ReadStringAttr(elem, "loop_wait_random", str)) {
          sprite->animation.SetLoopWaitRandom(atoi(str.c_str()));
      }
  }
  return sprite;
}

class Profile
{
public:
  const xmlNode* GetElement(const std::string& resource_type, const std::string& resource_name) const
  {
      const xmlNode* elem = doc->Access(doc->GetRoot(), resource_type, resource_name);

      if (!elem) {
          std::string r_name = resource_name;
          const xmlNode* cur_elem = doc->GetRoot();

          while((r_name.find("/") != r_name.npos) && (cur_elem != nullptr)) {
              cur_elem = doc->Access(cur_elem, "section", r_name.substr(0, r_name.find("/")));
              r_name = r_name.substr(r_name.find("/") + 1, r_name.length());
          }
          if (cur_elem)
              elem = doc->Access(cur_elem, resource_type, r_name);
      }
      return elem;
  }

  Profile(std::string path, std::string filename, std::unique_ptr<XmlReader> doc) :
           relative_path(path), filename(filename), doc(std::move(doc)) { }
  
  std::unique_ptr<Sprite> LoadSprite(const std::string& resource_name) const
  {
      const xmlNode* elem_sprite = GetElement("sprite", resource_name);
      if (!elem_sprite)
          Error("ResourceManager: can't find sprite resource \"" + resource_name + "\" in profile " + filename);

      return ::LoadSprite(elem_sprite, resource_name, relative_path);
  }

  XmlReader * GetXMLDocument(void) const { return this->doc.get(); }

  std::string relative_path;
  std::string filename;
  std::unique_ptr<XmlReader> doc; //TODO move to private
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
};

inline ResourceManager& GetResourceManager() { return ResourceManager::GetRef(); }

#define LOAD_RES_IMAGE(name) GetResourceManager().LoadImage(res, name)
#define LOAD_RES_SPRITE(name) res->LoadSprite(name)
#define LOAD_RES_COLOR(name) GetResourceManager().LoadColor(res, name)
#define LOAD_RES_POINT(name) GetResourceManager().LoadPoint2i(res, name)

#endif /* _RESOURCE_MANAGER_H */
