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
 * Statistics. 
 *****************************************************************************/

#include "stats.h"
#ifdef ENABLE_STATS
//-----------------------------------------------------------------------------
#include <SDL.h>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <iomanip>
//-----------------------------------------------------------------------------

typedef uint StatTime_t;

  class StatOutputItem
  {
  public:
    StatOutputItem(const std::string &p_function) 
      : function(p_function), total(0), min(0), max(0)
    {}
    bool operator< (const StatOutputItem& b) const
    {
      return total > b.total;
    }
    std::string function;
    uint count;
    StatTime_t total;
    StatTime_t min;
    StatTime_t max;
  };


StatTime_t StatGetTimer()
{
  return SDL_GetTicks();
}

class StatItem
{
public:
  std::vector<StatTime_t> time;
  typedef std::vector<StatTime_t>::const_iterator time_it;
  uint count;
  StatTime_t last_time;
  StatItem() : count(0) { last_time = StatGetTimer(); }
};

std::map<std::string, StatItem> stats;
typedef std::map<std::string, StatItem>::iterator stats_it;

//-----------------------------------------------------------------------------

void StatStart(const std::string &function)
{
  stats_it it = stats.find(function);
  StatItem &item = (it != stats.end()) ? it->second : stats[function];
  item.count++;
  item.last_time = StatGetTimer();
}

void StatStop(const std::string &function)
{
  stats_it it = stats.find(function);
  if (it == stats.end()) return;
  it->second.time.push_back(StatGetTimer() - it->second.last_time);
}

void StatOutput(const std::list<StatOutputItem> &table, StatTime_t total_time)
{
  const uint func_width = 30;
  std::list<StatOutputItem>::const_iterator it=table.begin(), end=table.end();
  std::cout << "[Stats]" << std::endl;
  std::cout
    << std::setw(func_width) << "Function" << " | "
    << std::setw(5) << "" << "%"
    << std::setw(8) << "Call"
    << std::setw(8) << "Total"
    << std::setw(8) << "Average"
    << std::setw(8) << "Min"
    << std::setw(8) << "Max"
    << std::endl;

  std::cout.setf(std::ios_base::fixed);
  std::cout.precision(2);
  for (; it != end; ++it)
  {
    std::cout
      << std::setw(func_width) << std::setiosflags(std::ios::left) 
      << it->function 
      << std::setiosflags(std::ios::right) << " | "

      << std::setw(5) << ((double)it->total*100/total_time) << "%"
      << std::setw(8) << it->count
      << std::setw(8) << it->total
      << std::setw(8) << it->total/it->count
      << std::setw(8) << it->min
      << std::setw(8) << it->max
      << std::endl;
  }
    std::cout
      << std::setw(func_width) << "(total)" << " | "
      << std::setw(5) << 100 << "%"
      << std::setw(8) << ""
      << std::setw(8) << total_time
      << std::setw(8) << ""
      << std::setw(8) << ""
      << std::setw(8) << ""
      << std::endl;

  std::cout << std::endl;
}

void StatOutput()
{
  std::list<StatOutputItem> table;
  std::list<StatOutputItem>::iterator table_it;
  stats_it it = stats.begin(), end=stats.end();
  StatTime_t total_time = 0;
  for (; it != end; ++it)
  {
    StatOutputItem item(it->first);
    item.count = it->second.time.size();
    StatItem::time_it time = it->second.time.begin(), time_end=it->second.time.end();
    if (time != time_end)
    {
      item.min = *time; 
      item.max = *time;
      for (; time != time_end; ++time)
      {
        item.total += *time;
        item.min = std::min(item.min, *time);
        item.max = std::max(item.max, *time);
      }
    }
    table.push_back(item);
    total_time += item.total;
  }
  table.sort();
  StatOutput(table, total_time);
}

//-----------------------------------------------------------------------------
#endif // ENABLE_STATS
