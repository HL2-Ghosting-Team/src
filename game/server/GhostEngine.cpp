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
		//instance->addListener();
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
	//if ( Q_strcmp(event->GetName(), "game_newmap") == 0 ) {
	if ( Q_strcmp(event->GetName(), "game_init") == 0 ) {
		//Msg("The map %s has been loaded!\n", gpGlobals->mapname );
		Msg("The player has respawned!\n");
		ResetGhosts();
	}
}

void GhostEngine::addListener() {
	gameeventmanager->AddListener(this, "game_init", false);
}

void GhostEngine::transferGhostData() {
	for (int i = 0; i < ghosts.size(); i++) {
		GhostRun * it = ghosts[i];
		it->step = it->ent->step;
		Msg("Transferring step %i\n", it->ent->step);
		it->startTime = it->ent->startTime;
		Msg("Transferring startTime %f\n", it->ent->startTime);
	}
}

GhostRun* GhostEngine::getRun(GhostEntity* toGet) {
	for (int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		if (strcmp(it->ent->GetGhostName(), toGet->GetGhostName()) == 0) {
			return it;
		}
	}
	return NULL;
}

void GhostEngine::ResetGhosts() {
	Msg("Resetting ghosts!\n");
	for (int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		Msg("Resetting ghost: %s\n", it->ghostName);
		it->ResetGhost();
	}
}

//Is the engine supporting any ghosts right now?
bool GhostEngine::isActive() {
	return !ghosts.empty();
}

//called after the run kills itself, >>should be the last thing called!!<<
//For example, the order of EndRun() s to be called goes GhostEntity -> GhostRun -> GhostEngine
//Although, keep in mind that all GhostEntity's EndRun does is remove it from the game, and calling
//GhostRun's EndRun will call its GhostEntity's EndRun.
void GhostEngine::EndRun(GhostRun* run) {
	if (run) {
		ghosts.erase(std::find(ghosts.begin(), ghosts.end(), run));
	}
}

void GhostEngine::StartRun(const char* fileName) {
	GhostRun * run = new GhostRun();
	if (run->openRun(fileName)) {
		run->StartRun();
		ghosts.push_back(run);
	}
}

void startRun_f (const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (args.Arg(1) != "")) {
		GhostEngine::getEngine().StartRun(args.Arg(1));
	}
}

ConCommand start("gh_play", startRun_f, "Play back a run you did.", 0);