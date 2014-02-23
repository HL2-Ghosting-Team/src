#include "cbase.h"
#include "GhostEngine.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "filesystem.h"
#include "ghosthud.h"
#include "timer.h"
#include "GhostUtils.h"


#include "tier0/memdbgon.h"

//Is the engine supporting any ghosts right now?
bool GhostEngine::isActive() {
	return (ghosts.Count() != 0);// || (onlineGhosts.Count() != 0);
}

GhostEngine* GhostEngine::instance = NULL;

GhostEngine* GhostEngine::getEngine() {
	if (instance == NULL) {
		instance = new GhostEngine();
		instance->initVars();
	}
	return instance;
}

bool GhostEngine::isOnline() {
	return isOnlineMode;
}

void GhostEngine::setOnlineMode(bool newBool) {
	isOnlineMode = newBool;
}


static void listRunC(const CCommand &args) {
	GhostUtils::listRun(args.Arg(1));
}

static ConCommand listRun("gh_listrun", listRunC, "Gives an in-depth look at a ghost file.", 0, GhostUtils::FileAutoCompleteList);


static void onTrailLengthChange(IConVar *var, const char* pOldValue, float fOldValue) {
	int toCheck = ((ConVar*)var)->GetInt();
	//Msg("Trail length change: new %i | old: %i\n", toCheck, (int)fOldValue);
	if (toCheck == (int)fOldValue) return;
	if (toCheck < 0) {
		var->SetValue(((ConVar*)var)->GetDefault());
	}
	gpGlobals->trailLength = (unsigned char)((ConVar*)var)->GetInt();
}
static ConVar spriteLength("gh_trail_length", "5", 
						   FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						   "How long the trail of the ghost lasts in seconds.\n0 = no trail drawn for your ghost.", 
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
	CUtlVector<unsigned char> vec;
	GhostUtils::getColor(((ConVar*)var)->GetString(), vec);
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
	CUtlVector<unsigned char> vec;
	GhostUtils::getColor(((ConVar*)var)->GetString(), vec);
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


void GhostEngine::initVars() {
	CUtlVector<unsigned char> vec;
	GhostUtils::getColor(ghostColor.GetString(), vec);
	gpGlobals->ghostRed = vec[0];
	gpGlobals->ghostGreen = vec[1];
	gpGlobals->ghostBlue = vec[2];
	Msg("Setting ghost color: R: %i, G: %i, B: %i\n", gpGlobals->ghostRed, gpGlobals->ghostGreen, gpGlobals->ghostBlue);
	vec.RemoveAll();
	GhostUtils::getColor(spriteColor.GetString(), vec);
	gpGlobals->trailRed = vec[0];
	gpGlobals->trailGreen = vec[1];
	gpGlobals->trailBlue = vec[2];
	Msg("Current trail color: R: %i, G: %i, B: %i\n", gpGlobals->trailRed, gpGlobals->trailGreen, gpGlobals->trailBlue);
	gpGlobals->trailLength = (unsigned char)spriteLength.GetInt();
	Msg("Current trail length: %i\n", gpGlobals->trailLength);
	Msg("Name for the person is %s\n", engine->GetClientConVarValue(0, "gh_name"));
}

//-----------------------------------------END VARS ----------------------------------------------------------------



//Called before every level load to transfer the data of
//the last ghost, for continuity's sake.
void GhostEngine::transferGhostData() {
	int size = ghosts.Count();
	for (int i = 0; i < size; i++) {
		GhostRun * it = ghosts[i];
		if (!it->ent || !it->isPlaying) {//in user-reset or not started
			continue;
		}
		Msg("Transferring ghost data for %s!\n", it->ghostName);
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
	int size = ghosts.Count();
	for (int i = 0; i < size; i++) {
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
	int size = ghosts.Count();
	for (int i = 0; i < size; i++) {
		GhostRun* it = ghosts[i];
		Msg("Attempting to reset ghost: %s...\n", it->ghostName, it->step);
		if (it && it->isPlaying) it->StartRun(true);
	}
}

//Called after the run kills itself, >>should be the last thing called!!<<
//For example, the order of EndRun() s to be called goes GhostEntity -> GhostRun -> GhostEngine
//Although, keep in mind that all GhostEntity's EndRun does is remove it from the game, and calling
//GhostRun's EndRun will call its GhostEntity's EndRun.
void GhostEngine::EndRun(GhostRun* run) {
	if (run) {
		ghosts.FindAndRemove(run);
	}
}

void GhostEngine::StartRun(const char* fileName, bool shouldStart) {
	if (!fileName) {
		Warning("Run does not exist!\n");
		return;
	}
	GhostRun* run = new GhostRun();
	char filePath[256];
	Q_strcpy(filePath, "runs/");
	Q_strcat(filePath, fileName, sizeof(filePath));
	if (!filesystem->FileExists(filePath, "MOD")) {
		Msg("Run does not exist!\n");
	} else {
		if (run->openRun(filePath)) {
			if (shouldStart) {
				run->StartRun(false);
			}
			ghosts.AddToTail(run);
			Msg("Loaded run %s!\n", filePath);
		}
	}
}

void startRun_f (const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (Q_strcmp(args.Arg(1), "") != 0)) {
		GhostEngine::getEngine()->StartRun(args.Arg(1), true);
	}
}

ConCommand start("gh_play", startRun_f, "Loads a ghost and immediately starts playing it.", 0, GhostUtils::FileAutoComplete);

void loadRun_f(const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (Q_strcmp(args.Arg(1), "") != 0)) {
		GhostEngine::getEngine()->StartRun(args.Arg(1), false);
	}
}

ConCommand load("gh_load", loadRun_f, "Loads a ghost but does not play it. Use gh_play_all_ghosts to play it.", 0, GhostUtils::FileAutoCompleteLoad);

//Recursive until none left. 
//Didn't want to bother with having the size of the vector change in mid-loop
void GhostEngine::stopAllRuns() {
	if (!isActive()) return;
	GhostRun* it = ghosts[0];
	if(it) it->EndRun();
	stopAllRuns();
}

//The void for user-enduced reset (a la "gh_restart_runs")
void GhostEngine::restartAllGhosts() {
	//So we have the data with a given start time, we need to reset the entity,
	//but don't ruin the RunData in the GhostRun.
	if (!isActive()) return;
	int size = ghosts.Count();
	for (int i = 0; i < size; i++) {
		GhostRun* it = ghosts[i];
		if (it->ent && it->ent->isActive) {
			Msg("Restarting ghost %s!\n",it->ghostName);
			it->ent->EndRun();
			it->ent->clearRunData();
			it->ent = NULL;
			it->isPlaying = false;
			it->step = 0;
			it->startTime = 0.0f;
			Q_strcpy(it->currentMap, it->ghostData.RunData[0].map);
			GhostHud::hud()->UpdateGhost((size_t)it, 0, "Resetting...");
		}
	}
	//now the ghostruns are primed with their ents removed.
	//all we need to do now is call StartRun() again, but that's what
	//gh_play_all_ghosts will do.
}

void GhostEngine::playAllGhosts() {
	//Msg("Attempting to play all ghosts: %i...\n", ghosts.size());
	if (!isActive()) return;
	int size = ghosts.Count();
	for (int i = 0; i < size; i++) {
		GhostRun* it = ghosts[i];
		Msg("Attempting to play ghost %s...\n", it->ghostName);
		if(!(it->ent) || (!(it->ent->isActive))) {
			it->StartRun(false);
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

static ConVar shouldDraw("gh_draw_trails", "1", FCVAR_REPLICATED | FCVAR_ARCHIVE | FCVAR_DEMO,
	"Toggles global drawing of trails on (1) or off (0).");

bool GhostEngine::shouldDrawTrails() {
	return shouldDraw.GetBool();
}


//timer
void startTimer() {
	BlaTimer::timer()->Start();
}

void stopTimer() {
	BlaTimer::timer()->Stop();
}

ConCommand startTimer_c("gh_timer_start", startTimer, "Starts the timer.", 0);
ConCommand stopTimer_c("gh_timer_stop", stopTimer, "Stops the timer.", 0);




