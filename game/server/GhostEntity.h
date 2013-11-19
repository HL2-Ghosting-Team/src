#include "cbase.h"
#include <vector>
#include "runline.h"

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
	float startTime;
	char currentMap[32];
#ifndef CLIENT_DLL
	void SetRunData( std::vector<RunLine>& toSet);
	void SetGhostName( const char* );
	void SetGhostModel( const char* );
	//Increments the steps intelligently.
	void updateStep();
	void EndRun(bool);
	void CreateTrail();
	void StartRun();
	void HandleGhost();
	
	void clearRunData();




protected:
	void Think( void );
	void Spawn( void );
	void Precache( void );
#endif
private:
	char  m_gModel[256];
	char  m_gName[256];
	RunLine* currentStep; 
	RunLine* nextStep;
	CBaseEntity *trail;
	std::vector<RunLine> RunData;
	
};
