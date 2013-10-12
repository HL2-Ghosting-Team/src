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


bool GhostRun::open(const char* fileName) {
	std::ifstream myFile = std::ifstream(fileName);
	if (!myFile) return false;
	RunData.clear();
	for(int i = 0; myFile.good(); i++)
	{
		std::string temp;
		std::getline(myFile, temp);
		RunLine result = readLine(temp);
		if (result.tim < 0) {
			ghostName = result.name;
		}
		RunData.push_back(result);
	}
	myFile.close();
	return true;

}

RunLine GhostRun::readLine(std::string line) {
	struct RunLine l;
	std::sscanf(line.c_str(), "%*s %31s %31s %f %f %f %f", &l.map, &l.name, &l.tim, &l.x, &l.y, &l.z);
	return l;
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
void GhostRun::addRunData(RunLine line) {
	if (strcmp(line.map, gpGlobals->mapname.ToCStr()) == 0) {
		if (strcmp(ghostName.c_str(), line.name) == 0) {
			RunData.push_back(line);
		}
	}
}

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
				if (strcmp(curr->map, next->map) == 0) {
					Msg("Trying to interpolate...\n");
					//interpolate position
					float x = curr->x;
					float y = curr->y;
					float z = curr->z;
					float x2 = next->x;
					float y2 = next->y;
					float z2 = next->z;
					float t1 = curr->tim;
					float t2 = next->tim;
					float scalar = (((float)currentEngineTime) - t1) / (t2 - t1); 
					float xfinal = x + (scalar * (x2 - x));
					float yfinal = y + (scalar * (y2 - y));
					float zfinal = z + (scalar * (z2 - z));
					ent->SetAbsOrigin(Vector(xfinal, yfinal, zfinal));
				} else {//set it to the last position before it updates to the next map
					Msg("On its last step before the next map!\n");
					ent->SetAbsOrigin(Vector(curr->x, curr->y, (curr->z + 40.0f)));
				}
			}//else it's last step, updateStep should have already taken care of this
		}
	}
	
}

void GhostRun::EndRun() {
	ent->EndRun();
	RunData.clear();
	GhostEngine::getEngine().EndRun(this);
}

void GhostRun::updateStep(float currentTime) {
	const size_t runsize = RunData.size();
	if ((step < 0) || (step >= runsize)) {
		currentStep = nextStep = NULL;
		return;
	} 
	if (step == 0) step = 1;
	currentStep = &RunData[step];
	Msg("Line: %s, %s, %f, %f, %f, %f\n", currentStep->map, currentStep->name, currentStep->tim, currentStep->x, currentStep->y, currentStep->z);
	if (currentTime > currentStep->tim) {//catching up to a fast ghost, you came in late
		int x = step + 1;
		while (++x < runsize && currentTime > RunData[x].tim);
		step = x-1;
	}
	Msg("Step %i\n", step);
	currentStep = &RunData[step];//update it to the new step
	Msg("Line Updated: %s, %s, %f, %f, %f, %f\n", currentStep->map, currentStep->name, currentStep->tim, currentStep->x, currentStep->y, currentStep->z);
	if (currentTime > currentStep->tim) {//if it's on the last step
		nextStep = NULL;
		if (step == (runsize - 1)) {
			EndRun();
		}
	} else {
		nextStep = &RunData[step+1];
	}
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
