#CPP := arm-linux-gnueabi-g++
CPP := g++

all: server client

COMMON_SOURCE := RoomManager.cpp
SRV_SOURCE := server.cpp $(COMMON_SOURCE)
CLI_SOURCE := client.cpp $(COMMON_SOURCE)

FLAGS := -std=c++0x -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive

server: $(SRV_SOURCE)
	$(CPP) -o $@ $(SRV_SOURCE) $(FLAGS)

client: $(CLI_SOURCE)
	$(CPP) -o $@ $(CLI_SOURCE) $(FLAGS)

