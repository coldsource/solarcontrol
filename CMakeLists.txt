cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Release)

project(solarcontrol)

aux_source_directory(src/mqtt srcMQTT)
aux_source_directory(src/energy srcEnergy)
aux_source_directory(src/datetime srcDatetime)
aux_source_directory(src/control srcControl)
aux_source_directory(src/device srcDevice)
aux_source_directory(src/device/electrical srcDeviceElectrical)
aux_source_directory(src/device/weather srcDeviceWeather)
aux_source_directory(src/thread srcThread)
aux_source_directory(src/utils srcUtils)
aux_source_directory(src/configuration srcConfiguration)
aux_source_directory(src/database srcDatabase)
aux_source_directory(src/websocket srcWebsocket)
aux_source_directory(src/api srcAPI)
aux_source_directory(src/display srcDisplay)
aux_source_directory(src/logs srcLogs)
aux_source_directory(src/weather srcWeather)
aux_source_directory(src/meter srcMeter)
aux_source_directory(src/shelly srcShelly)

add_executable(solarcontrol
	${srcMQTT}
	${srcEnergy}
	${srcDatetime}
	${srcControl}
	${srcDevice}
	${srcDeviceElectrical}
	${srcDeviceWeather}
	${srcThread}
	${srcUtils}
	${srcConfiguration}
	${srcDatabase}
	${srcWebsocket}
	${srcAPI}
	${srcDisplay}
	${srcLogs}
	${srcWeather}
	${srcMeter}
	${srcShelly}
	
	src/solarcontrol.cpp
	)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
else()
    message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++20 support.")
endif()

target_compile_options(solarcontrol PRIVATE -Wall -Wextra -Wpedantic)

find_path(MYSQL_INCLUDE mysql.h /usr/include/mysql)
find_path(MARIADB_INCLUDE mysql.h /usr/include/mariadb)
if(MYSQL_INCLUDE)
	add_definitions(-DMYSQL_PATH_MYSQL)
	target_link_libraries(solarcontrol mysqlclient)
	message("Found mysql.h in /usr/include/mysql")
elseif(MARIADB_INCLUDE)
	add_definitions(-DMYSQL_PATH_MARIADB)
	target_link_libraries(solarcontrol mariadbclient)
	message("Found mysql.h in /usr/include/mariadb")
else()
	 message(FATAL_ERROR "Needing lib mysqlclient or mariadbclient")
endif()

target_link_libraries(solarcontrol mosquitto)
target_link_libraries(solarcontrol curl)
target_link_libraries(solarcontrol websockets)

include_directories(include /usr/include)
