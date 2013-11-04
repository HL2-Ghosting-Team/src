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
	void SetGhostName( const char* );
	void SetGhostModel( const char* );
	//returns the ghost's name
	const char* GetGhostName();
	//returns the ghost's model
	const char* GetGhostModel();
	//Increments the steps intelligently.
	void updateStep();
	void EndRun(bool);
	void CreateTrail();
	void StartRun();
	void HandleGhost();
	float startTime;
	void clearRunData();
	unsigned int step;
	bool isActive;
	bool inReset;

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
	std::vector<RunLine> RunData;
	
};
