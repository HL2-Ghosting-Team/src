#include "cbase.h"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "runline.h"
#include "GhostEntity.h"

#pragma once

class GhostRun {
public:
	GhostRun(void);
	~GhostRun(void);

	void StartRun();
	//ends the run for good
	void EndRun();
	void addRunData(RunLine);
	bool openRun(const char*);
	void ResetGhost();
	GhostEntity* ent;
	float startTime;
	std::vector<RunLine> RunData;
	unsigned int step;
	char ghostName[32];
	char currentMap[32];
	bool inReset;

	//convars
	unsigned char trailLength;
	unsigned char trailRed;
	unsigned char trailGreen;
	unsigned char trailBlue;
	unsigned char typeGhost;//0 = solid, 1 = translucent
	unsigned char ghostRed;
	unsigned char ghostGreen;
	unsigned char ghostBlue;
};