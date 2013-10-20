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

void GhostEngine::transferGhostData() {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun * it = ghosts[i];
		it->step = it->ent->step;
		it->startTime = it->ent->startTime;
	}
}

GhostRun* GhostEngine::getRun(GhostEntity* toGet) {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		if (Q_strcmp(it->ent->GetGhostName(), toGet->GetGhostName()) == 0) {
			return it;
		}
	}
	return NULL;
}

void GhostEngine::ResetGhosts() {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
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
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (Q_strcmp(args.Arg(1), "") != 0)) {
		GhostEngine::getEngine().StartRun(args.Arg(1));
	}
}

static int FileAutoComplete ( char const *partial, 
char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	char* fileDir;
	if (UTIL_GetModDir(fileDir, 256)) {

	}
	strcpy( commands[0], "hello" );
	strcpy( commands[1], "goodbye" );
	return 2; // number of entries
}

ConCommand start("gh_play", startRun_f, "Play back a run you did.", 0/*, FileAutoComplete*/);