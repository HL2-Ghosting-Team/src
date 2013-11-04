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

	void updateForNewMap();
	void StartRun();
	//ends the run for good
	void EndRun();
	void addRunData(RunLine);
	bool openRun(const char*);
	//bool SetUpGhost();
	void ResetGhost();
	GhostEntity* ent;
	float startTime;
	std::vector<RunLine> RunData;
	unsigned int step;
	char ghostName[32];
	bool inReset;
};