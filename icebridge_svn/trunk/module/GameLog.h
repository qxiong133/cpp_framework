/* 
 * File:   GameLog.h
 * Author: Edwin Xie
 *
 * Created on 2013年10月17日, 下午2:18
 */

#ifndef GAMELOG_H
#define	GAMELOG_H
//#include <string>
#include <sstream>
using namespace std;

enum gameLogLevel {
    GAME_LOG_VERBOSE,
    GAME_LOG_NOTICE,
    GAME_LOG_WARNING,
    GAME_LOG_ERROR,
    GAME_LOG_FATAL_ERROR,
    GAME_LOG_SILENT // this one is special and should always be last,
    // set ofSetLogLevel to GAME_SILENT to not receive any messages
};

class GameLog {
public:

    GameLog() {
        level = GAME_LOG_NOTICE;
    }

    GameLog(string logTopic) {
        level = GAME_LOG_NOTICE;
        topic = logTopic;
    }

    ~GameLog();

    template <class T>
    GameLog& operator<<(const T& value) {
        message << value;
        return *this;
    }

    GameLog& operator<<(std::ostream& (*func)(std::ostream&)) {
        func(message);
        return *this;
    }
	void log(std::string msg){
		message << msg;
	}
    static void setLevel(gameLogLevel logLevel);
    static gameLogLevel getLevel();

    static void setFilePath(const string& file);
    static string getFilePath();

    static void enableFileRotationMins(unsigned int minutes);
    static void enableFileRotationHours(unsigned int hours);
    static void enableFileRotationDays(unsigned int days);
    static void enableFileRotationMonths(unsigned int months);
    static void enableFileRotationSize(unsigned int sizeKB);
    static void disableFileRotation();

    static void setFileRotationMaxNum(unsigned int num);

    static void setFileRotationNumber();
    static void setFileRotationTimestamp();

    static void addTopic(const string& logTopic, gameLogLevel logLevel = GAME_LOG_NOTICE);
    static void removeTopic(const string& logTopic);
    static bool topicExists(const string& logTopic);
    static void setTopicLogLevel(const string& logTopic, gameLogLevel logLevel);
    static void resetTopicLogLevel(const string& logTopic);

    static void enableHeader();
    static void disableHeader();
    static bool usingHeader();

    static void enableHeaderDate();
    static void disableHeaderDate();
    static bool usingHeaderDate();

    static void enableHeaderTime();
    static void disableHeaderTime();
    static bool usingHeaderTime();
    static void enableHeaderMillis();
    static void disableHeaderMillis();
    static bool usingHeaderMillis();

protected:

    gameLogLevel level; //log level
    std::string topic; //log topic

private:
    std::ostringstream message;

    GameLog(GameLog const&) {
    }

    GameLog& operator=(GameLog& from) {
        return *this;
    }
};

class GameLogVerbose : public GameLog {
public:

    GameLogVerbose() {
        level = GAME_LOG_VERBOSE;
    }

    GameLogVerbose(std::string logTopic) {
        level = GAME_LOG_VERBOSE;
        topic = logTopic;
    }
};

class GameLogWarning : public GameLog {
public:

    GameLogWarning() {
        level = GAME_LOG_WARNING;
    }

    GameLogWarning(std::string logTopic) {
        level = GAME_LOG_WARNING;
        topic = logTopic;
    }
};

class GameLogError : public GameLog {
public:

    GameLogError() {
        level = GAME_LOG_ERROR;
    }

    GameLogError(std::string logTopic) {
        level = GAME_LOG_ERROR;
        topic = logTopic;
    }
};

class GameLogFatalError : public GameLog {
public:

    GameLogFatalError() {
        level = GAME_LOG_FATAL_ERROR;
    }

    GameLogFatalError(std::string logTopic) {
        level = GAME_LOG_FATAL_ERROR;
        topic = logTopic;
    }
};
#endif	/* GAMELOG_H */
