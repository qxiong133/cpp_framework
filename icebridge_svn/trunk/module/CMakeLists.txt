ADD_DEFINITIONS(-DDEBUG)
ADD_DEFINITIONS("-Wall -ansi -pedantic -g -fpermissive")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/lib)
#SET(SRC_LIST main.cpp gameLogger.cpp redisClient.cpp AppLogger.cpp ofxLog.cpp ofxLogger.cpp)
#SET(SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
AUX_SOURCE_DIRECTORY(. SRC_LIST)
ADD_EXECUTABLE(gameServer ${SRC_LIST})
TARGET_LINK_LIBRARIES(gameServer PocoUtild PocoDataMySQLd PocoDatad PocoZipd PocoDataSQLited PocoNetd PocoXMLd hiredis pthread PocoFoundationd mysqlclient z)
INSTALL(TARGETS gameServer RUNTIME DESTINATION bin)
