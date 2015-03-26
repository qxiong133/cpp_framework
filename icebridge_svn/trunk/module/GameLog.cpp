#include "GameLog.h"
#include "GameLogger.h"

GameLog::~GameLog() {
    if (topic.empty()) {
        GameLogger::instance().log(level, message.str());
    } else {
        GameLogger::instance().log(topic, level, message.str());
    }
}

void GameLog::setLevel(gameLogLevel logLevel) {
    GameLogger::instance().setLevel(logLevel);
}

gameLogLevel GameLog::getLevel() {
    return (gameLogLevel) GameLogger::instance().getLevel();
}


void GameLog::setFilePath(const string& file) {
    GameLogger::instance().setFilePath(file);
}

string GameLog::getFilePath() {
    return GameLogger::instance().getFilePath();
}

void GameLog::enableFileRotationMins(unsigned int minutes) {
    GameLogger::instance().enableFileRotationMins(minutes);
}

void GameLog::enableFileRotationHours(unsigned int hours) {
    GameLogger::instance().enableFileRotationHours(hours);
}

void GameLog::enableFileRotationDays(unsigned int days) {
    GameLogger::instance().enableFileRotationDays(days);
}

void GameLog::enableFileRotationMonths(unsigned int months) {
    GameLogger::instance().enableFileRotationMonths(months);
}

void GameLog::enableFileRotationSize(unsigned int sizeKB) {
    GameLogger::instance().enableFileRotationSize(sizeKB);
}

void GameLog::disableFileRotation() {
    GameLogger::instance().disableFileRotation();
}

void GameLog::setFileRotationMaxNum(unsigned int num) {
    GameLogger::instance().setFileRotationMaxNum(num);
}

void GameLog::setFileRotationNumber() {
    GameLogger::instance().setFileRotationNumber();
}

void GameLog::setFileRotationTimestamp() {
    GameLogger::instance().setFileRotationTimestamp();
}

void GameLog::addTopic(const string& logTopic, gameLogLevel logLevel) {
    GameLogger::instance().addTopic(logTopic, logLevel);
}

void GameLog::removeTopic(const string& logTopic) {
    GameLogger::instance().removeTopic(logTopic);
}

bool topicExists(const string& logTopic) {
    return GameLogger::instance().topicExists(logTopic);
}

void GameLog::setTopicLogLevel(const string& logTopic, gameLogLevel logLevel) {
    GameLogger::instance().setTopicLogLevel(logTopic, logLevel);
}

void GameLog::resetTopicLogLevel(const string& logTopic) {
    GameLogger::instance().resetTopicLogLevel(logTopic);
}

void GameLog::enableHeader() {
    GameLogger::instance().enableHeader();
}

void GameLog::disableHeader() {
    GameLogger::instance().disableHeader();
}

bool usingHeader() {
    return GameLogger::instance().usingHeader();
}

void GameLog::enableHeaderDate() {
    GameLogger::instance().enableHeaderDate();
}

void GameLog::disableHeaderDate() {
    GameLogger::instance().disableHeaderDate();
}

bool usingHeaderDate() {
    return GameLogger::instance().usingHeaderDate();
}

void GameLog::enableHeaderTime() {
    GameLogger::instance().enableHeaderTime();
}

void GameLog::disableHeaderTime() {
    GameLogger::instance().disableHeaderTime();
}

bool usingHeaderTime() {
    return GameLogger::instance().usingHeaderTime();
}

void GameLog::enableHeaderMillis() {
    GameLogger::instance().enableHeaderMillis();
}

void GameLog::disableHeaderMillis() {
    GameLogger::instance().disableHeaderMillis();
}

bool usingHeaderMillis() {
    return GameLogger::instance().usingHeaderMillis();
}