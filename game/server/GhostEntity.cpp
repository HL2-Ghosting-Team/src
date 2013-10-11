#include "cbase.h"
#include "GhostEntity.h"
#include "GhostEngine.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
// Spawnflags

#define MODEL "models/cone.mdl"

LINK_ENTITY_TO_CLASS( ghost_entity, GhostEntity );

BEGIN_DATADESC( GhostEntity )
	// Declare our think function
	DEFINE_THINKFUNC( MoveThink ),

	END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void GhostEntity::Precache( void )
{
	PrecacheModel( MODEL );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void GhostEntity::Spawn( void )
{
	Precache();
	m_gModel = MODEL;
	SetModel( MODEL );
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NOCLIP );
}

void GhostEntity::StartRun() {
	SetThink(&GhostEntity::MoveThink);
	startTime = gpGlobals->curtime;
	shouldQuery = true;
	SetNextThink(startTime + 0.05f);
}

void GhostEntity::DoStep() {
	step++;
}

int GhostEntity::GetCurrentStep() {
	return step;
}

void GhostEntity::SetRunData(std::vector<RunLine>& toSet) {
	RunData = toSet;
}

//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void GhostEntity::MoveThink( void )
{
	EntityText(0, m_gName, 0.06f);
	double time_ = gpGlobals->curtime - startTime;
	double curTime = ((int)((time_ + .005) * 100)) / 100.0;//round it off for comparison
	// See if we should update
	if (shouldQuery) {
		GhostEngine::getEngine().handleGhost(curTime, this);
	}
	nextTime = curTime + 0.06f;
	SetNextThink( gpGlobals->curtime + 0.0555f );
}


void GhostEntity::SetGhostName(const char * newname) {
	if (newname) {
		m_gName = newname;
	}
}

void GhostEntity::SetGhostModel(const char * newmodel) {
	if (newmodel) {
		m_gModel = newmodel;
		PrecacheModel(m_gModel);
		SetModel(m_gModel);
	}
}

const char* GhostEntity::GetGhostName() {
	return m_gName;
}

const char* GhostEntity::GetGhostModel() {
	return m_gModel;
}

void GhostEntity::SetShouldUpdate(bool update) {
	shouldQuery = update;
}

void GhostEntity::EndRun(bool shouldDestroy) {
	SetNextThink(0);
	SetThink(NULL);
	//TODO delete it if needed
}


CON_COMMAND(gh_create_blank_ghost, "Creates an instance of the sdk model entity in front of the player.")
{
	Vector vecForward;
	//CBaseEntity *pEnt = CreateEntityByName( "ghost_entity" );
	GhostEntity *pEnt = (GhostEntity*) CreateEntityByName( "ghost_entity" );
	CBasePlayer * pPlayer = UTIL_GetLocalPlayer();
	if ( pEnt )
	{
		Vector vecOrigin;
		QAngle vecAngles;
		if (pPlayer) { // if the player exists, it can spawn it on his location and orientation
			AngleVectors( pPlayer->EyeAngles(), &vecForward );
			vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0,0,64);
			vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
			pEnt->SetAbsOrigin(vecOrigin);
			pEnt->SetAbsAngles(vecAngles);
			pEnt->EntityText(0, "Example Ghost", 0);
			DispatchSpawn(pEnt);
		}
	}
}