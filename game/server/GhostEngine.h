#include "GhostEntity.h"
#include <vector>
#include <map>
#include <string>
#include "runline.h"
#pragma once
class GhostEngine {

public:
	
	GhostEngine()
	{
		ghostCount = 0;
	};
	//gets the singleton Engine instance
	static GhostEngine& getEngine();
	//gets a ghost by the name <name> (used for online maybe)
	GhostEntity* GetGhost(const char * name); 
	void handleGhost(float t, GhostEntity * ghost);
	void StartRun(const char * thing);
	std::vector<GhostEntity*> ghosts;
	RunLine readLine(std::string line);
	bool readFileCompletely(std::string fileName, std::vector<RunLine> &vec);
private:
	static GhostEngine* instance;
	int ghostCount;
	
	
};