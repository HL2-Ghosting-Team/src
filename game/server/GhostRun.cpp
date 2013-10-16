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

RunLine GhostRun::readLine(std::string line) {
	struct RunLine l;
	std::sscanf(line.c_str(), "%*s %31s %31s %f %f %f %f", &l.map, &l.name, &l.tim, &l.x, &l.y, &l.z);
	return l;
}

bool GhostRun::openRun(const char* fileName) {
	std::ifstream myFile = std::ifstream(fileName);
	if (!myFile) return false;
	RunData.clear();
	for(int i = 0; myFile.good(); i++)
	{
		std::string temp;
		std::getline(myFile, temp);
		RunLine result = readLine(temp);
		if (result.tim < 0) {
			strcpy(ghostName, result.name);
			continue;
		}
		RunData.push_back(result);
	}
	myFile.close();
	return true;
}

void GhostRun::ResetGhost() {
	GhostEntity* tempGhost = (GhostEntity*) CreateEntityByName("ghost_entity");
	Msg("Should be spawning ghost now...\n");
	if(DispatchSpawn(tempGhost)) {
		Msg("Spawned!\n");
		tempGhost->RunData = RunData;
		tempGhost->step = step;
		tempGhost->SetGhostName(ghostName);
		tempGhost->StartRun();
		ent = *tempGhost;
	}
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
	GhostEntity * entity = (GhostEntity*) CreateEntityByName( "ghost_entity" );
	if (entity) {
		entity->SetGhostName(RunData[0].name);
		entity->RunData = RunData;
		entity->SetAbsOrigin(Vector(RunData[1].x, RunData[1].y, RunData[1].z));
		if (DispatchSpawn(entity) == 0) {
			Msg("Spawned the ent %s!\n", entity->GetGhostName());
			entity->startTime = gpGlobals->curtime;
			Msg("Start time: %f\n", entity->startTime);
			entity->StartRun();
			ent = *entity;
		}
	}
}

void GhostRun::EndRun() {
	ent.RunData.clear();
	ent.EndRun();
	RunData.clear();
	GhostEngine::getEngine().EndRun(this);
}


/*bool GhostRun::SetUpGhost() {
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
}*/
