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
	ghostData.RunData.RemoveAll();
	if (GhostUtils::openRun(fileName, &ghostData)) {
		Q_strcpy(currentMap, ghostData.RunData[0].map);
		Q_strcpy(ghostName, ghostData.RunData[0].name);
		GhostHud::hud()->AddGhost((size_t)this, ghostName, currentMap);
		//filesystem->Close(myFile);
		return true;
	} else {
		Msg("Could not open run %s!\n", fileName);
		return false;
	}
}

//The "resetGhost" boolean is for level transitions, as the settings for the new ghost
//need to be updated. IT IS NOT RESTARTING THE RUN!
void GhostRun::StartRun(bool resetGhost) {
	GhostEntity * entity = static_cast<GhostEntity*>(CreateEntityByName("ghost_entity"));
	if (entity) {
		GhostUtils::copyGhostData(&ghostData, &entity->ghostData);
		entity->SetGhostName(ghostName);
		Q_strcpy(entity->currentMap, currentMap);
		if (resetGhost) {
			entity->step = step;
			if(DispatchSpawn(entity) == 0) {
				entity->startTime = startTime;
				entity->StartRun();
				ent = entity;
			}
		} else {
			entity->SetAbsOrigin(Vector(ghostData.RunData[0].x, ghostData.RunData[0].y, ghostData.RunData[0].z));
			if (DispatchSpawn(entity) == 0) {
				entity->startTime = (float) Plat_FloatTime();
				entity->step = 0;
				entity->StartRun();
				ent = entity;
			}
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
	ghostData.RunData.RemoveAll();
	GhostEngine::getEngine()->EndRun(this);
}