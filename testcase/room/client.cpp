#include "RoomManager.h"

int main()
{
	char name[16];
	struct timeval tv;
	gettimeofday(&tv, NULL);
	std::srand(tv.tv_sec*1000*1000UL+tv.tv_usec);
	snprintf(name, sizeof(name), "%d", int(std::rand()*100.0f/RAND_MAX));
	printf("Name: %s\n", name);

	RoomManager *manager = RoomManager::getInstance();

	std::vector<RoomManager::RoomInfo> v;

	v = manager->scanRoom(1, 65536);

	printf("Found: %s\n", v[0].host.c_str());
	RoomClient *client = manager->joinRoom(name, v[0]);
	printf("Join: %s\n", v[0].host.c_str());

	auto event = [](RoomClient *client, const RoomPacket &packet) {
		if (packet["TYPE"] == "message") {
			std::cout << std::endl << packet["FROM"] << ": " 
				<< packet["CONTENT"] << std::endl;
		}
	};
	client->on("message", event);

	sleep(1);
	client->broadcastMessage("hello1");
	client->broadcastMessage("hello2");

	ssize_t read;
	size_t len = 0;
	char *line = NULL;
	while ((read = getline(&line, &len, stdin)) != -1) {
		client->broadcastMessage(std::string(line, read-1));
		printf("me: ");
	}
	free(line);

	manager->leaveRoom(client);
}
