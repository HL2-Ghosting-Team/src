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

GhostEntity* GhostEngine::GetGhost(const char * name) {
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
}
//tim is the current time (for comparison), ghost is the ghost (duh)
void GhostEngine::handleGhost(float tim, GhostEntity * ghost) {
	if (ghost) {
		int step = ghost->GetCurrentStep();
		int max = ghost->RunData.size();
		//first determine if the line is the last/should not read it
		if (step <= max) {//not over it (array index out of bounds)
			RunLine currentStep = ghost->RunData[step];
			//is it even on the same map?
			if (strcmp(gpGlobals->mapname.ToCStr(), currentStep.map) == 0) {
				ghost->SetAbsOrigin(Vector(currentStep.x, currentStep.y, currentStep.z));
				ghost->DoStep();
				/*
				//so now we should compare time
				//we have a tolerance of ~0.03 seconds.
				float diff = currentStep.tim - tim;
				if ( diff >= 0) {
					if (diff < 0.04) {//tolerance
						//here we go
						ghost->MoveGhost(Vector(currentStep.x, currentStep.y, currentStep.z));
						ghost->DoStep();
					} else {//the ghost has a time difference greater than the tolerance
						//there's two cases that this happens:
						//death of the ghost || level change
						//since line 41 checks the level, it's going to obviously be a death here
						//our action is to wait until it changes to that time, so no code goes here
					}
				} else {//the difference is negative. (our current time is more than the recorded time)
					//we may be falling behind, time to catch up
					//This can be caused from the current run being slower than the recorded one,
					//and now we've finally changed to the level the ghost is on, so it's going to need
					//to find the right line to read
					Msg("We may be falling behind!\n");
					//TODO catch up
				}*/
			} else {//the ghost is on/went to a different map, remove it if spawned
				//TODO make it de-spawn or spawn in appropriately
				//actually de-spawning won't be necessary, since the map change does that for us.
				ghost->SetShouldUpdate(false);//so just stop it from thinking/doing anything else til our map changes				
			}
		} else {//end the run, yo
			ghost->EndRun(false);
			//TODO make it remove from the vector of ghosts
		}
	}
}


//myFile is the file to open, vector is the array to write to
bool GhostEngine::readFileCompletely(std::string fileName, std::vector<RunLine> &vec) {
	std::ifstream myFile = std::ifstream(fileName);
	if (myFile) {
		for(int i = 0; myFile.good(); i++)
		{
			std::string temp;
			std::getline(myFile, temp);
			vec.push_back(GhostEngine::readLine(temp));
		}
		myFile.close();
		return true;
	} else {
		return false;
	}
}

//creates a ghost and sets its position and name data based on first line
//TODO create a header line for the first line with name and model
void setUpGhost(RunLine firstLine, GhostEntity &result) {
	const char* name = firstLine.name;
	Msg("Name is: %s\n", name);
	const char * mapname = gpGlobals->mapname.ToCStr();
	if (strstr(mapname, "background") == NULL) {//not in the menu
		if ( strcmp(mapname, firstLine.map) == 0) {//same map
			result.m_gName = name;//for the EntityText
			result.SetAbsOrigin(Vector(firstLine.x, firstLine.y, firstLine.z));
		} else {
			//TODO teleport to map, start it
		}
	}
}

/* GhostEngine::readLine(std::string line)
» struct RunLine l;
» std::sscanf(line, "GHOSTING %s %s %*s %f %f %f", &l.map, &l.name, &l.tim, &l.x, &l.y, &l.z);*/

RunLine GhostEngine::readLine(std::string line) {
	std::vector<std::string> spl;
	struct RunLine l;
	std::sscanf(line.c_str(), "%*s %31s %31s %f %f %f %f", &l.map, &l.name, &l.tim, &l.x, &l.y, &l.z);
	/*split(line, spl, ' ');//GHOSTING MAP NAME STATE TIME X Y Z
	struct RunLine l = {
		spl[1].c_str(),//map 
		spl[2].c_str(),//name 
		atof(spl[4].c_str()),//time
		atof(spl[5].c_str()),//x 
		atof(spl[6].c_str()),//y 
		atof(spl[7].c_str())//z
	};*/
	return l;
}

void GhostEngine::StartRun(const char* fileName) {
	std::vector<RunLine> tempVector;
	if (readFileCompletely(fileName, tempVector)) {
		GhostEntity* ent = (GhostEntity*) CreateEntityByName( "ghost_entity" );
		setUpGhost(tempVector[0], *ent);//essentially "step one" of the ghost, using step = 0
		ent->m_gName = tempVector[0].name;
		ent->step = 0;
		//so now we have a ghost that's not spawned or told to think yet
		//its think is at 0, but it already "read" that line, so
		ent->DoStep();
		//so now we're on the next step, ready to read the run, so
		ent->SetRunData(tempVector);
		if (DispatchSpawn(ent) == 0) {
			Msg("Spawned the ent %s!\n", ent->GetGhostName());
			ent->StartRun();
			ghosts.push_back(ent);
		}
	} else {

	}
}

void startRun_f (const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (args.Arg(1) != "")) {
		GhostEngine::getEngine().StartRun(args.Arg(1));
	}
}

ConCommand start("gh_play", startRun_f, "Play back a run you did.", 0);