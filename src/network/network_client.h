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
 * Network client layer for Warmux.
 *****************************************************************************/

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H
//-----------------------------------------------------------------------------
#include "network.h"
//-----------------------------------------------------------------------------

class WSocket;

class NetworkClient : public Network
{
protected:
  connection_state_t HandShake(WSocket& server_socket);
  void HandleAction(Action* a, DistantComputer* sender) override;
  void WaitActionSleep() override {};

public:
  NetworkClient(const std::string& password);
  ~NetworkClient() override;

  //virtual const bool IsConnected() const { return true; }
  bool IsClient() const override { return true; }

  void CloseConnection(std::list<DistantComputer*>::iterator) override;

  // Client specific methods
  connection_state_t ClientConnect(const std::string& host,
                                   const std::string& port);

  std::string GetServerAddress() const;
};

//-----------------------------------------------------------------------------
#endif
