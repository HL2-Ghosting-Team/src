#include "cbase.h"
#include "GhostRun.h"
#include "GhostEngine.h"
#include "filesystem.h"
#include "ghosthud.h"
#include "GhostUtils.h"

#include "tier0/memdbgon.h"


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
	RunData.RemoveAll();
	//--------------------------------------HEADER ---------------------------------------------
	unsigned char firstByte;
	if (!GhostUtils::readHeader(filesystem, myFile, firstByte, typeGhost, ghostRed, ghostGreen, ghostBlue, trailRed, trailGreen, trailBlue, trailLength)) {
		return false;
	}
	//-------------------------------------END HEADER -----------------------------------------
	while (!filesystem->EndOfFile(myFile)) {
		struct RunLine l = GhostUtils::readLine(filesystem, myFile);
		RunData.AddToTail(l);
	}
	Q_strcpy(currentMap, RunData[0].map);
	Q_strcpy(ghostName, RunData[0].name);
	GhostHud::hud()->AddGhost((size_t)this, ghostName, currentMap);
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
		ent->EndRun();
	}
	GhostHud::hud()->RemoveGhost((size_t)this);
	RunData.RemoveAll();
	GhostEngine::getEngine()->EndRun(this);
}