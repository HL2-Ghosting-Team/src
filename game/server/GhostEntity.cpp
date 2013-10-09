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
	PrecacheModel( m_gModel );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void GhostEntity::Spawn( void )
{
	Precache();
	SetModel( m_gModel );
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NOCLIP );
	if (shouldThink) {
		SetThink(&GhostEntity::MoveThink);
		SetNextThink(startTime + 0.05);
	}
	BaseClass::Spawn();
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

void GhostEntity::MoveGhost(Vector toMove) {
	SetAbsOrigin(toMove);
}

//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void GhostEntity::MoveThink( void )
{
	EntityText(0, m_gName, 0);
	double time_ = Plat_FloatTime() - startTime;
	double curTime = ((int)((time_ + .005) * 100)) / 100.0;//round it off for comparison
	// See if we should update
	GhostEngine::getEngine().handleGhost(curTime, this);
	Msg("Should read something now\n");
	nextTime = curTime + 0.05;
	SetNextThink( nextTime );
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

void GhostEntity::EndRun(bool shouldDestroy) {
	SetThink(NULL);

}

//will create a ghost but not spawn it
void GhostEntity::CreateGhost() {
	Vector vecForward;
	CBaseEntity *pEnt = CreateEntityByName( "ghost_entity" );
	CBasePlayer * pPlayer = UTIL_GetLocalPlayer();
	startTime = Plat_FloatTime();
	if ( pEnt )
	{
		Vector vecOrigin;
		QAngle vecAngles;
		if (pPlayer) { // if the player exists, it can spawn it on his location and orientation
			AngleVectors( pPlayer->EyeAngles(), &vecForward );
			vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0,0,64);
			vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
		} else {//just go 0,0,0
			vecOrigin= Vector(0, 0, 0);
			vecAngles = QAngle(0, 0, 0);
		}
		pEnt->SetAbsOrigin(vecOrigin);
		pEnt->SetAbsAngles(vecAngles);
		shouldThink = true;
	}
}


CON_COMMAND(gh_create_blank_ghost, "Creates an instance of the sdk model entity in front of the player.")
{
	Vector vecForward;
	CBaseEntity *pEnt = CreateEntityByName( "ghost_entity" );
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
			DispatchSpawn(pEnt);
		}
	}
}