#include "cbase.h"
#include <vector>
#include "runline.h"

#pragma once
class GhostEntity : public CBaseAnimating
{
	DECLARE_CLASS( GhostEntity, CBaseAnimating );
	DECLARE_DATADESC();
public:
	void SetRunData( std::vector<RunLine>& toSet);
	void Spawn( void );
	void Precache( void );
	void MoveThink( void );
	void SetGhostName( const char* );
	void SetGhostModel( const char* );
	//returns the ghost's name
	const char* GetGhostName();
	//returns the ghost's model
	const char* GetGhostModel();
	//increments step by one
	void updateStep();
	//returns current step
	int GetCurrentStep();
	void EndRun();
	void StartRun();
	void updateStep();
	void HandleGhost();
	bool openRun(const char*);
	void SetShouldUpdate(bool);
	void SetRunData(std::vector<RunLine>);
	RunLine readLine(std::string);
	float startTime;
	std::vector<RunLine> RunData;
	//void SetGhostRun(GhostRun*);
	//void HandleGhost();
	

private:
	//GhostRun* run;
	int step;
	bool isActive;
	const char*  m_gModel;
	const char*  m_gName;
	RunLine* currentStep; 
	RunLine* nextStep;
	
};
