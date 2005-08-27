/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
 * Refresh des diff�rentes �quipes.
 *****************************************************************************/

#include "teams_list.h"
//-----------------------------------------------------------------------------
#include "../include/action_handler.h"
#include "../network/network.h"
#include "../game/config.h"
#include "../tool/file_tools.h"
#include "../tool/i18n.h"
#include "team_energy.h"
#include <algorithm>

#ifndef WIN32
#include <dirent.h>
#include <sys/stat.h>
#endif
//-----------------------------------------------------------------------------
TeamsList teams_list;
//-----------------------------------------------------------------------------

TeamsList::TeamsList()
{}

//-----------------------------------------------------------------------------

void TeamsList::NextTeam (bool debut_jeu)
{
  // Fin du tour pour l'�quipe active
  if (debut_jeu) return;
  if (network.is_client()) return;
  ActiveTeam().FinTour();
  
  // Passe � l'�quipe suivante
  std::vector<Team*>::iterator it=m_equipe_active;
  do
    {
      ++it;
      if (it == list.end()) it = list.begin();
    } while ((**it).NbAliveCharacter() == 0);
  action_handler.NewAction(ActionString(ACTION_CHANGE_TEAM, (**it).GetId()));
}

//-----------------------------------------------------------------------------

Team& TeamsList::ActiveTeam()
{
  assert (m_equipe_active != list.end());
  return **m_equipe_active;
}

//-----------------------------------------------------------------------------

void TeamsList::LoadOneTeam(const std::string &dir, const std::string &team)
{
  // Skip '.', '..' and hidden files
  if (team[0] == '.') return;
  
#ifndef WIN32
  // Is it a directory ?
  struct stat stat_file;
  std::string filename = dir+team;
  if (stat(filename.c_str(), &stat_file) != 0) return;
  if (!S_ISDIR(stat_file.st_mode)) return;
#endif
	
  // Add a new empty team
  Team nv_equipe;
  full_list.push_back(nv_equipe);

  // Try to load team 
  bool ok = full_list.back().Init (dir, team);

  // If fails, remove the team
  if (!ok)
  {
    full_list.pop_back();
	return;
  }
  std::cout << ((1<full_list.size())?", ":" ") << team;
  std::cout.flush();
}

//-----------------------------------------------------------------------------

void TeamsList::LoadList()
{  
  list.clear() ;
   
  std::cout << "o " << _("Load teams:");
  
  // Load Wormux teams
#ifndef WIN32
  struct dirent *file;

  std::string dirname = Wormux::config.data_dir+"team/";
  DIR *dir = opendir(dirname.c_str());
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL)  LoadOneTeam (dirname, file->d_name);
    closedir (dir);
  } else {
	Erreur (Format(_("Cannot open teams directory (%s)!"), dirname.c_str()));
  }
#else
  std::string dirname = Wormux::config.data_dir+"team\\";
  std::string pattern = dirname + "*.*";
  WIN32_FIND_DATA file;
  HANDLE file_search;
  file_search=FindFirstFile(pattern.c_str(),&file);
  if(file_search != INVALID_HANDLE_VALUE)
  {
    while (FindNextFile(file_search,&file))
	{
	  if(file.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	    LoadOneTeam(dirname,file.cFileName);
	}
  } else {
	Erreur (Format(_("Cannot open teams directory (%s)!"), dirname.c_str()));
  }
  FindClose(file_search);
#endif

  // Load personal teams
#ifndef WIN32
  dirname = Wormux::config.GetWormuxPersonalDir()+"team/";
  dir = opendir(dirname.c_str());
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL) LoadOneTeam (dirname, file->d_name);
    closedir (dir);
  }
#endif

  teams_list.full_list.sort(compareTeams);

  // On a au moins deux �quipes ?
  if (full_list.size() < 2)
    Erreur(_("You need at least two valid teams !"));

  // S�lection bidon
  std::list<uint> nv_selection;
  nv_selection.push_back (0);
  nv_selection.push_back (1);
  ChangeSelection (nv_selection);

  std::cout << std::endl << std::endl;
}

