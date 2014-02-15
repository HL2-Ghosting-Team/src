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
	char ghostNameHud[512];
	char ghostName[512];

	GhostUtils::GhostData ghostData;

private:
	bool isStarted;

	
};