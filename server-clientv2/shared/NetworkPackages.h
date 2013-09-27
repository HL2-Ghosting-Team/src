#pragma once
#include <iostream>
#include <list>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <sstream>
#define BYTE unsigned char

#define SV_PINGPACKAGE				1	// 00000001b
#define SV_CONNECTACKPACKAGE		2	// 00000002b
#define SV_RACESTARTPACKAGE			3	// 00000003b
#define SV_PLAYERUPDATEPACKAGE		4	// 00000004b

#define CL_PONGPACKAGE				129	// 10000001b
#define CL_CONNECTPACKAGE			130	// 10000002b
#define CL_MAPCHANGEPACKAGE			131	// 10000003b
#define CL_MODELCHANGEPACKAGE		132	// 10000004b
#define CL_PLAYERUPDATEPACKAGE		133	// 10000005b

#ifdef _DEBUG
#define inArchiveType boost::archive::text_iarchive
#define outArchiveType boost::archive::text_oarchive
#define streamflags std::ios_base::in | std::ios_base::out
#else
#define inArchiveType boost::archive::binary_iarchive
#define outArchiveType boost::archive::binary_oarchive
#define streamflags std::ios_base::in | std::ios_base::out | std::ios_base::binary
#endif

struct messageinfo
{
	char packagetype;
	int tick;
};

class NetworkPackage
{
private:
    friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & packagetype;
		ar & thetick;
	}
public:
	char packagetype;
	int thetick;
	NetworkPackage(void)
	{
	}
	virtual ~NetworkPackage(void)
	{
	}
};

//--------------------------------------------------------------------------------------------------
// Server -> Client packages
//--------------------------------------------------------------------------------------------------
// Ping package
class sv_PingPackage : public NetworkPackage
{
public:
	sv_PingPackage(int tick) { packagetype = SV_PINGPACKAGE; thetick = tick; }
	sv_PingPackage() { packagetype = SV_PINGPACKAGE; }
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<NetworkPackage>(*this);
	}
};

// Connect ack package
class sv_ConnectAckPackage : public NetworkPackage
{
public:
	sv_ConnectAckPackage(int tick, bool conn=false) { isConnected = conn; packagetype = SV_CONNECTACKPACKAGE; thetick = tick; }
	sv_ConnectAckPackage() { packagetype = SV_CONNECTACKPACKAGE; }
	bool isConnected;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<NetworkPackage>(*this);
		ar & isConnected;
	}
};

class PlayerInfo
{
public:
	std::string ip;
	std::string name;
	std::string model;
private:
	friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & ip;
        ar & name;
        ar & model;
    }
};
// Start race package
class sv_StartRacePackage : public NetworkPackage
{
public:
	sv_StartRacePackage() { packagetype = SV_RACESTARTPACKAGE; }
	sv_StartRacePackage(int tick, const std::list<PlayerInfo>& players) { theplayers = players; packagetype = SV_RACESTARTPACKAGE; thetick = tick; }
	std::list<PlayerInfo> theplayers;
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<NetworkPackage>(*this);
		ar & theplayers;
	}
};

class PlayerData
{
public:
	std::string ip;
	float x;
	float y;
	float z;

	float vx;
	float vy;
	float vz;
private:
    friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & ip;
        ar & x;
        ar & y;
        ar & z;
        ar & vx;
        ar & vy;
        ar & vz;
    }
};

// Player update package
class sv_PlayerUpdatePackage : public NetworkPackage
{
public:
	sv_PlayerUpdatePackage() { packagetype = SV_PLAYERUPDATEPACKAGE; }
	sv_PlayerUpdatePackage(int tick, const std::list<PlayerData>& infos) { theinfos = infos; packagetype = SV_PLAYERUPDATEPACKAGE; thetick = tick; }
	std::list<PlayerData> theinfos;
private:
	friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & boost::serialization::base_object<NetworkPackage>(*this);
		ar & theinfos;
	}
};

//--------------------------------------------------------------------------------------------------
// Client -> Server packages
//--------------------------------------------------------------------------------------------------
// Pong package
class cl_PongPackage : public NetworkPackage
{
public:
	friend class boost::serialization::access;
	cl_PongPackage() { packagetype = CL_PONGPACKAGE; }
	cl_PongPackage(int tick) { packagetype = CL_PONGPACKAGE; thetick = tick; }
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<NetworkPackage>(*this);
	}
};

// Connect package
class cl_ConnectPackage : public NetworkPackage
{
public:
	friend class boost::serialization::access;
	cl_ConnectPackage() { packagetype = CL_CONNECTPACKAGE; }
	cl_ConnectPackage(int tick, std::string name, std::string password) { thepassword = password; thename = name; packagetype = CL_CONNECTPACKAGE; thetick = tick; }
	std::string thepassword;
	std::string thename;
private:
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<NetworkPackage>(*this);
		ar & thename;
		ar & thepassword;
	}
};

// Map change package
class cl_MapChangePackage : public NetworkPackage
{
public:
	cl_MapChangePackage() { packagetype = CL_MAPCHANGEPACKAGE; }
	cl_MapChangePackage(int tick, std::string map) { themap = map; packagetype = CL_MAPCHANGEPACKAGE; thetick = tick; }
	std::string themap;
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<NetworkPackage>(*this);
		ar & themap;
	}
};

// Model & name change package
class cl_ModelChangePackage : public NetworkPackage
{
public:
	cl_ModelChangePackage() { packagetype = CL_MODELCHANGEPACKAGE; }
	cl_ModelChangePackage(int tick, std::string model) { themodel = model; packagetype = CL_MODELCHANGEPACKAGE; thetick = tick; }
	std::string themodel;
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<NetworkPackage>(*this);
		ar & themodel;
	}
};

// position+velocity update data
class cl_UpdatePackage : public NetworkPackage
{
public:
	cl_UpdatePackage() { packagetype = CL_PLAYERUPDATEPACKAGE; }
	cl_UpdatePackage(int tick, PlayerData info) { theinfo = info; packagetype = CL_PLAYERUPDATEPACKAGE; thetick = tick; }
	PlayerData theinfo;
private:
	friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & boost::serialization::base_object<NetworkPackage>(*this);
		ar & theinfo;
	}
};