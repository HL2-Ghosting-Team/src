#include "cbase.h"
#include "runline.h"
#include "GhostUtils.h"

#pragma once
class GhostOnlineEntity : public CBaseAnimating
{
	DECLARE_CLASS( GhostOnlineEntity, CBaseAnimating );
	DECLARE_DATADESC();
public:

	//returns the ghost's name
	const char* GetGhostName();
	//returns the ghost's model
	const char* GetGhostModel();

	bool isActive;

	void SetGhostName( const char* );

	GhostUtils::GhostData ghostData;//we're only using this for colors and trail length

	void updateStep(OnlineRunLine);
	void EndRun();
	void CreateTrail();
	void StartRun();
	void HandleGhost();




protected:
	void Think( void );
	void Spawn( void );
	void Precache( void );

private:
	char  m_gModel[256];
	char  m_gName[256];
	OnlineRunLine currentStep;
	float startTime;
	CBaseEntity *trail;
	
};