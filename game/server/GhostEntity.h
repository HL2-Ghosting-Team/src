#include "cbase.h"
#include "runline.h"
#include "GhostUtils.h"

#pragma once
class GhostEntity : public CBaseAnimating
{
	DECLARE_CLASS( GhostEntity, CBaseAnimating );
	DECLARE_DATADESC();
public:

	//returns the ghost's name
	const char* GetGhostName();
	//returns the ghost's model
	const char* GetGhostModel();
	unsigned int step;
	bool isActive;
	GhostUtils::GhostData ghostData;
	float startTime;
	char currentMap[32];
	//void SetRunData(CUtlVector<RunLine>&);
	void SetGhostName( const char* );
	void SetGhostModel( const char* );
	//Increments the steps intelligently.
	void updateStep();
	void EndRun();
	void CreateTrail();
	void StartRun();
	void HandleGhost();
	void clearRunData();




protected:
	void Think( void );
	void Spawn( void );
	void Precache( void );

private:
	char  m_gModel[256];
	char  m_gName[256];
	RunLine* currentStep; 
	RunLine* nextStep;
	CBaseEntity *trail;
	
};
