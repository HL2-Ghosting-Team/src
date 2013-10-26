//===== Copyright © 1996-2008, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#include <stdio.h>
#include <stdio.h>
#include <iomanip>
#include <sstream>
#include "filesystem.h"
#include <iostream>
#include <fstream>
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

// Uncomment this to compile the sample TF2 plugin code, note: most of this is duplicated in serverplugin_tony, but kept here for reference!
//#define SAMPLE_TF2_PLUGIN
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
edict_t *currPlayer;


float nextTime = 0.00;
float startTime = 0.00;
bool isSpawned = false;
const char* fileName;
bool shouldRecord = false;
bool firstTime = true;
//std::ofstream myFile;
FileHandle_t myFile;

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
GhostingRecord::GhostingRecord()
{
	m_iClientCommandIndex = 0;
}

GhostingRecord::~GhostingRecord()
{
}
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

	// get the interfaces we want to use
	if(	! ( engine && gameeventmanager && g_pFullFileSystem && helpers && enginetrace && randomStr && filesystem ) ){
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
	filesystem->Close(myFile);
	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void GhostingRecord::Pause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void GhostingRecord::UnPause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *GhostingRecord::GetPluginDescription( void )
{
	return "Ghosting Plugin, Gocnak";
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void GhostingRecord::LevelInit( char const *pMapName )
{
	//Msg( "Level \"%s\" has been loaded\n", pMapName );
	gameeventmanager->AddListener( this, true );
	if (shouldRecord) {
		float time = ((float)Plat_FloatTime()) - startTime;
		filesystem->FPrintf(myFile, "%s %s %s %f %f %f %f\n", "GHOSTING", pMapName, "empty", time, 0.0f, 0.0f, 0.0f);
		filesystem->Flush(myFile);
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void GhostingRecord::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ){}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void GhostingRecord::GameFrame( bool simulating )
{
	if (isSpawned == true) {
		if ( currPlayer != NULL && playerinfomanager ) 
		{
			const char *map = STRING ( gpGlobals->mapname );
			const char *point = strstr(map, "background");
			float time = (((float)Plat_FloatTime()) - startTime);
			if( point == NULL)//not in the menu silly
			{
				IPlayerInfo *info = playerinfomanager->GetPlayerInfo( currPlayer );
				if ( info != NULL ) {	
					if (shouldRecord) {
						if( firstTime) {
							filesystem->FPrintf(myFile, "%s %s %s %f %f %f %f\n", "GHOSTING", map, info->GetName(), -1, 0, 0, 0);
							filesystem->Flush(myFile);
							firstTime = false;
						}
						if ( time >= nextTime ) {//see if we should update again
							Vector loc = info->GetAbsOrigin();
							filesystem->FPrintf(myFile, "%s %s %s %f %f %f %f\n", "GHOSTING", map, info->GetName(),
								time, loc.x, loc.y, loc.z);
							filesystem->Flush(myFile);
							nextTime = time + 0.04f;//20 times a second
						}
					}
				}
			}
		} else {
			//NO PLAYER!?
		}
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void GhostingRecord::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	isSpawned = false;
	gameeventmanager->RemoveListener( this );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void GhostingRecord::ClientActive( edict_t *pEntity ){isSpawned = true;}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void GhostingRecord::ClientDisconnect( edict_t *pEntity ){isSpawned = false;}

//---------------------------------------------------------------------------------
// Purpose: called on 
//---------------------------------------------------------------------------------
void GhostingRecord::ClientPutInServer( edict_t *pEntity, char const *playername ){
	if (playername) currPlayer = pEntity;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void GhostingRecord::SetCommandClient( int index )
{
	m_iClientCommandIndex = index;
}

void ClientPrint( edict_t *pEdict, char *format, ... ){}
void GhostingRecord::ClientSettingsChanged( edict_t *pEdict ){}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT GhostingRecord::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	currPlayer = pEntity;
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT GhostingRecord::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	currPlayer = pEntity;
	return PLUGIN_CONTINUE;
}

PLUGIN_RESULT GhostingRecord::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID ){return PLUGIN_CONTINUE;}

void GhostingRecord::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue ){}
void GhostingRecord::OnEdictAllocated( edict_t *edict ){}
void GhostingRecord::OnEdictFreed( const edict_t *edict  ){}

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void GhostingRecord::FireGameEvent( KeyValues * event )
{
	const char * name = event->GetName();
	//TODO find event for player gaining control.
	//Msg( "GhostingRecord::FireGameEvent: Got event \"%s\"\n", name );
}

CON_COMMAND ( gh_stop, "Stop recording.") {
	Msg("Stopping recording\n");
	shouldRecord = false;
	filesystem->Close(myFile);
}

void record(const CCommand &args) {
	if ( args.ArgC() < 1 || args.Arg(1) == "" || shouldRecord || !currPlayer) {
		return;
	}
	//TODO catch a space in their name and kill it with fire
	//or just politely tell them to change their name, or deal with it somehow
	fileName = args.Arg(1);
	char fileName2[256];
	strcpy(fileName2, fileName);
	V_SetExtension(fileName2, ".run", sizeof(fileName2));
	//Msg("Filename: %s\n", fileName);
	//Msg("Filename2: %s\n", fileName2);
	if (strchr(fileName, '.') == NULL) {
		if (!filesystem->Open(fileName2, "r", "MOD")) {
			Msg("Recording to %s...\n", fileName2);
			myFile = filesystem->Open(fileName2, "w+", "MOD");
			shouldRecord = true;
			startTime = Plat_FloatTime();
		} else {
			Msg("File aready exists!\n");
		}
	} else {
		Msg("Usage: gh_record runname  || DO NOT INCLUDE EXTENSION!\n");
	}
}

ConCommand rec( "gh_record", record, "Records a run.", 0);