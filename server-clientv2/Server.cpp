#include "Server.hpp"
#include "shared\NetworkPackages.h"
#include <string>

Server::Server(void)
{
	currenttick = 0;
	racestarted = false;
}


Server::~Server(void)
{
}

void Server::Tick()
{
	if(!racestarted)return;
	++currenttick;
	// Send updates to all clients
	std::list<PlayerData> players;
	for(auto it = ghosts.begin(); it != ghosts.end(); ++it)
	{
		PlayerData pl;
		pl.ip = it->second.ip;
		pl.x = it->second.x;
		pl.y = it->second.y;
		pl.z = it->second.z;
		pl.vx = it->second.vx;
		pl.vy = it->second.vy;
		pl.vz = it->second.vz;
		players.push_back(pl);
	}
	sv_PlayerUpdatePackage pack(currenttick,players);
	std::stringstream stream(streamflags);
	outArchiveType oa(stream);
	oa << pack;
}

void Server::ReceiveMessage(const std::string& senderip, const std::string& data)
{
	messageinfo inf;
	memcpy(&inf,data.c_str(),sizeof(messageinfo));
	switch(inf.packagetype)
	{
	case CL_PONGPACKAGE:
		std::cout << "Pong received, everything OK" << std::endl;
		HandlePongPackage(senderip, data);
		break;
	case CL_CONNECTPACKAGE:
		HandleConnectPackage(senderip, data);
		break;
	case CL_MODELCHANGEPACKAGE:
		HandleModelChangePackage(senderip, data);
		break;
	case CL_MAPCHANGEPACKAGE:
		HandleMapChangePackage(senderip, data);
		break;
	case CL_PLAYERUPDATEPACKAGE:
		HandleUpdatePackage(senderip, data);
		break;
	}
}

void Server::StartRace()
{
	std::list<PlayerInfo> players;
	racestarted = true;
	for(auto it = ghosts.begin(); it != ghosts.end(); ++it)
	{
		if(it->second.model != "" && it->second.name != "")
		{
			PlayerInfo pl = {it->second.ip, it->second.name, it->second.model};
			players.push_back(pl);
		}
		else
		{
			SendConnectAckPackage(it->second.ip,false);
		}
	}
	sv_StartRacePackage pack(currenttick, players);
	std::stringstream str(streamflags);
	outArchiveType oa(str);
	oa << pack;
	SendToAll(str.str());
}

// Client sent a connect request, check that and if everything is ok, add as a client
void Server::HandleConnectPackage(const std::string& senderip, const std::string& data)
{
	std::stringstream str(data,streamflags);
	inArchiveType ia(str);
	cl_ConnectPackage pack;
	ia >> pack;
	// If invalid data has been transmitted, byebye
	if(pack.thename == "")return;
	// Check if the client isnt connected and if he is, just ignore this.
	for(auto it = ghosts.begin(); it != ghosts.end(); ++it)
	{
		if(it->second.name == pack.thename) return;
	}
	if((pack.thepassword == password || password == "") && !racestarted)
	{
		// Add client
		Ghost tmp = {senderip,pack.thename,"",0};
		ghosts[senderip] = tmp;

		// Send connect ack
		SendConnectAckPackage(senderip,true);
	}
	else
	{
		SendConnectAckPackage(senderip,false);
		// remove from ghosts
		ghosts.erase(senderip);
	}
}

void Server::HandleMapChangePackage(const std::string& senderip, const std::string& data)
{
	std::stringstream str(data,streamflags);
	inArchiveType ia(str);
	cl_MapChangePackage pack;
	ia >> pack;

	if(ghosts.find(senderip) != ghosts.end())ghosts[senderip].map = pack.themap;
}

void Server::HandleModelChangePackage(const std::string& senderip, const std::string& data)
{
	std::stringstream str(data,streamflags);
	inArchiveType ia(str);
	cl_ModelChangePackage pack;
	ia >> pack;

	if(ghosts.find(senderip) != ghosts.end())ghosts[senderip].model = pack.themodel;
}

void Server::HandlePongPackage(const std::string& senderip, const std::string& data)
{
	// TODO: one day we can implement pongs...
}

void Server::HandleUpdatePackage(const std::string& senderip, const std::string& data)
{
	std::stringstream str(data,streamflags);
	inArchiveType ia(str);
	cl_UpdatePackage pack;
	ia >> pack;

	if(ghosts.find(senderip) != ghosts.end())
	ghosts[senderip].SetPositionAndVelocity(pack.theinfo.x,pack.theinfo.y,pack.theinfo.z,
											pack.theinfo.vx,pack.theinfo.vy,pack.theinfo.vz);
}


void Server::SendConnectAckPackage(const std::string& ip, bool conn)
{
	sv_ConnectAckPackage pack(currenttick,conn);
	std::stringstream stream(streamflags);
	outArchiveType oa(stream);
	oa << pack;
	// TODO: send the package here
	// for example Send(stream.str(),stream.str().size())
}

void Server::SendToAll(const std::string& data)
{
	for(auto it = ghosts.begin(); it != ghosts.end(); ++it)
	{
		// TODO: Send data to the client at the ip it->second.ip
	}
}