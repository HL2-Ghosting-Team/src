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




class GhostRun
{



public:
	GhostRun(void);
	~GhostRun(void);

	bool open(const char*);
	void updateForNewMap();
	void StartRun();
	//ends the run for good
	void EndRun();
	RunLine readLine(std::string);
	void addRunData(RunLine);
	void updateStep(float currentTime);
	RunLine* getCurrentRunStep();
	RunLine* getNextRunStep();
	bool SetUpGhost();
	double startTime;
	GhostEntity* ent;
	bool isRunActive();
	void setRunActive(bool);
	void HandleFrame();
	void SpawnGhost();
private:
	
	std::string ghostName;
	
	RunLine* currentStep; 
	RunLine* nextStep;
	bool isActive;
	
};

