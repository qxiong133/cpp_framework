#ifndef GAME_LOGGER_H
#define GAME_LOGGER_H

#include <Poco/AutoPtr.h>
#include <Poco/Logger.h>
#include <Poco/FileChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/SplitterChannel.h>
#include "GameLog.h"

class GameLogger {
public:
    static GameLogger& instance();

    void log(gameLogLevel logLevel, const std::string& message);
    void log(const std::string& logTopic, gameLogLevel logLevel, const std::string& message);

    void setLevel(gameLogLevel logLevel);
    gameLogLevel getLevel();

    void setFilePath(const std::string& file);
    std::string getFilePath();

    void enableFileRotationMins(unsigned int minutes);
    void enableFileRotationHours(unsigned int hours);
    void enableFileRotationDays(unsigned int days);
    void enableFileRotationMonths(unsigned int months);
    void enableFileRotationSize(unsigned int sizeKB);
    void disableFileRotation();

    void setFileRotationMaxNum(unsigned int num);

    /// Set the suffix appended to the rotated log files:
    ///  - number: logfile.log.#
    ///	 - timestamp: logfile.log.YYYYMMDDHHMMSSms
    void setFileRotationNumber();
    void setFileRotationTimestamp();

    void addTopic(const std::string& logTopic, gameLogLevel logLevel = GAME_LOG_NOTICE);
    void removeTopic(const std::string& logTopic);
    bool topicExists(const std::string& logTopic);
    void setTopicLogLevel(const std::string& logTopic, gameLogLevel logLevel);
    void resetTopicLogLevel(const std::string& logTopic);

    void enableHeader();
    void disableHeader();
    bool usingHeader();

    void enableHeaderDate();
    void disableHeaderDate();
    bool usingHeaderDate();

    void enableHeaderTime();
    void disableHeaderTime();
    bool usingHeaderTime();

    void enableHeaderMillis();
    void disableHeaderMillis();
    bool usingHeaderMillis();

protected:

    Poco::AutoPtr<Poco::Logger> logger; ///< the logger
    Poco::AutoPtr<Poco::Channel> formattingChannel; ///< formatter (needed for creating topics)
    Poco::AutoPtr<Poco::SplitterChannel> splitterChannel; ///< channel source mixer
    Poco::AutoPtr<Poco::FileChannel> fileChannel; ///< the file io channel

    bool bHeader;
    bool bDate;
    bool bTime;
    bool bMillis;
private:

    /// logs the message to the specified logger
    void _log(gameLogLevel logLevel, const std::string& message, Poco::Logger* theLogger);

    /// this is used if the logger has been destroyed
    void _logDestroyed(const string& message);

    /// convert an OF log level to the Poco log level and vice versa
    gameLogLevel _convertPocoLogLevel(const int level);
    int _convertGameLogLevel(const gameLogLevel level);

    /// static date format strings for the header
    static const std::string s_dateFormat;
    static const std::string s_timeFormat;
    static const std::string s_dateAndTimeFormat;

    // hide all the constructors, copy functions here
    GameLogger(GameLogger const&); // not defined, not copyable
    GameLogger& operator =(GameLogger const&); // not defined, not assignable
    GameLogger(); // singleton constructor

    ~GameLogger() {
    }
};
#endif