#include "cbase.h"
#include "GhostRun.h"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "GhostEngine.h"
#include "tier0/memdbgon.h"


GhostRun::GhostRun()
{
}


GhostRun::~GhostRun(void)
{
}






RunLine * GhostRun::getCurrentRunStep() {
	return currentStep;
}

RunLine * GhostRun::getNextRunStep() {
	return nextStep;
}

bool GhostRun::isRunActive() {
	return isActive;
}

void GhostRun::setRunActive(bool toSet) {
	isActive = toSet;
}

//compare (for online runlines, ignore for now)
/*void GhostRun::addRunData(RunLine line) {
	if (strcmp(line.map, gpGlobals->mapname.ToCStr()) == 0) {
		if (strcmp(ghostName.c_str(), line.name) == 0) {
			RunData.push_back(line);
		}
	}
}*/

void GhostRun::StartRun() {
	ent = (GhostEntity*) CreateEntityByName( "ghost_entity" );
	ent->SetGhostName(ghostName.c_str());
	if (SetUpGhost()) {
		if (DispatchSpawn(ent) == 0) {
			Msg("Spawned the ent %s!\n", ent->GetGhostName());
			isActive = true;
			startTime = Plat_FloatTime();
			ent->StartRun();
		}
	}
}
//sets up a ghost from a level change
void GhostRun::SpawnGhost() {
	ent = (GhostEntity*) CreateEntityByName( "ghost_entity" );
	if (DispatchSpawn(ent) == 0) {
		isActive = true;
		ent->StartRun();
	}
}
//To interpolate:
//Ta = time of current line
//Tb = time of next line
//Tc = current engine time
//Pa = current line position
//Pb = next line position
//Pa + (Tc - Ta) / (Tb - Ta) * (Pb - Pa)

//TODO: there's too much in this method. break it apart.
void GhostRun::HandleFrame() {
	double currentEngineTime = Plat_FloatTime() - startTime;
	updateStep(currentEngineTime);
	RunLine * curr = currentStep;
	RunLine * next = nextStep;
	if (curr == NULL) {
		EndRun();
		return;
	} else {
		if (!isActive) {//the ghost isn't spawned!
			if (strcmp(gpGlobals->mapname.ToCStr(), curr->map) == 0) {
				SpawnGhost();
				ent->SetAbsOrigin(Vector(curr->x, curr->y, (curr->z + 40.0f)));
			}else {
				Msg("The ghost will not be spawned!\n");
			}
		} else {
			if (strcmp(curr->map, gpGlobals->mapname.ToCStr()) != 0) {
				//spawned, but not on the map (anymore). Kill it! Kill it with fire!
				Msg("spawned, but not on the map (anymore). Kill it! Kill it with fire!\n");
				ent->EndRun();
				isActive = false;
				return;
			}
			if (next != NULL) {	
				
			}//else it's last step, updateStep should have already taken care of this
		}
	}
	
}

void GhostRun::EndRun() {
	ent->EndRun();
	RunData.clear();
	GhostEngine::getEngine().EndRun(this);
}




bool GhostRun::SetUpGhost() {
	const char * mapname = gpGlobals->mapname.ToCStr();
	if (strstr(mapname, "background") == NULL) {//not in the menu
		if ( strcmp(mapname, RunData[0].map) == 0) {//same map
			return true;
		} else {
			//TODO teleport to map, start it
			//but for now,
			return false;
		}
	}
	return false;
}
