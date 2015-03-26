#include "PlayerManager.h"
#include <time.h>

static OnlinePlayersMAP onlinePlayers;

static OnlinePlayersMAP::iterator iter;

bool isOnline(unsigned int uid) {
    iter = onlinePlayers.find(uid);
    return (iter != onlinePlayers.end()) ? true : false;
}

void userLogin(unsigned int uid) {
    time_t timep;
    time(&timep);
    onlinePlayers[uid] = timep;
}

void userLogout(unsigned int uid) {
    iter = onlinePlayers.find(uid);
    
    if (iter != onlinePlayers.end()) {
        onlinePlayers.erase(iter);
    }
}