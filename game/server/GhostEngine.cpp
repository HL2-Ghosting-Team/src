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




void GhostEngine::Loop() {
	if (ghosts.empty()) return;
	if (!gpGlobals) return;
	for (auto it = ghosts.begin(); it != ghosts.end(); ++it) {
		if (!(*it)->isRunActive()) {
			
		}
	}
}

void GhostEngine::ResetGhosts() {
	for (auto it = ghosts.begin(); it != ghosts.end(); ++it) {
		(*it)->setRunActive(false);
	}
}

//Is the engine supporting any ghosts right now?
bool GhostEngine::isActive() {
	return !ghosts.empty();
}

//called after the run kills itself
void GhostEngine::EndRun(GhostRun* run) {
	if (run) {
		ghosts.erase(std::find(ghosts.begin(), ghosts.end(), run));
	}
}


void GhostEngine::StartRun(const char* fileName) {
	GhostRun* gh = new GhostRun();
	if (gh->open(fileName)) {
		gh->StartRun();
		ghosts.push_back(gh);
	}
}



void startRun_f (const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (args.Arg(1) != "")) {
		GhostEngine::getEngine().StartRun(args.Arg(1));
	}
}

ConCommand start("gh_play", startRun_f, "Play back a run you did.", 0);