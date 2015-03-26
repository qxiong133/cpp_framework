/*
 * PlayersManager.cpp
 *
 *  Created on: Aug 11, 2013
 *      Author: root
 */

#include "PlayersManager.h"
#include "sdk/tcp_session_handler.h"
#include "sdk/tcp_io_thread_manager.h"
#include <iostream>

using namespace eddy;
using namespace std;
using namespace boost;

extern TCPIOThreadManager manager;

PlayersManager::PlayersManager() {
	// TODO Auto-generated constructor stub
	activeClose = false;
}

PlayersManager::~PlayersManager() {
	// TODO Auto-generated destructor stub
}

/*once the client logins,we register it into the two maps playerSessionMap_,sessionPlayerMap_. if the session has
 exist before,then close it so the player can relate to the new session

 */
void PlayersManager::onLogin(PlayerID playerID, TCPSessionID sessionID) {

	//TCPSessionID sessionID = handler.session_id();

	LeftIterator it = playerSessionBiMap_.left.find(playerID);
	/*std::cout << "playerID = " << playerID << "    sessionID = " << sessionID
			<< std::endl;*/
	if (it != playerSessionBiMap_.left.end()) {

		playerSessionBiMap_.erase(PlayerSession(playerID, sessionID));
		playerIDDeletedSet.insert(playerID);

		//this player is already login , so close the old one
		manager.GetSessionHandler(it->second)->Close();
	}

	//now it is ok to add this new player
	playerSessionBiMap_.insert(PlayerSession(playerID, sessionID));
}

// once the session out in whatever situation ,the function should always be invoked
void PlayersManager::onPlayerOut(TCPSessionID sessionID) {

	//TCPSessionID sessionID = handler.session_id();
	RightIterator it = playerSessionBiMap_.right.find(sessionID);

	if (it != playerSessionBiMap_.right.end()) {

		if (playerIDDeletedSet.find(it->second) == playerIDDeletedSet.end()) {

			playerSessionBiMap_.erase(PlayerSession(it->second, sessionID));

		} else {
			playerIDDeletedSet.erase(it->second);
		}

	}

}

void PlayersManager::loginFailed(eddy::TCPSessionID sessionID) {

	//handler.Close();
}

PlayersManager::PlayerID PlayersManager::getPlayerID(
		eddy::TCPSessionID sessionID) {

	PlayerID playerID;
	RightIterator it = playerSessionBiMap_.right.find(sessionID);

	if (it != playerSessionBiMap_.right.end()) {
		playerID = it->second;
	}

	return playerID;
}

TCPSessionID PlayersManager::getSessionID(PlayerID playerID) {

	LeftIterator it = playerSessionBiMap_.left.find(playerID);

	if (it != playerSessionBiMap_.left.end()) {

		return it->second;

	}

	return kInvalidTCPSessionID;
}
/*
 PlayersManager::LeftIterator PlayersManager::getBeginSession(){
 return playerSessionBiMap_.left.begin();
 }

 PlayersManager::LeftIterator PlayersManager::getNextSession(LeftIterator it){

 return
 }
 */

boost::optional<TCPSessionID> PlayersManager::getBeginSessionID() {

	LeftIterator it = playerSessionBiMap_.left.begin();
	//leftCurIterator = ++it;

	if (it != playerSessionBiMap_.left.end()) {
		TCPSessionID id = it->second;
		leftCurIterator = ++it;

		return optional<TCPSessionID>(id);
	}

	return optional<TCPSessionID>(boost::none);
}

//eddy::TCPSessionID getEndSession();
boost::optional<eddy::TCPSessionID> PlayersManager::getNextSessionID() {

	return optional<TCPSessionID>(boost::none);
}

