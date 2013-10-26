#include "GhostRun.h"
#include "igameevents.h"
#include <vector>
#include <map>
#include <string>
#include "runline.h"
#pragma once
class GhostEngine {

public:
	
	GhostEngine()
	{
		isLocal = true;
	};
	//gets the singleton Engine instance
	static GhostEngine& getEngine();
	//gets a ghost by the name <name> (used for online maybe)
	//GhostEntity* GetGhost(const char * name); 
	std::vector<GhostRun*> ghosts;
	//main loop of the engine
	bool readFileCompletely(std::string fileName, std::vector<RunLine> &vec);
	void EndRun(GhostRun*);
	void StartRun(const char*);
	void ResetGhosts();
	void transferGhostData();
	bool isActive();
	GhostRun* getRun(GhostEntity*);
	void stopAllRuns();
	
private:
	static GhostEngine* instance;
	bool isLocal;
	
	
};