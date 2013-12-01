//===== Copyright © 1996-2008, Valve Corporation, All rights reserved. ======//
//
// Purpose: Records your positions into a .run file
//
// $NoKeywords: $
//
//===========================================================================//
#define GAME_DLL 1
#include "cbase.h"
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
#include "tier2/tier2.h"
#include "game/server/iplayerinfo.h"
#include "GhostEngine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
IVEngineServer	*engine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IPlayerInfoManager *playerinfomanager = NULL; // game dll interface to interact with players
IFileSystem* filesystem = NULL;
CGlobalVars *gpGlobals = NULL;
edict_t *currPlayer = NULL;
IPlayerInfo* playerInfo = NULL;
IServerGameEnts * serverGameEnts = NULL;

float nextTime = 0.00f;
float startTime = 0.00f;
char mapName[32];
bool shouldRecord = false;
bool mapNameDirty = true;
bool playerNameDirty = true;
bool firstTime = true;
FileHandle_t myFile = NULL;
std::vector<unsigned char> ghostColor;
std::vector<unsigned char> trailColor;

static void onNameChange(IConVar *var, const char* pOldValue, float fOldValue) {
	if(Q_strcmp(((ConVar*)var)->GetString(), pOldValue) != 0) { 
		playerNameDirty = true;
	}
}
static ConVar ghName("gh_name", "Ghost", FCVAR_ARCHIVE | FCVAR_REPLICATED | FCVAR_DEMO, "Sets the name of your ghost.\nThis can also be used as the base name of your files!", onNameChange);


//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class GhostingRecord: public IServerPluginCallbacks
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
	engine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	filesystem = (IFileSystem *)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	serverGameEnts = (IServerGameEnts*)gameServerFactory(INTERFACEVERSION_SERVERGAMEENTS, NULL);
	// get the interfaces we want to use
	if(	! ( engine && g_pFullFileSystem && filesystem && serverGameEnts && playerinfomanager ) ){
		return false; // we require all these interface to function
	}
	gpGlobals = playerinfomanager->GetGlobalVars();
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void GhostingRecord::Unload( void )
{
	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
}

void writeLine(const char* map, const char* name, float ti, float x, float y, float z) {
	if (!myFile) return;
	unsigned char mapLength = strlen(map);
	unsigned char nameLength = strlen(name);
	filesystem->Write((void*)&mapLength, sizeof(mapLength), myFile);
	filesystem->Write((void*)map, mapLength, myFile);//map
	filesystem->Write((void*)&nameLength, sizeof(nameLength), myFile);
	filesystem->Write((void*)name, nameLength, myFile);
	filesystem->Write(&ti, sizeof(ti), myFile);//time
	filesystem->Write(&x, sizeof(x), myFile);//x
	filesystem->Write(&y, sizeof(y), myFile);//y
	filesystem->Write(&z, sizeof(z), myFile);//z
}

void writeHeader() {
	if (!myFile) return;
	unsigned char first = 0xAF;
	filesystem->Write(&first, sizeof(first), myFile); 
	unsigned char gt = gpGlobals->ghostType;
	filesystem->Write(&gt, sizeof(gt), myFile);//ghost type
	unsigned char gr = gpGlobals->ghostRed;
	filesystem->Write(&gr, sizeof(gr), myFile);//ghost red
	unsigned char gg = gpGlobals->ghostGreen;
	filesystem->Write(&gg, sizeof(gg), myFile);//ghost green
	unsigned char gb = gpGlobals->ghostBlue;
	filesystem->Write(&gb, sizeof(gb), myFile);//ghost blue
	unsigned char tr = gpGlobals->trailRed;
	filesystem->Write(&tr, sizeof(tr), myFile);//trail red
	unsigned char tg = gpGlobals->trailGreen;
	filesystem->Write(&tg, sizeof(tg), myFile);//trail green
	unsigned char tb = gpGlobals->trailBlue;
	filesystem->Write(&tb, sizeof(tb), myFile);//trail blue
	unsigned char tl = gpGlobals->trailLength;
	filesystem->Write(&tl, sizeof(tl), myFile);//trail length
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void GhostingRecord::LevelInit( char const *pMapName )
{
	if (shouldRecord) {
		mapNameDirty = true;
	}
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void GhostingRecord::GameFrame( bool simulating ) {
	if (shouldRecord) {
		if (playerInfo == NULL) {
			for (int i = 1; i < gpGlobals->maxEntities; i++) {
				currPlayer = engine->PEntityOfEntIndex(i);
				playerInfo = playerinfomanager->GetPlayerInfo(currPlayer);
				if (playerInfo != NULL) {
					break;
				}
			}
		}
		if (currPlayer != NULL && playerInfo != NULL) {
			CBaseEntity *playerEntity = serverGameEnts->EdictToBaseEntity(currPlayer);
			if (playerEntity) {
				float time = (((float)Plat_FloatTime()) - startTime);
				Vector loc = playerEntity->EyePosition();
				if( firstTime) {
					writeHeader();
					firstTime = false;
				}
				if ( time >= nextTime ) {//see if we should update again
					const char* mapToWrite = "";
					const char* playerToWrite = "";
					if (mapNameDirty) {
						if(Q_strcmp(mapName, STRING(gpGlobals->mapname)) != 0) {
							mapToWrite = STRING(gpGlobals->mapname);
							Q_strcpy(mapName, mapToWrite);
						}
						mapNameDirty = false;
					}
					if (playerNameDirty) {
						playerToWrite = ghName.GetString();
						playerNameDirty = false;
					}
					writeLine(mapToWrite, playerToWrite, time, loc.x, loc.y, loc.z); 
					nextTime = time + 0.04f;//~20 times a second, the more there is, the smoother it'll be
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void GhostingRecord::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	currPlayer = NULL;
	playerInfo = NULL;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------

CON_COMMAND ( gh_stop, "Stop recording.") {
	shouldRecord = false;
	if (myFile) {
		Msg("Stopping recording\n");
		filesystem->Close(myFile);
	}
	mapName[0] = 0;
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
	char playerName[256];
	char fileName[256];
	std::string test = std::string(ghName.GetString());
	while (test.find(' ') != std::string::npos) {
		Q_strcpy(playerName, test.replace(test.find(' '), 1, "").c_str());
		ghName.SetValue(playerName);
	}
	Q_strcpy(playerName, ghName.GetString());
	if (args.ArgC() > 1 && (Q_strcmp(args.Arg(1), "") != 0)) {
		Q_strcpy(fileName, args.Arg(1));//specified name
	} else {
		std::stringstream sstr;
		int count = GetFileCount(playerName);
		char fileish[MAX_PATH];
		std::sprintf(fileish, "%03d", (count + 1));
		sstr << playerName << "_" << fileish;
		Q_strcpy(fileName, sstr.str().c_str());
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
		mapNameDirty = true;
		playerNameDirty = true;
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
void GhostingRecord::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ){}
void GhostingRecord::Pause( void ){}
void GhostingRecord::UnPause( void ){}