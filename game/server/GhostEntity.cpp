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
	m_gModel = MODEL;//TODO when online/custom models work, delete this line!
	SetModel( MODEL );
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NOCLIP );
	isActive = true;
}

void GhostEntity::StartRun() {
	SetThink(&GhostEntity::MoveThink);
	SetNextThink(gpGlobals->curtime + 0.01f);
}


void GhostEntity::updateStep() {
	const size_t runsize = RunData.size();
	if ((step < 0) || (step >= runsize)) {
		currentStep = nextStep = NULL;
		return;
	} 
	if (step == 0) step = 1;
	currentStep = &RunData[step];
	//if (strcmp(gpGlobals->mapname.ToCStr(), currentStep->map) != 0) return;//not on the same map yet
	float currentTime = (gpGlobals->curtime - startTime);
	if (currentTime > currentStep->tim) {//catching up to a fast ghost, you came in late
		int x = step + 1;
		while (++x < runsize && currentTime > RunData[x].tim);
		step = x-1;
	}
	currentStep = &RunData[step];//update it to the new step
	currentTime = (gpGlobals->curtime - startTime);//update to new time
	if (step == (runsize - 1)) {//if it's on the last step
		nextStep = NULL;
		if (step == (runsize - 1)) {
			GhostEngine::getEngine().getRun(this)->EndRun();
		}
	} else {
		nextStep = &RunData[step+1];
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void GhostEntity::MoveThink( void )
{
	EntityText(0, m_gName, 0);
	updateStep();
	HandleGhost();
	SetNextThink( gpGlobals->curtime + 0.005f );
}

void GhostEntity::HandleGhost() {
	if (currentStep == NULL) {
		GhostEngine::getEngine().getRun(this)->EndRun();
	} else {
		if (strcmp(currentStep->map, gpGlobals->mapname.ToCStr()) != 0) {
			//spawned, but not on the map (anymore). Kill it! Kill it with fire!
			//Msg("spawned, but not on the map (anymore). Kill it! Kill it with fire!\n");
			//EndRun();
		} else {
			if (!isActive) {
				DispatchSpawn(this);
			} else {
				if (nextStep != NULL) {
					if (strcmp(currentStep->map, nextStep->map) == 0) {
						//interpolate position
						float x = currentStep->x;
						float y = currentStep->y;
						float z = currentStep->z;
						if (strcmp(currentStep->name, "empty") == 0) return;
						if (strcmp(nextStep->name, "empty") == 0) return;
						if (x == 0) return;
						float x2 = nextStep->x;
						float y2 = nextStep->y;
						float z2 = nextStep->z;
						float t1 = currentStep->tim;
						float t2 = nextStep->tim;
						float scalar = ((gpGlobals->curtime - startTime) - t1) / (t2 - t1); 
						float xfinal = x + (scalar * (x2 - x));
						float yfinal = y + (scalar * (y2 - y));
						float zfinal = z + (scalar * (z2 - z));
						SetAbsOrigin(Vector(xfinal, yfinal, (zfinal + 25.0f)));
					} else {//set it to the last position before it updates to the next map
						Msg("On its last step before the next map!\n");
						SetAbsOrigin(Vector(currentStep->x, currentStep->y, (currentStep->z + 30.0f)));
					}
				}
			}
		}
	}
}

void GhostEntity::SetGhostName(const char * newname) {
	if (newname) {
		strcpy(m_gName, newname);
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

void GhostEntity::EndRun() {
	SetNextThink(0);
	SetThink(NULL);
	Remove();
	isActive = false;
}


CON_COMMAND(gh_create_blank_ghost, "Creates an instance of the sdk model entity in front of the player.")
{
	Vector vecForward;
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
			pEnt->EntityText(0, "Example Ghost", 9999);
			DispatchSpawn(pEnt);
		}
	}
}