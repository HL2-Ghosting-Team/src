#include "Client.hpp"
#include "shared\NetworkPackages.h"
#include <iostream>
#include <list>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <sstream>


Client::Client(std::string name)
{
	currenttick = 0;
	myName = name;
	// TODO: gotta set IP...
}


Client::~Client(void)
{
}


void Client::ReceiveMessage(const std::string& data)
{
	messageinfo inf;
	memcpy(&inf,data.c_str(),sizeof(messageinfo));
	switch(inf.packagetype)
	{
	case SV_PINGPACKAGE:
		std::cout << "Ping received, sending pong." << std::endl;
		SendPongPackage();
		break;
	case SV_CONNECTACKPACKAGE:
		HandleConnectAckPackage(data);
		break;
	case SV_RACESTARTPACKAGE:
		HandleRaceStartPackage(data);
		break;
	case SV_PLAYERUPDATEPACKAGE:
		HandleUpdatePackage(data);
		break;
	}
}


void Client::HandleConnectAckPackage(const std::string& data)
{
	std::stringstream str(data,streamflags);
	inArchiveType ia(str);
	sv_ConnectAckPackage pack;
	ia >> pack;
	// TODO: actually handle the package (especially when disconnected)
	if(pack.isConnected)std::cout << "Connection established." << std::endl;
	else std::cout << "Disconnected." << std::endl;
}

void Client::HandleRaceStartPackage(const std::string& data)
{
	std::stringstream str(data,streamflags);
	inArchiveType ia(str);
	sv_StartRacePackage pack;
	ia >> pack;
	std::cout << "Race starting! Connected players:" << std::endl;
	for(auto it = pack.theplayers.begin(); it != pack.theplayers.end(); ++it)
	{
		std::cout << "Player " << it->name << "(IP: " << it->ip << ") with model " << it->model << std::endl;
		Ghost gh = { it->ip, it->name, it->model };
		ghosts[it->ip] = gh;
	}
	// TODO: actually handle it
}

void Client::HandleUpdatePackage(const std::string& data)
{
	std::stringstream str(data,streamflags);
	inArchiveType ia(str);
	sv_PlayerUpdatePackage pack;
	ia >> pack;
	std::cout << "Update tick! Player locations:" << std::endl;
	for(auto it = pack.theinfos.begin(); it != pack.theinfos.end(); ++it)
	{
		std::cout << "Player " << ghosts[it->ip].name << " at position (" << it->x << "," << it->y << "," << it->z << ")" << std::endl;
		ghosts[it->ip].SetPositionAndVelocity(it->x,it->y,it->z,it->vx,it->vy,it->vz);
	}
	// TODO: actually handle it
}


std::string Client::SendPongPackage()
{
	cl_PongPackage pack(currenttick);
	std::stringstream stream(streamflags);
	outArchiveType oa(stream);
	oa << pack;
	// TODO: send the package here
	// for example Send(stream.str(),stream.str().size())
	return stream.str();
}

std::string Client::SendConnectPackage(std::string password)
{
	cl_ConnectPackage pack(currenttick,myName,password);
	std::stringstream stream(streamflags);
	outArchiveType oa(stream);
	oa << pack;
	// TODO: send the package here
	// for example Send(stream.str(),stream.str().size())
	return stream.str();
}

std::string Client::SendMapChangePackage(std::string map)
{
	cl_MapChangePackage pack(currenttick,map);
	std::stringstream stream(streamflags);
	outArchiveType oa(stream);
	oa << pack;
	// TODO: send the package here
	// for example Send(stream.str(),stream.str().size())
	return stream.str();
}

std::string Client::SendModelChangePackage(std::string model)
{
	cl_ModelChangePackage pack(currenttick,model);
	std::stringstream stream(streamflags);
	outArchiveType oa(stream);
	oa << pack;
	// TODO: send the package here
	// for example Send(stream.str(),stream.str().size())
	return stream.str();
}

std::string Client::SendUpdatePackage(float x, float y, float z, float vx, float vy, float vz)
{
	PlayerData pd = {myIP,x,y,z,vx,vy,vz};
	cl_UpdatePackage pack(currenttick, pd);
	std::stringstream stream(streamflags);
	outArchiveType oa(stream);
	oa << pack;
	// TODO: send the package here
	// for example Send(stream.str(),stream.str().size())
	return stream.str();
}