#ifndef PLAYERMANAGER
#define PLAYERMANAGER


#include <map>

//<uid, online time>
typedef std::map<unsigned int, long> OnlinePlayersMAP;

//判断玩家是否在线
bool isOnline(unsigned int uid);
void userLogin(unsigned int uid);
void userLogout(unsigned int uid);

#endif