//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name netdriver.h - Net driver header file. */
//
//      (c) Copyright 2005 by Edward Haase and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

#ifndef __NETDRIVER_H__
#define __NETDRIVER_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "network/net_lowlevel.h"

#include <time.h>

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define DEFAULT_PORT			7775			// Server port
#define DEFAULT_MAX_CONN		500			// Max Connections
#define DEFAULT_SESSION_TIMEOUT		900			// 15 miniutes
#define DEFAULT_POLLING_DELAY		250			// MS (1000 = 1s)

#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32

#define MAX_GAMENAME_LENGTH 32
#define MAX_VERSION_LENGTH 8

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class GameData;

/**
** Global server variables.
*/
class ServerStruct {
public:
	int Port = 0;
 	int MaxConnections = 0;
	int IdleTimeout = 0;
	int PollingDelay = 0;
};

extern ServerStruct Server;

/**
**  Session data
**
**  One per connection.
*/
class Session {
public:
	Session()
	{
		Buffer[0] = '\0';
		AddrData.Host = 0;
		AddrData.IPStr[0] = '\0';
		AddrData.Port = 0;
		UserData.Name[0] = '\0';
		UserData.GameName[0] = '\0';
		UserData.Version[0] = '\0';
		UserData.LoggedIn = 0;
	}
	
	Session *Next = nullptr;
	Session *Prev = nullptr;

	char Buffer[1024];
	time_t Idle = 0;

	Socket Sock = 0;

	struct {
		unsigned long Host;
		char IPStr[16];
		int Port;
	} AddrData;               /// Remote address data.

	struct {
		char Name[MAX_USERNAME_LENGTH + 1];
		char GameName[MAX_GAMENAME_LENGTH + 1];
		char Version[MAX_VERSION_LENGTH + 1];
		int LoggedIn;
	} UserData;               /// Specific user data.

	GameData *Game = nullptr;
};

/**
**  Global session tracking.
*/
class SessionPool {
public:
	Session *First = nullptr;
	Session *Last = nullptr;
	int Count = 0;

	SocketSet *Sockets = nullptr;
};

	/// external reference to session tracking.
extern SessionPool *Pool;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void Send(Session *session, const char *msg);

extern int ServerInit(int port);
extern void ServerQuit(void);
extern int UpdateSessions(void);

#endif
