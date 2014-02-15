#include "cbase.h"
#include "GhostOnlineRun.h"
#include "GhostOnlineEngine.h"
#include "ghosthud.h"


#include "tier0\memdbgon.h"



GhostOnlineRun::GhostOnlineRun(const char* name, GhostUtils::GhostData data) {
	ghostData = data;
	Q_strcpy(ghostName, name);//reset the name
	//TODO keep two names here, name for the hud and the name for comparison
	Q_strcpy(ghostNameHud, "OL ");
	Q_strcat(ghostNameHud, ghostName, 512);
	GhostHud::hud()->AddGhost((size_t)this, ghostNameHud, "Unknown");
	isStarted = false;
}

void GhostOnlineRun::StartRun() {
	GhostOnlineEntity *entity = static_cast<GhostOnlineEntity*>(CreateEntityByName("ghost_online_entity"));
	if (entity) {
		entity->ghostData = ghostData;
		entity->SetGhostName(ghostName);
		if (DispatchSpawn(entity) == 0) {
			isStarted = true;
			entity->StartRun();
			ent = entity;
		}
	}
}

void GhostOnlineRun::updateStep(RunLine newLine) {
	if (ent && ent->isActive) ent->updateStep(newLine);
}

//THEY disconnected
void GhostOnlineRun::EndRun() {
	if (ent) {
		ent->EndRun();
	}
	GhostHud::hud()->RemoveGhost((size_t)this);
	GhostOnlineEngine::getEngine()->EndRun(this);

}

