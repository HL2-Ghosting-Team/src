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
	std::vector<GhostRun*> ghosts;
	void EndRun(GhostRun*);
	void StartRun(const char*, bool);
	void ResetGhosts();
	void restartAllGhosts();
	void playAllGhosts();
	void transferGhostData();
	bool isActive();
	GhostRun* getRun(GhostEntity*);
	void stopAllRuns();
	
private:
	static GhostEngine* instance;
	bool isLocal;
	
	
};