#include "cbase.h"
#include <vector>
#include "runline.h"
#pragma once
class GhostEntity : public CBaseAnimating
{
public:
	DECLARE_CLASS( GhostEntity, CBaseAnimating );
	DECLARE_DATADESC();
	// Start of our data description for the class
	GhostEntity()
	{
		m_gModel = "models/cone.mdl";
		m_gName = "Ghosting Entity";
		step = 0;
	};
	GhostEntity(const char* name) {
		m_gModel = "models/cone.mdl";
		m_gName = name;
		step = 0;
	};
	GhostEntity(const char* name, const char* model) {
		m_gName = name;
		m_gModel = model;
		step = 0;
	};
	void SetRunData( std::vector<RunLine>& toSet);
	void Spawn( void );
	void Precache( void );
	void MoveThink( void );
	void SetGhostName( const char* );
	void SetGhostModel( const char* );
	const char* GetGhostName();
	const char* GetGhostModel();
	//spawns the entity with the model and name
	void CreateGhost();
	//increments step by one
	void DoStep();
	//returns current step
	int GetCurrentStep();
	void EndRun(bool);
	void MoveGhost( Vector loc );
	
	std::vector<RunLine> RunData;
	
 
private:
	float startTime;
	float nextTime;
	const char*  m_gModel;
	const char*  m_gName;
	int step;
	
};

LINK_ENTITY_TO_CLASS( ghost_entity, GhostEntity );

BEGIN_DATADESC( GhostEntity )
	// Declare our think function
	DEFINE_THINKFUNC( MoveThink ),
 
END_DATADESC()