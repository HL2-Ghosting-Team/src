#include "GhostRun.h"
#include <vector>
#include <string>
#include "runline.h"
#pragma once
class GhostEngine {

public:
	GhostEngine() {
	};
	//gets the singleton Engine instance
	static GhostEngine* getEngine();
	std::vector<GhostRun*> ghosts;
	static GhostEngine* instance;
	bool isActive();
	//convars
	void initVars();
	unsigned char getGhostType();
	unsigned char shouldDrawTrails();
	unsigned char getTrailLength();
	void EndRun(GhostRun*);
	void StartRun(const char*, bool);
	void ResetGhosts(void);
	void restartAllGhosts(void);
	void playAllGhosts();
	void transferGhostData(void);
	GhostRun* getRun(GhostEntity*);
	void stopAllRuns();	
	
private:
	bool isLocal;

};