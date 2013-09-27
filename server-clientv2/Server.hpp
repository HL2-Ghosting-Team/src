#pragma once
#include <map>
#include <string>
#include "shared\NetworkPackages.h"
#include "shared\Ghost.h"

#define TIMEOUTTICKS 660 // 10 seconds

class Server
{
public:
	Server(void);
	~Server(void);
	void ReceiveMessage(const std::string& senderip, const std::string& data);
	void SendConnectAckPackage(const std::string& ip, bool isConn);
	void StartRace();
	void Tick();
	void SendUpdatePackage();

	void SendToAll(const std::string& data);

	std::map<std::string,Ghost> ghosts;
	std::string password;
	int currenttick;
	bool racestarted;
private:
	void HandleConnectPackage(const std::string& senderip, const std::string& data);
	void HandleMapChangePackage(const std::string& senderip, const std::string& data);
	void HandleModelChangePackage(const std::string& senderip, const std::string& data);
	void HandlePongPackage(const std::string& senderip, const std::string& data);
	void HandleUpdatePackage(const std::string& senderip, const std::string& data);
};

