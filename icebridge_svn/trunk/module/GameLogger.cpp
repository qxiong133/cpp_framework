#include "GameLogger.h"
#include "GameUtils.h"

#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Message.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormatter.h>
//#include <string>  
#include <sstream> 
#include <iostream>

const string GameLogger::s_dateFormat = "%Y-%m-%d %H:%M:%S.%i";
const string GameLogger::s_timeFormat = "%H:%M:%S.%i";
const string GameLogger::s_dateAndTimeFormat = "%Y-%m-%d %H:%M:%S.%i";

GameLogger::GameLogger() {
    bHeader = true;
    bDate = true;
    bTime = true;
    bMillis = true;

    splitterChannel = new Poco::SplitterChannel();
    fileChannel = new Poco::FileChannel("../log/game.log");

    splitterChannel->addChannel(fileChannel);

    Poco::AutoPtr<Poco::Formatter> formatter(new Poco::PatternFormatter("[%p] %t"));
    formattingChannel = new Poco::FormattingChannel(formatter, splitterChannel);

    // the root logger has an empty name
    logger = &Poco::Logger::create("root", formattingChannel, Poco::Message::PRIO_NOTICE);

    // setup file logger
    fileChannel->setProperty("times", "local"); // use local system time
    //fileChannel->setProperty("archive", "number"); // use number suffixs
    fileChannel->setProperty("compress", "false"); // don't compress
    //fileChannel->setProperty("purgeCount", "10"); // max number of log files

    //add default of topic
    addTopic("dev");
}

GameLogger& GameLogger::instance() {
    static GameLogger* pointerToTheSingletonInstance = new GameLogger;
    return *pointerToTheSingletonInstance;
}

void GameLogger::log(gameLogLevel logLevel, const string& message) {
    _log(logLevel, message, logger);
}

void GameLogger::log(const string& logTopic, gameLogLevel logLevel, const string& message) {
    Poco::Logger* topicLogger = Poco::Logger::has(logTopic);
    if (!topicLogger) {
        _log(GAME_LOG_WARNING, "log topic \"" + logTopic + "\" not found", logger);
    } else {
        _log(logLevel, logTopic + ": " + message, topicLogger);
    }
}

void GameLogger::setLevel(gameLogLevel logLevel) {
    logger->setLevel(_convertGameLogLevel(logLevel));
}

gameLogLevel GameLogger::getLevel() {
    return _convertPocoLogLevel(logger->getLevel());
}

void GameLogger::setFilePath(const string& file) {
    fileChannel->setProperty("path", file);
}

string GameLogger::getFilePath() {
    return fileChannel->getProperty("path");
}

void GameLogger::enableFileRotationMins(unsigned int minutes) {
    fileChannel->setProperty("rotation", gameToString(minutes) + " minutes");
}

void GameLogger::enableFileRotationHours(unsigned int hours) {
    fileChannel->setProperty("rotation", gameToString(hours) + " hours");
}

void GameLogger::enableFileRotationDays(unsigned int days) {
    fileChannel->setProperty("rotation", gameToString(days) + " days");
}

void GameLogger::enableFileRotationMonths(unsigned int months) {
    fileChannel->setProperty("rotation", gameToString(months) + " months");
}

void GameLogger::enableFileRotationSize(unsigned int sizeKB) {
    fileChannel->setProperty("rotation", gameToString(sizeKB) + " K");
}

void GameLogger::disableFileRotation() {
    fileChannel->setProperty("rotation", "never");
}

//--------------------------------------------------------------------------------

void GameLogger::setFileRotationMaxNum(unsigned int num) {
    fileChannel->setProperty("purgeCount", gameToString(num));
}

//--------------------------------------------------------------------------------

void GameLogger::setFileRotationNumber() {
    fileChannel->setProperty("archive", "number");
}

void GameLogger::setFileRotationTimestamp() {
    fileChannel->setProperty("archive", "timestamp");
}

void GameLogger::addTopic(const string& logTopic, gameLogLevel logLevel) {
    try {
        Poco::Logger::create(logTopic, formattingChannel, _convertGameLogLevel(logLevel));
    } catch (Poco::ExistsException& e) {
        log(GAME_LOG_WARNING, "log topic \"" + logTopic + "\" does not exist");
    }
}

void GameLogger::removeTopic(const string& logTopic) {
    Poco::Logger::destroy(logTopic);
}

bool GameLogger::topicExists(const string& logTopic) {
    return (bool) Poco::Logger::has(logTopic); // has returns NULL if topic not found
}

