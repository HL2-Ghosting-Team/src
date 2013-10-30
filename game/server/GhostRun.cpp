#include "cbase.h"
#include "GhostRun.h"
#include "filesystem.h"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "GhostEngine.h"
#include "tier0/memdbgon.h"
#include "utlbuffer.h"


GhostRun::GhostRun(){ent = NULL;}

GhostRun::~GhostRun(void){}

RunLine GhostRun::readLine(std::string line) {
	struct RunLine l;
	std::sscanf(line.c_str(), "%*s %31s %31s %f %f %f %f", &l.map, &l.name, &l.tim, &l.x, &l.y, &l.z);
	return l;
}

bool GhostRun::openRun(const char* fileName) {
	if (!fileName) return false;//this is just incase
	char dir[MAX_PATH];
	char file[256];
	Q_strcpy(file, fileName);
	engine->GetGameDir(dir, MAX_PATH);
	V_AppendSlash(dir, MAX_PATH);
	V_SetExtension(file, ".run", sizeof(file));
	strcat(dir, fileName);
	V_FixupPathName(dir, 0, dir);
	std::ifstream myFile = std::ifstream(dir);
	RunData.clear();
	for(int i = 0; myFile.good(); i++)
	{
		std::string temp;
		std::getline(myFile, temp);
		RunLine result = readLine(temp);
		if (i == 0) {
			Q_strcpy(ghostName, result.name);
			continue;
		}
		RunData.push_back(result);
	}
	myFile.close();
	return true;
}

void GhostRun::ResetGhost() {
	if (inReset) return;//if we've been reset by the user, ignore us
	GhostEntity* tempGhost = dynamic_cast<GhostEntity*>(CreateEntityByName("ghost_entity"));
	if(DispatchSpawn(tempGhost) == 0) {
		tempGhost->SetRunData(RunData);
		tempGhost->step = step;
		tempGhost->SetGhostName(ghostName);
		tempGhost->startTime = startTime;
		tempGhost->StartRun();
		ent = tempGhost;
	}
}

void GhostRun::StartRun() {
	GhostEntity * entity = (GhostEntity*) CreateEntityByName( "ghost_entity" );
	if (entity) {
		entity->SetGhostName(RunData[0].name);
		entity->RunData = RunData;
		entity->SetAbsOrigin(Vector(RunData[1].x, RunData[1].y, RunData[1].z));
		if (DispatchSpawn(entity) == 0) {
			entity->startTime = (float) Plat_FloatTime();
			entity->step = 0;
			entity->StartRun();
			ent = entity;
		}
	}
}

//Ends the run and clears data.
//>>CALLS THE ENT's ENDRUN()!<<
//This should only be called when gh_stop_all_ghosts is called
//OR if the run actually finished.
void GhostRun::EndRun() {
	if (ent) {
		ent->RunData.clear();
		ent->EndRun(false);
	}
	RunData.clear();
	GhostEngine::getEngine().EndRun(this);
}
