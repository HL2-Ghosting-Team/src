#include "cbase.h"
#include "runline.h"
#include "GhostOnlineEntity.h"
#include "GhostUtils.h"

#pragma once

class GhostOnlineRun {
public:

	static struct GhostColor_t {
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	};
	static struct TrailColor_t {
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	};

	GhostOnlineRun(const char*, GhostUtils::GhostData);
	~GhostOnlineRun(void);

	void StartRun();
	//ends the run for good
	void EndRun();
	void updateStep(RunLine);
	GhostOnlineEntity* ent;
	bool IsStarted() {
		return isStarted;
	}

	//The name of the ghost for the other person. The engine will search for this.
	//this will never change.
	char ghostName[32];
	//Subject to change, since this is a placeholder for the map the ghost.
	char currentMap[32];

	GhostUtils::GhostData ghostData;

private:
	bool isStarted;

	
};