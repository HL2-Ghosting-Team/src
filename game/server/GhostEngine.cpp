#include "cbase.h"
#include "GhostEngine.h"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "tier0/memdbgon.h"

GhostEngine *GhostEngine::instance = NULL;

GhostEngine& GhostEngine::getEngine() {
	if (instance == NULL) {
		instance = new GhostEngine();
		instance->addListener();
	}	
	return *instance;
}

/*GhostEntity* GhostEngine::GetGhost(const char * name) {
	if (name) {
		for (int i = 0; i < ghosts.size(); i++) {
			GhostEntity * ent = ghosts[i];
			if (ent->GetGhostName() != NULL && (strcmp(name, ent->GetGhostName()) == 0)) {
				return ent;
			}
		}
	} else {
		return NULL;
	}
}*/
void GhostEngine::FireGameEvent( IGameEvent* event )
{
if ( Q_strcmp(event->GetName(), "game_newmap") == 0 )
	Msg("The map %s has been loaded!\n", event->GetString("mapname") );
}

void GhostEngine::addListener() {
	gameeventmanager->AddListener(this, "game_newmap", false);
}

void GhostEngine::ResetGhosts() {
	for (auto it = ghosts.begin(); it != ghosts.end(); ++it) {
		(*it)->StartRun();
	}
}

//Is the engine supporting any ghosts right now?
bool GhostEngine::isActive() {
	return !ghosts.empty();
}

//called after the run kills itself
void GhostEngine::EndRun(GhostEntity* run) {
	if (run) {
		ghosts.erase(std::find(ghosts.begin(), ghosts.end(), run));
	}
}


void GhostEngine::StartRun(const char* fileName) {
	GhostEntity * ent = (GhostEntity*) CreateEntityByName( "ghost_entity" );
	if (ent) {
		if (ent->openRun(fileName)) {
			ent->SetGhostName(ent->RunData[0].name);
			ent->SetAbsOrigin(Vector(ent->RunData[1].x, ent->RunData[1].y, ent->RunData[1].z));
			if (DispatchSpawn(ent) == 0) {
				Msg("Spawned the ent %s!\n", ent->GetGhostName());
				ent->startTime = gpGlobals->curtime;
				ent->StartRun();
			}
		}
	}
}



void startRun_f (const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (args.Arg(1) != "")) {
		GhostEngine::getEngine().StartRun(args.Arg(1));
	}
}

ConCommand start("gh_play", startRun_f, "Play back a run you did.", 0);