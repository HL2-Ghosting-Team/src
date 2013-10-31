#include "cbase.h"
#include "GhostEngine.h"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "tier0/memdbgon.h"
#include "filesystem.h"
#include "utlbuffer.h"

GhostEngine *GhostEngine::instance = NULL;

GhostEngine& GhostEngine::getEngine() {
	if (instance == NULL) {
		instance = new GhostEngine();
	}	
	return *instance;
}

//Called every level load to transfer the data of
//the last ghost, for continuity's sake.
void GhostEngine::transferGhostData() {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun * it = ghosts[i];
		Msg("Transferring ghost data for %s!\n", it->ghostName);
		if (!it->ent) {
			it->inReset = true;
			continue;
		}
		it->inReset = it->ent->inReset;
		//Msg("In reset? %s\n", (it->inReset ? "yes" : "no"));
		if (it->inReset) continue;
		it->step = it->ent->step;
		it->startTime = it->ent->startTime;
	}
}

GhostRun* GhostEngine::getRun(GhostEntity* toGet) {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		if (!it || !it->ent) continue;
		if (Q_strcmp(it->ent->GetGhostName(), toGet->GetGhostName()) == 0) {
			return it;
		}
	}
	return NULL;
}

//This is not to be mistaken for the resetAllGhosts, this is the
//handler for Level transitions, not resetting the run.
void GhostEngine::ResetGhosts() {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		//Msg("Attempting to reset ghost: %s...\n", it->ghostName);
		if (it) it->ResetGhost();
	}
}

//Is the engine supporting any ghosts right now?
bool GhostEngine::isActive() {
	return !ghosts.empty();
}

//Called after the run kills itself, >>should be the last thing called!!<<
//For example, the order of EndRun() s to be called goes GhostEntity -> GhostRun -> GhostEngine
//Although, keep in mind that all GhostEntity's EndRun does is remove it from the game, and calling
//GhostRun's EndRun will call its GhostEntity's EndRun.
void GhostEngine::EndRun(GhostRun* run) {
	if (run) {
		ghosts.erase(std::find(ghosts.begin(), ghosts.end(), run));
	}
}

void GhostEngine::StartRun(const char* fileName, bool shouldStart) {
	GhostRun * run = new GhostRun();
	if (!filesystem->FileExists(fileName, "MOD")) {
		Msg("Run does not exist!\n");
	} else {
		if (run->openRun(fileName)) {
			if (shouldStart) {
				run->StartRun();
			}
			ghosts.push_back(run);
			Msg("Loaded run %s!\n",fileName);
		}
	}
}

void startRun_f (const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (Q_strcmp(args.Arg(1), "") != 0)) {
		GhostEngine::getEngine().StartRun(args.Arg(1), true);
	}
}

static int FileAutoComplete ( char const *partial, 
char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	char fileDir[MAX_PATH];
	int toReturn = 0;
	if (UTIL_GetModDir(fileDir, MAX_PATH)) {
		FileFindHandle_t findHandle; // note: FileFINDHandle
		const char *pFilename = filesystem->FindFirstEx( "*.run", "MOD", &findHandle );
		for(int i = 0; pFilename; i++) {
			std::stringstream ss;
			ss << "gh_play " << pFilename;
			Q_strcpy(commands[i], ss.str().c_str());
			pFilename = filesystem->FindNext(findHandle);
			toReturn = i;
		}
	}
	return (toReturn + 1); // number of entries
}

ConCommand start("gh_play", startRun_f, "Loads a ghost and immediately starts playing it.", 0, FileAutoComplete);

void loadRun_f(const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (Q_strcmp(args.Arg(1), "") != 0)) {
		GhostEngine::getEngine().StartRun(args.Arg(1), false);
	}
}

static int FileAutoCompleteLoad ( char const *partial, 
char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	char fileDir[MAX_PATH];
	int toReturn = 0;
	
	if (UTIL_GetModDir(fileDir, MAX_PATH)) {
		FileFindHandle_t findHandle; // note: FileFINDHandle
		const char *pFilename = filesystem->FindFirstEx( "*.run", "MOD", &findHandle );
		for(int i = 0; pFilename; i++) {
			std::stringstream ss;
			ss << "gh_load " << pFilename;
			Q_strcpy(commands[i], ss.str().c_str());
			pFilename = filesystem->FindNext(findHandle);
			toReturn = i;
		}
	}
	return (toReturn + 1); // number of entries
}

ConCommand load("gh_load", loadRun_f, "Loads a ghost but does not play it. Use gh_play_all_ghosts to play it.", 0, FileAutoCompleteLoad);


//Recursive until none left. 
//Didn't want to bother with having the size of the vector change in mid-loop
void GhostEngine::stopAllRuns() {
	if (!isActive()) return;
	GhostRun* it = ghosts[0];
	if(it) it->EndRun();
	stopAllRuns();
}

void GhostEngine::restartAllGhosts() {
	//So we have the data with a given start time, we need to reset the entity,
	//but don't ruin the RunData in the GhostRun.
	if (!isActive()) return;
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		if (!it) {
			Msg("BROKEN: %i\n", i);
			continue;
		}
		if (it->ent && it->ent->isActive) {
			Msg("Restarting ghost %s!\n",it->ghostName);
			it->ent->EndRun(true);
			it->ent->RunData.clear();
		}
	}
	//now the ghostruns are primed with their ents removed.
	//all we need to do now is call StartRun() again, but that's what
	//gh_play_all_ghosts will do.
}

void GhostEngine::playAllGhosts() {
	//Msg("Attempting to play all ghosts: %i...\n", ghosts.size());
	if (!isActive()) return;
	//Msg("Attempting to play all ghosts: %i...\n", ghosts.size());
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		Msg("Attempting to play ghost %s...\n", it->ghostName);
		if (!it) continue;
		if(!it->ent || (!(it->ent->isActive))) {
			it->StartRun();
		}
	}
}


void stopallg() {
	GhostEngine::getEngine().stopAllRuns();
}

void restartG() {
	GhostEngine::getEngine().restartAllGhosts();
}

void playAllG() {
	GhostEngine::getEngine().playAllGhosts();
}


ConCommand stop("gh_stop_all_ghosts", stopallg, "Stops all current ghosts, if there are any.", 0);
ConCommand restart("gh_restart_runs", restartG, "Restarts the run(s) back to the first step. Use gh_play_all_ghosts in order to play them again.", 0);
ConCommand playAll("gh_play_all_ghosts", playAllG, "Plays back all ghosts that are loaded, but not currently playing.", 0);