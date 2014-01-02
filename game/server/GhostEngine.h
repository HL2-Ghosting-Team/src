#include "GhostRun.h"
#include "runline.h"
//#include "OnlineGhostRun.h"
#pragma once
class GhostEngine {

public:
	GhostEngine() {
		isOnlineMode = false;
	};
	//gets the singleton Engine instance
	static GhostEngine* getEngine();
	CUtlVector<GhostRun*> ghosts;
	//CUtlVector<OnlineGhostRun*> onlineGhosts;
	static GhostEngine* instance;
	bool isOnline();
	void setOnlineMode(bool);
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
	bool isOnlineMode;

};