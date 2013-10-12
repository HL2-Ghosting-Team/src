#include "GhostEntity.h"
#include "GhostRun.h"
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
	GhostEntity* GetGhost(const char * name); 
	void handleGhost(float t, GhostEntity * ghost);
	void StartRun(const char * thing);
	std::vector<GhostRun*> ghosts;
	//main loop of the engine
	void Loop();
	bool readFileCompletely(std::string fileName, std::vector<RunLine> &vec);
	void EndRun(GhostRun*);
	void ResetGhosts();
	bool isActive();
	
private:
	static GhostEngine* instance;
	bool isLocal;
	
	
};