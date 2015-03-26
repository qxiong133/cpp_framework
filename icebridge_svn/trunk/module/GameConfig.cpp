#include <iostream>
#include "Poco/Exception.h"
#include "GameConfig.h"
#include "GameLog.h"
using namespace std;

GameConfig::GameConfig(const std::string &ini_file) {
    _config = new Poco::Util::IniFileConfiguration(ini_file);
}

GameConfig::~GameConfig() {
}

/*
void GameConfig::Load(const std::string& ini_file) {
    _config->load(ini_file);
}
 */

std::string GameConfig::getString(const std::string &key) {
    std::string value;
    try {
        value = _config->getString(key);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed.";
        value = "";
    }
    return value;
}

std::string GameConfig::getString(const std::string &key, std::string defaultValue) {
    std::string value;
    try {
        value = _config->getString(key, defaultValue);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed. Default value \"" << defaultValue << "\" returned.";
        value = defaultValue;
    }
    return value;
}

int GameConfig::getInt(const std::string &key) {
    int value;
    try {
        value = _config->getInt(key);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed.";
        value = 0;
    }
    return value;
}

int GameConfig::getInt(const std::string &key, int defaultValue) {
    int value;
    try {
        value = _config->getInt(key, defaultValue);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed. Default value \"" << defaultValue << "\" returned.";
        value = defaultValue;
    }
    return value;
}

double GameConfig::getDouble(const std::string &key) {
    double value;
    try {
        value = _config->getDouble(key);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed.";
        value = 0;
    }
    return value;
}

double GameConfig::getDouble(const std::string &key, double defaultValue) {
    double value;
    try {
        value = _config->getDouble(key, defaultValue);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed. Default value \"" << defaultValue << "\" returned.";
        value = defaultValue;
    }
    return value;
}

bool GameConfig::getBool(const std::string &key) {
    bool value;
    try {
        value = _config->getBool(key);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed.";
        value = false;
    }
    return value;
}

bool GameConfig::getBool(const std::string &key, bool defaultValue) {
    bool value;
    try {
        value = _config->getBool(key, defaultValue);
    } catch (Poco::Exception& e) {
        GameLogWarning() << "read config key <" << key << "> failed. Default value \"" << defaultValue << "\" returned.";
        value = defaultValue;
    }
    return value;
}
