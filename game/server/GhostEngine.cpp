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
				}
			} else {//the ghost is on/went to a different map, remove it if spawned
				//TODO make it de-spawn or spawn in appropriately
				//actually de-spawning won't be necessary, since the map change does that for us.
				ghost->SetNextThink(0);//so just stop it from thinking/doing anything else til our map changes				
			}
		} else {//end the run, yo
			ghost->EndRun(false);
			//TODO make it remove from the vector of ghosts
		}
	}
}


void split(std::string &txt, std::vector<std::string> &strs, char ch)
{
    unsigned int pos = txt.find( ch );
    unsigned int initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos + 1 ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
	strs.push_back( txt.substr( initialPos, min( pos, txt.size() ) - initialPos + 1 ) );
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
		return true;
	} else {
		Msg("Could not load %s for reading!\n", fileName);
		return false;
	}
}

//creates a ghost and sets its position and name data based on first line
//TODO create a header line for the first line with name and model
void setUpGhost(RunLine firstLine, GhostEntity &result) {
	const char* name = firstLine.name;
	const char * mapname = gpGlobals->mapname.ToCStr();
	if (strstr(mapname, "background") == NULL) {//not in the menu
		if ( mapname == firstLine.map) {//same map
			result.SetGhostName(name);
			result.SetAbsOrigin(Vector(firstLine.x, firstLine.y, firstLine.z));
			result.CreateGhost();
		} else {
			//TODO teleport to map, start it
		}
	}
}

RunLine GhostEngine::readLine(std::string line) {
	std::vector<std::string> spl;
	split(line, spl, ' ');//GHOSTING MAP NAME STATE TIME X Y Z
	struct RunLine l = {
		spl[1].c_str(),//map 
		spl[2].c_str(),//name 
		atof(spl[4].c_str()),//time
		atof(spl[5].c_str()),//x 
		atof(spl[6].c_str()),//y 
		atof(spl[7].c_str())//z
	};
	return l;
}

void GhostEngine::StartRun(const char* fileName) {
	std::vector<RunLine> tempVector;
	if (readFileCompletely(fileName, tempVector)) {
		GhostEntity* ent = new GhostEntity();
		setUpGhost(tempVector[0], *ent);//essentially "step one" of the ghost, using step = 0
		//so now we have a ghost that's not spawned or told to think yet
		//its think is at 0, but it already "read" that line, so
		ent->DoStep();
		//so now we're on the next step, ready to read the run, so
		ent->SetRunData(tempVector);
		ent->Spawn();
		ghosts.push_back(ent);
	}
}

void startRun_f (const CCommand &args) {
	if ( (args.ArgC() > 1) && (args.Arg(1) != NULL) && (args.Arg(1) != "")) {
		GhostEngine::getEngine().StartRun(args.Arg(1));
	}
}

ConCommand start("gh_play", startRun_f, "Play back a run you did.", 0);