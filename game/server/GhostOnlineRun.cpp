#include "cbase.h"
#include "GhostOnlineRun.h"
#include "GhostOnlineEngine.h"


#include "tier0\memdbgon.h"



GhostOnlineRun::GhostOnlineRun(const char* name, GhostUtils::GhostData data) {
	ghostData = data;
	Q_strcpy(ghostName, name);
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
	//TODO GhostHud::hud()->RemoveGhost((size_t)this);
	GhostOnlineEngine::getEngine()->EndRun(this);

}

