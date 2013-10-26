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
	if (!fileName) return false;//this is just incase
	char dir[MAX_PATH];
	engine->GetGameDir(dir, MAX_PATH);
	V_AppendSlash(dir, MAX_PATH);
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
			strcpy(ghostName, result.name);
			continue;
		}
		RunData.push_back(result);
	}
	myFile.close();
	return true;


	/*FileHandle_t fh = filesystem->Open(fileName, "r", "MOD");
	if (fh) {


	}


	if (filesystem->ReadFile(fileName, "MOD", buf)) {
	bool first = true;
	while () {
	char tempchar[256];
	buf.GetLine(tempchar);

	std::string temp = std::string(tempchar);
	RunLine result = readLine(temp);
	if (first) {
	strcpy(ghostName, result.name);
	first = false;
	continue;
	}
	RunData.push_back(result);
	}
	buf.Clear();
	return true;
	}*/
	return false;

}

void GhostRun::ResetGhost() {
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
			entity->startTime = (float) Plat_FloatTime();
			entity->StartRun();
			ent = entity;
		}
	}
}

//Ends the run and clears data.
//>>CALLS THE ENT's ENDRUN()!<<
void GhostRun::EndRun() {
	ent->RunData.clear();
	ent->EndRun();
	RunData.clear();
	GhostEngine::getEngine().EndRun(this);
}
