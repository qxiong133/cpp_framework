/* 
 * File:   GameConfig.h
 * Author: Edwin Xie
 *
 * Created on 2013年10月18日, 下午12:09
 */

#ifndef GAMECONFIG_H
#define	GAMECONFIG_H
#include <string>
#include "Poco/AutoPtr.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IniFileConfiguration.h"

#include <cstdlib>
#include <iostream>

class GameConfig {
    GameConfig(const std::string &ini_file);
public:
    static GameConfig& getInstance(const std::string &ini_file = ""){
		std::string file = ini_file;
        /*std::cout << std::getenv("PATH") << std::endl;
          std::cout << std::getenv("LD_LIBRARY_PATH") << std::endl;
          std::cout << std::getenv("GameLog") << std::endl;
          */
		if (file.empty() && std::getenv("GameLog")){
			file = std::getenv("GameLog");
		}
		if (file.empty())
			file = "../conf/game.conf";
		std::cout << file << std::endl;
        static GameConfig gc(file);
        return gc;
    }

    ~GameConfig();
    
    std::string getString(const std::string &key);
    std::string getString(const std::string &key, const std::string defaultValue);
    
    int getInt(const std::string &key);
    int getInt(const std::string &key, int defaultValue);

    double getDouble(const std::string &key);
    double getDouble(const std::string &key, double defaultValue);

    bool getBool(const std::string &key);
    bool getBool(const std::string &key, bool defaultValue);
protected:
    Poco::Util::IniFileConfiguration *_config;
};

#endif	/* GAMECONFIG_H */
