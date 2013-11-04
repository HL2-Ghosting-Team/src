//===== Copyright © 1996-2008, Valve Corporation, All rights reserved. ======//
//
// Purpose: Records your positions into a .run file
//
// $NoKeywords: $
//
//===========================================================================//

#include <stdio.h>
#include <iomanip>
#include <sstream>
#include "filesystem.h"
#include <iostream>
#include <fstream>
#include <string>
#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "eiface.h"
#include "igameevents.h"
#include "convar.h"
#include "Color.h"
#include "vstdlib/random.h"
#include "engine/IEngineTrace.h"
#include "tier2/tier2.h"
#include "game/server/iplayerinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
IVEngineServer	*engine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IGameEventManager *gameeventmanager = NULL; // game events interface
IPlayerInfoManager *playerinfomanager = NULL; // game dll interface to interact with players
IServerPluginHelpers *helpers = NULL; // special 3rd party plugin helpers from the engine
IUniformRandomStream *randomStr = NULL;
IEngineTrace *enginetrace = NULL;
IFileSystem* filesystem = NULL;
CGlobalVars *gpGlobals = NULL;
edict_t *currPlayer = NULL;
IPlayerInfo* playerInfo = NULL;
IServerGameEnts * serverGameEnts = NULL;

static ConVar ghName("gh_name", "Ghost", FCVAR_ARCHIVE | FCVAR_REPLICATED | FCVAR_DEMO, "Sets the name of your ghost.\nThis can also be used as the base name of your files!");

float nextTime = 0.00;
float startTime = 0.00;
const char* fileName;
const char* playerName = NULL;
bool shouldRecord = false;
bool firstTime = true;
FileHandle_t myFile = NULL;

//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class GhostingRecord: public IServerPluginCallbacks, public IGameEventListener
{
public:
	GhostingRecord();
	~GhostingRecord();

	// IServerPluginCallbacks methods
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload( void );
	virtual void			Pause( void );
	virtual void			UnPause( void );
	virtual const char     *GetPluginDescription( void );      
	virtual void			LevelInit( char const *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			GameFrame( bool simulating );
	virtual void			LevelShutdown( void );
	virtual void			ClientActive( edict_t *pEntity );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername );
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );

	// added with version 3 of the interface.
	virtual void			OnEdictAllocated( edict_t *edict );
	virtual void			OnEdictFreed( const edict_t *edict  );	

	// IGameEventListener Interface
	virtual void FireGameEvent( KeyValues * event );

	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

private:
	int m_iClientCommandIndex;

	void record( const CCommand &args);
};


// 
// The plugin is a static singleton that is exported as an interface
//
GhostingRecord g_EmtpyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(GhostingRecord, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_EmtpyServerPlugin );

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
GhostingRecord::GhostingRecord(){m_iClientCommandIndex = 0;}

GhostingRecord::~GhostingRecord(){}
//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool GhostingRecord::Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	ConnectTier1Libraries( &interfaceFactory, 1 );
	ConnectTier2Libraries( &interfaceFactory, 1 );

	playerinfomanager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	if ( !playerinfomanager )
	{
		Warning( "Unable to load playerinfomanager, ignoring\n" ); // this isn't fatal, we just won't be able to access specific player data
	}
	engine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	gameeventmanager = (IGameEventManager *)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER,NULL);
	helpers = (IServerPluginHelpers*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);
	enginetrace = (IEngineTrace *)interfaceFactory(INTERFACEVERSION_ENGINETRACE_SERVER,NULL);
	randomStr = (IUniformRandomStream *)interfaceFactory(VENGINE_SERVER_RANDOM_INTERFACE_VERSION, NULL);
	filesystem = (IFileSystem *)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	serverGameEnts = (IServerGameEnts*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER, NULL);

	// get the interfaces we want to use
	if(	! ( engine && gameeventmanager && g_pFullFileSystem && helpers && enginetrace && randomStr && filesystem && serverGameEnts ) ){
		return false; // we require all these interface to function
	}
	if ( playerinfomanager ){
		gpGlobals = playerinfomanager->GetGlobalVars();
	}
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void GhostingRecord::Unload( void )
{
	gameeventmanager->RemoveListener( this ); // make sure we are unloaded from the event system
	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
}