//-----------------------------------------------------------------------------

void TeamsList::Reset()
{
  m_equipe_active = list.begin();

  // Commence par d�sactiver tous les vers
  iterator it=list.begin(), fin=list.end();
  for (; it != fin; ++it) 
  {
	Team &team = **it;
    Team::iterator ver=team.begin(), dernier_ver=team.end();
    for (; ver != dernier_ver; ++ver) (*ver).Desactive();
  } 

  // Reset de toutes les �quipes
  for (it=list.begin(); it != fin; ++it) (**it).Reset();
}

//-----------------------------------------------------------------------------

Team *TeamsList::FindById (const std::string &id, int &pos)
{
  full_iterator it=full_list.begin(), fin=full_list.end();
  int i=0;
  for (; it != fin; ++it, ++i) 
  {
    if (it -> GetId() == id) 
    {
      pos = i;
      return &(*it);
    }
  }
  pos = -1;
  return NULL;
}

//-----------------------------------------------------------------------------

Team *TeamsList::FindByIndex (uint index)
{
  full_iterator it=full_list.begin(), fin=full_list.end();
  uint i=0;
  for (; it != fin; ++it, ++i) 
  {
    if (i == index) return &(*it);
  }
  assert (false);
  return NULL;
}

//-----------------------------------------------------------------------------

void TeamsList::InitList (const std::list<std::string> &liste_nom)
{
  Clear();
  std::list<std::string>::const_iterator it=liste_nom.begin(), fin=liste_nom.end();
  for (; it != fin; ++it) AddTeam (*it, false);
  m_equipe_active = list.begin();
}

//-----------------------------------------------------------------------------

void TeamsList::InitEnergy ()
{
  //On cherche l'�quipe avec le plus d'�nergie pour fixer le niveau max
  //(arrive dans le cas d'�quipe n'ayant pas le m�me nombre de vers)
  iterator it=list.begin(), fin=list.end();
  uint max = 0;
  for (; it != fin; ++it)
  {
    if( (**it).LitEnergie() > max)
      max = (**it).LitEnergie();
  }

  //Initialisation de la barre d'�nergie de chaque �quipe
  it=list.begin();
  uint i = 0;
  for (; it != fin; ++it)
  {
    (**it).InitEnergy (max);
    ++i;
  }

  //Calcul du classement initial
  it=list.begin();
  for (; it != fin; ++it)
  {
    uint classement = 0;
    iterator it2=list.begin();
    for (; it2 != fin; ++it2)
    {
      if((it != it2)
      && (**it2).LitEnergie() > (**it).LitEnergie() )
        ++classement;
    }
    (**it).energie.classement_tmp = classement;
  }
  it=list.begin();
  for (; it != fin; ++it)
  {
    uint classement = (**it).energie.classement_tmp;
    iterator it2=list.begin();
    for (it2 = it; it2 != fin; ++it2)
    {
      if((it != it2)
      && (**it2).LitEnergie() == (**it).LitEnergie() )
        ++classement;
    }
    (**it).energie.FixeClassement(classement);
  }
}

//-----------------------------------------------------------------------------

