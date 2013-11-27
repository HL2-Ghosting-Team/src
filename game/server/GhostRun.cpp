#include "cbase.h"
#include "GhostRun.h"
#include <string>
#include <vector>
#include <fstream>
#include "stdio.h"
#include <sstream>
#include "GhostEngine.h"
#include "filesystem.h"
#include "tier0/memdbgon.h"
#include "timer.h"


GhostRun::GhostRun(){ent = NULL;}

GhostRun::~GhostRun(void){}

bool GhostRun::openRun(const char* fileName) {
	if (!fileName) return false;//this is just incase
	char file[256];
	Q_strcpy(file, fileName);
	V_SetExtension(file, ".run", sizeof(file));
	FileHandle_t myFile = filesystem->Open(file, "rb", "MOD");
	if (myFile == NULL) {
		Msg("File is null!\n");
		return false;
	}
	RunData.clear();
	//--------------------------------------HEADER ---------------------------------------------
	unsigned char firstByte;
	filesystem->Read(&firstByte, sizeof(firstByte), myFile);
	if (firstByte == 0xAF) {
		//Msg("File is uncompressed!\n");
	} //TODO add the compressed byte check
	else {
		Msg("File is malformed!\n");
		return false;
	}
	filesystem->Read(&typeGhost, sizeof(typeGhost), myFile); 
	filesystem->Read(&ghostRed, sizeof(ghostRed), myFile); 
	filesystem->Read(&ghostGreen, sizeof(ghostGreen), myFile); 
	filesystem->Read(&ghostBlue, sizeof(ghostBlue), myFile); 
	filesystem->Read(&trailRed, sizeof(trailRed), myFile); 
	filesystem->Read(&trailGreen, sizeof(trailGreen), myFile); 
	filesystem->Read(&trailBlue, sizeof(trailBlue), myFile); 
	filesystem->Read(&trailLength, sizeof(trailLength), myFile); 
	//-------------------------------------END HEADER -----------------------------------------
	while (!filesystem->EndOfFile(myFile)) {
		struct RunLine l;
		unsigned char mapNameLength;
		filesystem->Read((void*)&mapNameLength, sizeof(mapNameLength), myFile);
		char* mapName = new char[mapNameLength + 1];
		filesystem->Read((void*)mapName, mapNameLength, myFile);
		mapName[mapNameLength] = 0;
		unsigned char nameLength;
		filesystem->Read((void*)&nameLength, sizeof(nameLength), myFile);
		char* playerName = new char[nameLength + 1];
		filesystem->Read((void*)playerName, nameLength, myFile);
		playerName[nameLength] = 0;
		Q_strcpy(l.map, mapName);
		Q_strcpy(l.name, playerName);
		filesystem->Read(&l.tim, sizeof(l.tim), myFile);//time
		filesystem->Read(&l.x, sizeof(l.x), myFile);//x
		filesystem->Read(&l.y, sizeof(l.y), myFile);//y
		filesystem->Read(&l.z, sizeof(l.z), myFile);//z
		RunData.push_back(l);
		delete[] mapName;
		delete[] playerName;
	}
	Q_strcpy(currentMap, RunData[0].map);
	Q_strcpy(ghostName, RunData[0].name);
	BlaTimer::timer()->AddGhost((size_t)this, ghostName, currentMap);
	filesystem->Close(myFile);
	return true;
}

void GhostRun::ResetGhost() {
	if (inReset) return;//if we've been reset by the user, ignore us
	GhostEntity* tempGhost = static_cast<GhostEntity*>(CreateEntityByName("ghost_entity"));
	tempGhost->typeGhost = typeGhost;
	tempGhost->ghostRed = ghostRed;
	tempGhost->ghostGreen = ghostGreen;
	tempGhost->ghostBlue = ghostBlue;
	tempGhost->trailRed = trailRed;
	tempGhost->trailGreen = trailGreen;
	tempGhost->trailBlue = trailBlue;
	tempGhost->trailLength = trailLength;
	tempGhost->SetRunData(RunData);
	tempGhost->step = step;
	tempGhost->SetGhostName(ghostName);
	Q_strcpy(tempGhost->currentMap, currentMap);
	if(DispatchSpawn(tempGhost) == 0) {
		tempGhost->startTime = startTime;
		tempGhost->StartRun();
		ent = tempGhost;
	}
}

void GhostRun::StartRun() {
	GhostEntity * entity = static_cast<GhostEntity*>(CreateEntityByName("ghost_entity"));
	if (entity) {
		entity->typeGhost = typeGhost;
		entity->ghostRed = ghostRed;
		entity->ghostGreen = ghostGreen;
		entity->ghostBlue = ghostBlue;
		entity->trailRed = trailRed;
		entity->trailGreen = trailGreen;
		entity->trailBlue = trailBlue;
		entity->trailLength = trailLength;
		entity->SetGhostName(ghostName);
		entity->SetRunData(RunData);
		Q_strcpy(entity->currentMap, currentMap);
		entity->SetAbsOrigin(Vector(RunData[0].x, RunData[0].y, RunData[0].z));
		if (DispatchSpawn(entity) == 0) {
			entity->startTime = (float) Plat_FloatTime();
			entity->step = 0;
			entity->StartRun();
			ent = entity;
		}
	}
}

//Ends the run and clears data.
//>>CALLS THE ENT's ENDRUN()!<<
//This should only be called when gh_stop_all_ghosts is called
//OR if the run actually finished.
void GhostRun::EndRun() {
	if (ent) {
		ent->clearRunData();
		ent->EndRun(false);
	}
	BlaTimer::timer()->RemoveGhost((size_t)this);
	RunData.clear();
	GhostEngine::getEngine()->EndRun(this);
}