void writeLine(const char* map, const char* name, float ti, float x, float y, float z) {
	if (!myFile) return;
	unsigned char mapLength = strlen(map);
	//Msg("Map length: %i\n", mapLength);
	unsigned char nameLength = strlen(name);
	//Msg("Name length: %i\n", nameLength);
	filesystem->Write((void*)&mapLength, sizeof(mapLength), myFile);
	filesystem->Write((void*)map, mapLength, myFile);//map
	filesystem->Write((void*)&nameLength, sizeof(nameLength), myFile);
	filesystem->Write((void*)name, nameLength, myFile);
	filesystem->Write(&ti, sizeof(ti), myFile);//time
	filesystem->Write(&x, sizeof(x), myFile);//x
	filesystem->Write(&y, sizeof(y), myFile);//y
	filesystem->Write(&z, sizeof(z), myFile);//z
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void GhostingRecord::LevelInit( char const *pMapName )
{
	gameeventmanager->AddListener( this, true );
	if (shouldRecord) {
		float time = ((float)Plat_FloatTime()) - startTime;
		writeLine(pMapName, "empty", time, 0.0f, 0.0f, 0.0f);
		//filesystem->FPrintf(myFile, "%s %s %s %f %f %f %f\n", "GHOSTING", pMapName, "empty", time, 0.0f, 0.0f, 0.0f);
		//filesystem->Flush(myFile);
	}
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void GhostingRecord::GameFrame( bool simulating ) {

	if (shouldRecord) {
		if (playerInfo == NULL) {
			//Msg("Player info null! Getting player info...\n");
			for (int i = 1; i < gpGlobals->maxEntities; i++) {
				currPlayer = engine->PEntityOfEntIndex(i);
				playerInfo = playerinfomanager->GetPlayerInfo(currPlayer);
				if (playerInfo != NULL/* && playerInfo->IsConnected()*/) {
					//Msg("Found valid playerInfo: %s\n", playerInfo->GetName());
					break;
				}
			}
		}
		if (currPlayer != NULL && playerInfo != NULL) {
			//CBaseEntity *playerEntity; FIND EYEPOS
			//playerEntity = serverGameEnts->EdictToBaseEntity(currPlayer); FIND EYEPOS
			float time = (((float)Plat_FloatTime()) - startTime);
			//TODO: Find eye pos
			Vector loc = playerInfo->GetAbsOrigin();
			if( firstTime) {
				unsigned char first = 0xAF;
				filesystem->Write(&first, sizeof(first), myFile); 
				writeLine(STRING(gpGlobals->mapname), ghName.GetString(), -1.0f, 0.0f, 0.0f, 0.0f); 
				firstTime = false;
			}
			if ( time >= nextTime ) {//see if we should update again
				writeLine(STRING(gpGlobals->mapname), ghName.GetString(), time, loc.x, loc.y, loc.z); 
				nextTime = time + 0.04f;//~20 times a second, the more there is, the smoother it'll be
			}
		}
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void GhostingRecord::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	playerInfo = NULL;
	gameeventmanager->RemoveListener( this );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------

CON_COMMAND ( gh_stop, "Stop recording.") {
	shouldRecord = false;
	if (myFile) {
		Msg("Stopping recording\n");
		filesystem->Close(myFile);
		//filesystem->Close(myFile);
	}
	myFile = NULL;
}

int GetFileCount(const char *searchkey)
{
	std::stringstream sstr;
	sstr << searchkey << "_*.run";
	int toReturn = 0;
	FileFindHandle_t findHandle; // note: FileFINDHandle
	const char *pFilename = filesystem->FindFirstEx(sstr.str().c_str(), "MOD", &findHandle );
	for(int i = 0; pFilename; i++) {
		pFilename = filesystem->FindNext(findHandle);
		toReturn = i + 1;
	}
	filesystem->FindClose(findHandle);
	return toReturn; // number of entries
}

void record(const CCommand &args) {
	if (shouldRecord) {
		Msg("Already recording!\n");
		return;
	}
	std::string test = std::string(ghName.GetString());
	while (test.find(' ') != std::string::npos) {
		playerName = test.replace(test.find(' '), 1, "").c_str();
		ghName.SetValue(playerName);
	}
	playerName = ghName.GetString();
	if (args.ArgC() > 1 && args.Arg(1) != "") {
		fileName = args.Arg(1);//specified name
	} else {
		std::stringstream sstr;
		int count = GetFileCount(playerName);
		char fileish[MAX_PATH];
		std::sprintf(fileish, "%03d", (count + 1));
		sstr << playerName << "_" << fileish;
		fileName = sstr.str().c_str();
	}
	char fileName2[256];
	Q_strcpy(fileName2, fileName);
	V_SetExtension(fileName2, ".run", sizeof(fileName2));
	if (!(filesystem->FileExists(fileName2, "MOD"))) {
		Msg("Recording to %s...\n", fileName2);
		myFile = filesystem->Open(fileName2, "w+b", "MOD");
		shouldRecord = true;
		firstTime = true;
		startTime = (float) Plat_FloatTime();
		nextTime = 0.0f;
	} else {
		Msg("File aready exists!\n");
	}
}

ConCommand rec( "gh_record", record, "Records a run.", 0);


//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *GhostingRecord::GetPluginDescription( void ){return "Ghosting Plugin by Gocnak";}
//Unused
void GhostingRecord::ClientActive( edict_t *pEntity ){}
void GhostingRecord::ClientDisconnect( edict_t *pEntity ){}
void GhostingRecord::ClientPutInServer( edict_t *pEntity, char const *playername ){}
void GhostingRecord::SetCommandClient( int index ){m_iClientCommandIndex = index;}
void ClientPrint( edict_t *pEdict, char *format, ... ){}
void GhostingRecord::ClientSettingsChanged( edict_t *pEdict ){}
PLUGIN_RESULT GhostingRecord::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ){return PLUGIN_CONTINUE;}
PLUGIN_RESULT GhostingRecord::ClientCommand( edict_t *pEntity, const CCommand &args ){return PLUGIN_CONTINUE;}
PLUGIN_RESULT GhostingRecord::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID ){return PLUGIN_CONTINUE;}
void GhostingRecord::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue ){}
void GhostingRecord::OnEdictAllocated(edict_t *edict ){}
void GhostingRecord::OnEdictFreed(const edict_t *edict){}
void GhostingRecord::FireGameEvent(KeyValues* event){}
void GhostingRecord::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ){}
void GhostingRecord::Pause( void ){}
void GhostingRecord::UnPause( void ){}