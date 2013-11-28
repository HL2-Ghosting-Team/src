#include "cbase.h"
#include "GhostEngine.h"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "tier0/memdbgon.h"
#include "filesystem.h"
#include "utlbuffer.h"
#include "timer.h"

//Is the engine supporting any ghosts right now?
bool GhostEngine::isActive() {
	return ghosts.size() != 0;
}

GhostEngine * GhostEngine::instance = NULL;

GhostEngine* GhostEngine::getEngine() {
	if (instance == NULL) {
		instance = new GhostEngine();
		instance->initVars();
	}
	return instance;
}


void splitSpaces(const char* toSplit, std::vector<unsigned char> &toCopyInto) {
	char toBeSplit[1000];
	strcpy(toBeSplit, toSplit);
	char* parts[100] = {0};
	unsigned int index = 0;
	parts[index] = strtok(toBeSplit, " ");
	while(parts[index] != 0)
	{
		toCopyInto.push_back(atoi(parts[index]));
		++index;
		parts[index] = strtok(0, " ");
	}  
}

void fixInts(std::vector<unsigned char> &vec) {
	if (vec.size() == 3) {
		if (vec[0] > 255 || vec[0] < 0) {
			vec[0] = 237;
		}
		if (vec[1] > 255 || vec[1] < 0) {
			vec[1] = 133;
		}
		if (vec[2] > 255 || vec[2] < 0) {
			vec[2] = 60;
		}
	} else {
		vec.clear();
		vec[0] = 237;
		vec[1] = 133;
		vec[2] = 60;
	}
}

//Used to check the ghost and trail colors, and reset them back to default if not appropriate
//and assigns the r, g, b values to the vector.
void getColor(const char* word, std::vector<unsigned char> &vec) {
	if (!word) {
		vec[0] = 237;
		vec[1] = 133;
		vec[2] = 60;
	}
	splitSpaces(word, vec);
	if (vec.empty()) {//default to orange
		vec[0] = 237;
		vec[1] = 133;
		vec[2] = 60;
	} else {
		if (vec.size() == 3) {
			fixInts(vec);
			//Msg("Setting color to be: R: %i G: %i B: %i A: %i\n", atoi(vec[0]), atoi(vec[1]), atoi(vec[2]), atoi(vec[3]));
		} else {
			//Msg("Vector not full: size is %i ... Resetting to default!\n", vec.size());
			vec.clear();
			vec[0] = 237;
			vec[1] = 133;
			vec[2] = 60;
			//Msg("Setting color to be: R: %i G: %i B: %i A: %i\n", atoi(vec[0]), atoi(vec[1]), atoi(vec[2]), atoi(vec[3]));
		}
	}
}
static void onTrailLengthChange(IConVar *var, const char* pOldValue, float fOldValue) {
	int toCheck = ((ConVar*)var)->GetInt();
	//Msg("Trail length change: new %i | old: %i\n", toCheck, (int)fOldValue);
	if (toCheck == (int)fOldValue) return;
	if (toCheck < 0) {
		var->SetValue(((ConVar*)var)->GetDefault());
	}
	gpGlobals->ghostType = (unsigned char)((ConVar*)var)->GetInt();
}
static ConVar spriteLength("gh_trail_length", "5", 
						   FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						   "How long the trail of the ghost lasts in seconds.", 
						   onTrailLengthChange);
unsigned char GhostEngine::getTrailLength() {
	return (unsigned char) spriteLength.GetInt();
}


static void onColorTChange(IConVar *var, const char* pOldValue, float fOldValue) {
	if (!((ConVar*)var)->GetString()) return;
	if (!pOldValue) return;
	if (Q_strcmp(((ConVar*)var)->GetString(), pOldValue) == 0) {
		return;
	}
	std::vector<unsigned char> vec;
	getColor(((ConVar*)var)->GetString(), vec);
	if (vec[0] != 237) {
		std::stringstream ss;
		ss << (int)vec[0] << " " << (int)vec[1] << " " << (int)vec[2];
		gpGlobals->trailRed = vec[0];
		gpGlobals->trailGreen = vec[1];
		gpGlobals->trailBlue = vec[2];
		var->SetValue(ss.str().c_str());
	} else {
		((ConVar*)var)->Revert();
	}
}
static ConVar spriteColor("gh_trail_color", "237 133 60", 
						  FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						  "The R G B values for the color of the ghost's trail.", 
						  onColorTChange);

