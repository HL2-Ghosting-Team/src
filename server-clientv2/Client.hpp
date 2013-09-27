#pragma once
#include <map>
#include <string>
#include "shared\Ghost.h"

class Client
{
public:
	void ReceiveMessage(const std::string& data);

	std::string SendPongPackage();
	std::string SendConnectPackage(std::string password);
	std::string SendMapChangePackage(std::string map);
	std::string SendModelChangePackage(std::string model);
	std::string SendUpdatePackage(float x, float y, float z, float vx, float vy, float vz);
	
	std::map<std::string,Ghost> ghosts;
	Client(std::string name);
	~Client(void);
	std::string myName;
	std::string myIP;
	int currenttick;
private:
	void HandleConnectAckPackage(const std::string& data);
	void HandleRaceStartPackage(const std::string& data);
	void HandleUpdatePackage(const std::string& data);
};