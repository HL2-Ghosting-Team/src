#include "GhostRun.h"
#include "runline.h"
#include "GhostOnlineRun.h"
#include "GhostUtils.h"
#include <SFML\Network.hpp>
#pragma once
class GhostOnlineEngine {

public:
	GhostOnlineEngine() {
		shouldAct = false;
		mapNameDirty = true;
		firstTime = true;
		nextTime = 0.0f;
		inTransition = false;
	};
	//gets the singleton Engine instance
	static GhostOnlineEngine* getEngine();
	CUtlVector<GhostOnlineRun*> ghosts;
	//CUtlVector<OnlineGhostRun*> onlineGhosts;
	static GhostOnlineEngine* instance;
	bool isActive();

	void EndRun(GhostOnlineRun*);
	//make this start the ghost by setting up the data and name
	void StartRun(const char*, bool);
	void ResetGhosts(void);//respawn the ghosts with their correct data

	void sendFirstData();

	void handleEvent(sf::Packet*);

	static float nextTime;
	static bool shouldAct;
	static bool firstTime;
	static bool mapNameDirty;

	static void connect(void);//bind the socket, make the thread
	static void disconnect(void);//close the socket, remove all ghost entities (clear the vector)


	static sf::UdpSocket sendSock;

	static sf::UdpSocket receiveSock;
	static sf::IpAddress serverIPAddress;

	void handleLine(sf::Packet*);
	void handleDisconnect(sf::Packet*);
	void handleConnect(sf::Packet*);
	GhostOnlineRun* getRun(const char*);//TODO make this by name, the recieve thread is going to use this heavily!
	void stopAllRuns();	
	void sendLine(RunLine);
	bool inTransition;

};