void TeamsList::RefreshEnergy ()
{
  //Dans l'ordre des priorit�s :
  // - Terminer l'op�ration en cours
  // - On change la valeur de l'�nergie
  // - On change le classement
  // - On pr�pare les jauges � l'�venement suivant
  
  iterator it=list.begin(), fin=list.end();
  uint status;

  bool en_attente = true; // Toute les jauges sont en attente

  for (; it != fin; ++it) {
    if( (**it).energie.status != EnergieStatusAttend)
    {
      en_attente = false;
      break;
    }
  }

  //Une des jauge �x�cute un ordre?
  if(!en_attente)
  {
    status = EnergieStatusOK;

    //Une des jauges change de valeur?
    for (it=list.begin(); it != fin; ++it) {
      if( (**it).energie.status == EnergieStatusValeurChange) {
        status = EnergieStatusValeurChange;
        break;
      }
    }
  
    //Une des jauges change de classement?
    for (it=list.begin(); it != fin; ++it) {
      if( (**it).energie.status == EnergieStatusClassementChange
      && ((**it).energie.EstEnMouvement() || status == EnergieStatusOK)) {
        status = EnergieStatusClassementChange;
        break;
      }
    }
  }
  else {
    //Les jauges sont toutes en attente
    //->on les met OK pour un nouvel ordre
    status = EnergieStatusOK;
  }

  // On recopie l'ordre a donner aux jauges
  if(status != EnergieStatusOK || en_attente)
  {
    it=list.begin();
    for (; it != fin; ++it) {
      (**it).energie.status = status;
    }
  }

  // Actualisation des valeurs (pas d'actualisation de l'affichage)
  for (it=list.begin(); it != fin; ++it)
  {
    (**it).ActualiseBarreEnergie();
    RefreshSort();
  }
}
//-----------------------------------------------------------------------------

void TeamsList::RefreshSort ()
{
  iterator it=list.begin(), fin=list.end();
  uint classement;
  
  //Cherche le classement sans tenir comte des �galit�s
  it=list.begin();
  for (; it != fin; ++it)
  {
    classement = 0;
    iterator it2=list.begin();
    for (; it2 != fin; ++it2)
    {
      if((it != it2)
      && (**it2).LitEnergie() > (**it).LitEnergie() )
        ++classement;
    }
    (**it).energie.classement_tmp = classement;
  }

  //R�glage des �galit�s
  it=list.begin();
  for (; it != fin; ++it)
  {
    classement = (**it).energie.classement_tmp;
    iterator it2=list.begin();
    for (it2 = it; it2 != fin; ++it2)
    {
      if((it != it2)
      && (**it2).LitEnergie() == (**it).LitEnergie() )
        ++classement;
    }
    (**it).energie.NouveauClassement(classement);
  }
}

//-----------------------------------------------------------------------------

void TeamsList::ChangeSelection (const std::list<uint>& nv_selection)
{
  selection = nv_selection;

  selection_iterator it=selection.begin(), fin=selection.end();
  list.clear();
  for (; it != fin; ++it) list.push_back (FindByIndex(*it));
  m_equipe_active = list.begin();
}

//-----------------------------------------------------------------------------

bool TeamsList::IsSelected (uint index)
{
  selection_iterator pos = std::find (selection.begin(), selection.end(), index);
  return pos != selection.end();
}

void TeamsList::Clear()
{
  selection.clear();
  list.clear();
}
  
//-----------------------------------------------------------------------------

void TeamsList::AddTeam (const std::string &id, bool generate_error)
{
    int pos;
    Team *equipe = FindById (id, pos);
    if (equipe != NULL) {
      selection.push_back (pos);
      list.push_back (equipe);
    } else {
		std::string msg = Format(_("Can't find team %s :-("), id.c_str());
		if (generate_error)
		  Erreur (msg);
		else
		  std::cout << "! " << msg << std::endl;
    }
	m_equipe_active = list.begin();
}
  
//-----------------------------------------------------------------------------

void TeamsList::SetActive(const std::string &id)
{
	iterator
		it = list.begin(),
		end = list.end();
	for (; it != end; ++it)
	{
		Team &team = **it;
		if (team.GetId() == id)
		{
			m_equipe_active = it;
			return;
		}
	}
	Erreur (Format(_("Can't find team %s !"), id.c_str()));
}
  
//-----------------------------------------------------------------------------

Team& ActiveTeam()
{ return teams_list.ActiveTeam(); }

//-----------------------------------------------------------------------------

Character& ActiveCharacter()
{ return ActiveTeam().ActiveCharacter(); }

//-----------------------------------------------------------------------------

bool compareTeams(const Team& a, const Team& b)
{
  return a.GetName() < b.GetName();
}

//-----------------------------------------------------------------------------