static void onColorGChange(IConVar *var, const char* pOldValue, float fOldValue) {
	if (!((ConVar*)var)->GetString()) return;
	if (!pOldValue) return;
	if (Q_strcmp(((ConVar*)var)->GetString(), pOldValue) == 0) {
		return;
	}
	std::vector<unsigned char> vec;
	getColor(((ConVar*)var)->GetString(), vec);
	if (vec[0] != 237) {
		std::stringstream ss;
		ss << (int)vec[0] << " " << (int)vec[1] << " " << (int)vec[2];
		gpGlobals->ghostRed = vec[0];
		gpGlobals->ghostGreen = vec[1];
		gpGlobals->ghostBlue = vec[2];
		var->SetValue(ss.str().c_str());
	} else {
		((ConVar*)var)->Revert();
	}
}
static ConVar ghostColor("gh_ghost_color", "237 133 60", 
						 FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						 "The R G B (0 - 255) values for color for your ghost.", onColorGChange);



static void onGTypeChange(IConVar *var, const char* pOldValue, float fOldValue) {
	int toCheck = ((ConVar*)var)->GetInt();
	if (toCheck != 0 && toCheck != 1) {
		var->SetValue(((ConVar*)var)->GetDefault());
	}
	gpGlobals->ghostType = (unsigned char)((ConVar*)var)->GetInt();
}
static ConVar ghostType("gh_ghost_type", "0", 
						FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						"Sets the type of ghost model.\n0 = Solid-fill arrow | 1 = Translucent-style arrow", onGTypeChange);
unsigned char GhostEngine::getGhostType() {
	return (unsigned char) ghostType.GetInt();
}

void GhostEngine::initVars() {
	std::vector<unsigned char> vec;
	getColor(ghostColor.GetString(), vec);
	gpGlobals->ghostRed = vec[0];
	gpGlobals->ghostGreen = vec[1];
	gpGlobals->ghostBlue = vec[2];
	Msg("Setting ghost color: R: %i, G: %i, B: %i\n", gpGlobals->ghostRed, gpGlobals->ghostGreen, gpGlobals->ghostBlue);
	vec.clear();
	getColor(spriteColor.GetString(), vec);
	gpGlobals->trailRed = vec[0];
	gpGlobals->trailGreen = vec[1];
	gpGlobals->trailBlue = vec[2];
	Msg("Current trail color: R: %i, G: %i, B: %i\n", gpGlobals->trailRed, gpGlobals->trailGreen, gpGlobals->trailBlue);
	gpGlobals->ghostType = (unsigned char)ghostType.GetInt();
	Msg("Current ghost type: %i\n", gpGlobals->ghostType);
	gpGlobals->trailLength = (unsigned char)spriteLength.GetInt();
	Msg("Current trail length: %i\n", gpGlobals->trailLength);
}

//-----------------------------------------END VARS ----------------------------------------------------------------
//Called before every level load to transfer the data of
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
		if (Q_strlen(it->ent->currentMap) != 0) {
			Q_strcpy(it->currentMap, it->ent->currentMap);
		} else {//this should never happen, just incase though
			Q_strcpy(it->currentMap, STRING(gpGlobals->mapname));
		}
		it->step = it->ent->step;
		it->startTime = it->ent->startTime;
	}
}

GhostRun* GhostEngine::getRun(GhostEntity* toGet) {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		if (!it || !it->ent) continue;
		if (it->ent == toGet) {
			return it;
		}
	}
	return NULL;
}

