#include "GameUtils.h"
#include <sstream>
#include <sys/time.h>
using namespace std;

static unsigned long long startTime = gameGetSystemTime();

const std::string gameToString(int value) {
    std::stringstream sstr;
    sstr << value;
    return sstr.str();
}

const std::string gameFloatToString(float value){
	std::stringstream sstr;
    sstr << value;
    return sstr.str();
}

unsigned long long gameGetSystemTime() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return
    (unsigned long long) now.tv_usec / 1000 +
            (unsigned long long) now.tv_sec * 1000;
}

unsigned long long gameGetElapsedTimeMillis() {
    return gameGetSystemTime() - startTime;
}

const int gameToInt(const std::string& intString) {
    int x = 0;
    istringstream cur(intString);
    cur >> x;
    return x;
}

const unsigned int gameToUnsignedInt(const std::string& intString){
	unsigned int x = 0;
	istringstream cur(intString);
    cur >> x;
    return x;
}