void GameLogger::setTopicLogLevel(const string& logTopic, gameLogLevel logLevel) {
    Poco::Logger* topicLogger = Poco::Logger::has(logTopic);
    if (topicLogger) {
        topicLogger->setLevel(_convertGameLogLevel(logLevel));
    } else {
        log(GAME_LOG_WARNING, "log topic \"" + logTopic + "\" not found");
    }
}

void GameLogger::resetTopicLogLevel(const string& logTopic) {
    setTopicLogLevel(logTopic, _convertPocoLogLevel(logger->getLevel()));
}

void GameLogger::enableHeader() {
    bHeader = true;
}

void GameLogger::disableHeader() {
    bHeader = false;
}

bool GameLogger::usingHeader() {
    return bHeader;
}

//----------------------------

void GameLogger::enableHeaderDate() {
    bDate = true;
}

void GameLogger::disableHeaderDate() {
    bDate = false;
}

bool GameLogger::usingHeaderDate() {
    return bDate;
}
//----------------------------

void GameLogger::enableHeaderTime() {
    bTime = true;
}

void GameLogger::disableHeaderTime() {
    bTime = false;
}

bool GameLogger::usingHeaderTime() {
    return bTime;
}

void GameLogger::enableHeaderMillis() {
    bMillis = true;
}

void GameLogger::disableHeaderMillis() {
    bMillis = false;
}

bool GameLogger::usingHeaderMillis() {
    return bMillis;
}

void GameLogger::_log(gameLogLevel logLevel, const string& message, Poco::Logger* theLogger) {
    string timestamp;

    // build the header
    if (bHeader) {
        Poco::LocalDateTime now;

        if (bDate && bTime) {
            timestamp += Poco::DateTimeFormatter::format(now, s_dateAndTimeFormat) + " ";
        } else if (bDate) {
            timestamp += Poco::DateTimeFormatter::format(now, s_dateFormat) + " ";
        } else if (bTime) {
            timestamp += Poco::DateTimeFormatter::format(now, s_timeFormat) + " ";
        }

        if (bMillis) {
            timestamp += gameToString(gameGetElapsedTimeMillis()) + " ";
        }
    }

    // log the message
    //
    // Each log call is wrapped in a try / catch in case the logger is called
    // when it has already been destroyed. This can happen if it is used in
    // another destructor as the destruction order is not predictabale.
    //
    switch (logLevel) {
        case GAME_LOG_SILENT:
            break;

        case GAME_LOG_VERBOSE:
            try {
                theLogger->trace(timestamp + message);
            } catch (...) {
                _logDestroyed("VERBOSE: " + message);
            }
            break;

        case GAME_LOG_NOTICE:
            try {
                theLogger->notice(timestamp + message);
            } catch (...) {
                _logDestroyed(message);
            }
            break;

        case GAME_LOG_WARNING:
            try {
                theLogger->warning(timestamp + message);
            } catch (...) {
                _logDestroyed("WARNING: " + message);
            }
            break;

        case GAME_LOG_ERROR:
            try {
                theLogger->error(timestamp + message);
            } catch (...) {
                _logDestroyed("ERROR: " + message);
            }
            break;

        case GAME_LOG_FATAL_ERROR:
            try {
                theLogger->fatal(timestamp + message);
            } catch (...) {
                _logDestroyed("FATAL_ERROR: " + message);
            }
            break;
    }
}

void GameLogger::_logDestroyed(const string& message) {
    std::cout << "----------\n\tHey ... don't call ofxLog in a destructor!" << std::endl;
    std::cout << "\t%s\n" << message << "----------" << std::endl;
}

gameLogLevel GameLogger::_convertPocoLogLevel(const int level) {
    switch (level) {
        case Poco::Message::PRIO_TRACE:
            return GAME_LOG_VERBOSE;

        case Poco::Message::PRIO_NOTICE:
            return GAME_LOG_NOTICE;

        case Poco::Message::PRIO_WARNING:
            return GAME_LOG_WARNING;

        case Poco::Message::PRIO_ERROR:
            return GAME_LOG_ERROR;

        case Poco::Message::PRIO_FATAL:
            return GAME_LOG_FATAL_ERROR;

        case -1:
            return GAME_LOG_SILENT;
    }
}

int GameLogger::_convertGameLogLevel(const gameLogLevel level) {
    switch (level) {
        case GAME_LOG_VERBOSE:
            return Poco::Message::PRIO_TRACE;

        case GAME_LOG_NOTICE:
            return Poco::Message::PRIO_NOTICE;

        case GAME_LOG_WARNING:
            return Poco::Message::PRIO_WARNING;

        case GAME_LOG_ERROR:
            return Poco::Message::PRIO_ERROR;

        case GAME_LOG_FATAL_ERROR:
            return Poco::Message::PRIO_FATAL;

        case GAME_LOG_SILENT:
            return -1;
    }
}