//This is not to be mistaken for the resetAllGhosts, this is the
//handler for Level transitions, not resetting the run.
//This gets called after the level inits again.
void GhostEngine::ResetGhosts(void) {
	for (unsigned int i = 0; i < ghosts.size(); i++) {
		GhostRun* it = ghosts[i];
		//Msg("Attempting to reset ghost: %s...\n", it->ghostName);
		if (it) it->ResetGhost();
	}
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
		GhostEngine::getEngine()->StartRun(args.Arg(1), true);
	}
}

static int FileAutoComplete ( char const *partial, 
							 char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	char fileDir[MAX_PATH];
	int toReturn = 0;
	char part[MAX_PATH];
	char* toSearch[2] = {0};
	strcpy(part, partial);
	toSearch[0] = strtok(part, " ");
	toSearch[1] = strtok(0, " ");
	if (UTIL_GetModDir(fileDir, MAX_PATH)) {
		FileFindHandle_t findHandle; // note: FileFINDHandle
		std::stringstream ss1;
		ss1 << (toSearch[1] == 0 ? "" : toSearch[1]) << "*.run";
		const char *pFilename = filesystem->FindFirstEx( ss1.str().c_str(), "MOD", &findHandle );
		for(int i = 0; pFilename; i++) {
			std::stringstream ss;
			ss << "gh_play " << pFilename;
			Q_strcpy(commands[i], ss.str().c_str());
			pFilename = filesystem->FindNext(findHandle);
			toReturn = i + 1;
		}
		filesystem->FindClose(findHandle);
	}
	return toReturn; // number of entries
}

ConCommand start("gh_play", startRun_f, "Loads a ghost and immediately starts playing it.", 0, FileAutoComplete);

void loadRun_f(const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (Q_strcmp(args.Arg(1), "") != 0)) {
		GhostEngine::getEngine()->StartRun(args.Arg(1), false);
	}
}

static int FileAutoCompleteLoad ( char const *partial, 
								 char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	char fileDir[MAX_PATH];
	int toReturn = 0;
	char part[MAX_PATH];
	char* toSearch[2] = {0};
	strcpy(part, partial);
	toSearch[0] = strtok(part, " ");
	toSearch[1] = strtok(0, " ");
	if (UTIL_GetModDir(fileDir, MAX_PATH)) {
		FileFindHandle_t findHandle; // note: FileFINDHandle
		std::stringstream ss1;
		ss1 << (toSearch[1] == 0 ? "" : toSearch[1]) << "*.run";
		const char *pFilename = filesystem->FindFirstEx( ss1.str().c_str(), "MOD", &findHandle );
		for(int i = 0; pFilename; i++) {
			std::stringstream ss;
			ss << "gh_load " << pFilename;
			Q_strcpy(commands[i], ss.str().c_str());
			pFilename = filesystem->FindNext(findHandle);
			toReturn = i + 1;
		}
		filesystem->FindClose(findHandle);
	}
	return toReturn; // number of entries
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
		if (it->ent && it->ent->isActive) {
			Msg("Restarting ghost %s!\n",it->ghostName);
			it->ent->EndRun(true);
			it->ent->clearRunData();
			it->ent = NULL;
			Q_strcpy(it->currentMap, it->RunData[0].map);
			BlaTimer::timer()->UpdateGhost((size_t)it, 0, "Resetting...");
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
		if(!(it->ent) || (!(it->ent->isActive))) {
			it->StartRun();
		}
	}
}


void stopallg() {
	GhostEngine::getEngine()->stopAllRuns();
}

void restartG() {
	GhostEngine::getEngine()->restartAllGhosts();
}

void playAllG() {
	GhostEngine::getEngine()->playAllGhosts();
}


ConCommand stop("gh_stop_all_ghosts", stopallg, "Stops all current ghosts, if there are any.", 0);
ConCommand restart("gh_restart_runs", restartG, "Restarts the run(s) back to the first step. Use gh_play_all_ghosts in order to play them again.", 0);
ConCommand playAll("gh_play_all_ghosts", playAllG, "Plays back all ghosts that are loaded, but not currently playing.", 0);

