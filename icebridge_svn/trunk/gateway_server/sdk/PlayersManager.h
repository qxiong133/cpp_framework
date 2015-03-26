/*
 * PlayersManager.h
 *
 *  Created on: Aug 11, 2013
 *      Author: root
 */

#ifndef PLAYERSMANAGER_H_
#define PLAYERSMANAGER_H_
#include    <boost/shared_ptr.hpp>
#include    <boost/bimap.hpp>
#include    <boost/optional.hpp>
#include    "sdk/tcp_defs.h"
#include    <string>
#include <set>
#include "sdk/tcp_session.h"
#include "sdk/tcp_session_handler.h"
#include <list>

class PlayersManager {

public:

	typedef unsigned int  PlayerID;
	typedef boost::bimap<PlayerID, eddy::TCPSessionID> PlayerSessionBiMap;
	typedef PlayerSessionBiMap::value_type PlayerSession;
	typedef PlayerSessionBiMap::left_const_iterator LeftIterator;
	typedef PlayerSessionBiMap::right_const_iterator RightIterator;
    typedef std::set<unsigned int>    PlayerIDSet;

	PlayersManager();
	virtual ~PlayersManager();

	static PlayersManager *sharedInstance() {
         static PlayersManager* pSharedInstance = NULL;

		if (pSharedInstance == NULL) {
			pSharedInstance = new PlayersManager();
		}

		return pSharedInstance;
	}

	void onLogin(PlayerID playerID, eddy::TCPSessionID sessionID);
	void onPlayerOut(eddy::TCPSessionID sessionID);
	void loginFailed(eddy::TCPSessionID sessionID);

	PlayerID getPlayerID(eddy::TCPSessionID);
	eddy::TCPSessionID getSessionID(PlayerID playerID);

	boost::optional<eddy::TCPSessionID> getBeginSessionID();
	//eddy::TCPSessionID getEndSession();
	boost::optional<eddy::TCPSessionID> getNextSessionID();

private:
	PlayerSessionBiMap playerSessionBiMap_;
	LeftIterator leftCurIterator;
	bool activeClose;
	//static PlayersManager *pSharedInstance;
	PlayerIDSet playerIDDeletedSet;
};

#endif /* PLAYERSMANAGER_H_ */
