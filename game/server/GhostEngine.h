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
	bool shouldDrawTrails();
	unsigned char getTrailLength();
	unsigned char getGhostColorRed();
	unsigned char getGhostColorGreen();
	unsigned char getGhostColorBlue();
	unsigned char getTrailColorRed();
	unsigned char getTrailColorGreen();
	unsigned char getTrailColorBlue();
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