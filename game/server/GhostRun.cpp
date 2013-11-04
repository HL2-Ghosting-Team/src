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


GhostRun::GhostRun(){ent = NULL;}

GhostRun::~GhostRun(void){}

bool GhostRun::openRun(const char* fileName) {
	if (!fileName) return false;//this is just incase
	char file[256];
	Q_strcpy(file, fileName);
	V_SetExtension(file, ".run", sizeof(file));
	FileHandle_t myFile = filesystem->Open(file, "rb", "MOD");
	RunData.clear();
	unsigned char firstByte;
	if (myFile == NULL) {
		Msg("File is null!\n");
		return false;
	}
	filesystem->Read(&firstByte, sizeof(firstByte), myFile);
	if (firstByte == 0xAF) {
		Msg("File is uncompressed!\n");
	} //TODO add the compressed byte check
	else {
		Msg("File is malformed!\n");
		return false;
	}
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
	Q_strcpy(ghostName, RunData[0].name);
	filesystem->Close(myFile);
	return true;
}

void GhostRun::ResetGhost() {
	if (inReset) return;//if we've been reset by the user, ignore us
	GhostEntity* tempGhost = dynamic_cast<GhostEntity*>(CreateEntityByName("ghost_entity"));
	if(DispatchSpawn(tempGhost) == 0) {
		tempGhost->SetRunData(RunData);
		tempGhost->step = step;
		tempGhost->SetGhostName(ghostName);
		tempGhost->startTime = startTime;
		tempGhost->StartRun();
		ent = tempGhost;
	}
}

void GhostRun::StartRun() {
	GhostEntity * entity = (GhostEntity*) CreateEntityByName( "ghost_entity" );
	if (entity) {
		entity->SetGhostName(ghostName);
		entity->SetRunData(RunData);
		entity->SetAbsOrigin(Vector(RunData[1].x, RunData[1].y, RunData[1].z));
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
	RunData.clear();
	GhostEngine::getEngine().EndRun(this);
}
