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

BEGIN_DATADESC(GhostEntity)

	END_DATADESC()

	//-----------------------------------------------------------------------------
	// Purpose: Precache assets used by the entity
	//-----------------------------------------------------------------------------
	void GhostEntity::Precache( void )
{
	PrecacheModel( MODEL );
	BaseClass::Precache();
}

static ConVar drawtrails("gh_draw_trails", "1", 0, "Draws a trail on each ghost.");

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void GhostEntity::Spawn( void )
{
	Precache();
	if (drawtrails.GetBool()) {
		CreateTrail();
	}
	RemoveEffects(EF_NODRAW);
	m_gModel = MODEL;//TODO when online/custom models work, delete this line!
	SetModel( MODEL );//and this one!
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NOCLIP );
	isActive = true;
}

void GhostEntity::StartRun() {
	//Msg("Starting run with Rundata: %i, Step %i, Name %s, Starttime: %f, This: %i\n", RunData.size(), step, m_gName, startTime, this);
	SetNextThink(gpGlobals->curtime + 0.005f);
}

void GhostEntity::CreateTrail(){
	trail = CreateEntityByName("env_spritetrail");
	trail->SetAbsOrigin(GetAbsOrigin());
	trail->SetParent(this);
	trail->KeyValue("rendermode", "5");
	trail->KeyValue("spritename", "materials/sprites/laser.vmt");
	trail->KeyValue("lifetime", "5.00");
	trail->KeyValue("rendercolor", "237 133 60");
	trail->KeyValue("renderamt", "255");
	trail->KeyValue("startwidth", "11");
	trail->KeyValue("endwidth", "1.05");
	DispatchSpawn(trail);
}

void GhostEntity::updateStep() {
	if (inReset) return;
	const size_t runsize = RunData.size();
	if (step < 0 || step >= runsize) {
		currentStep = nextStep = NULL;
		return;
	}
	if (step == 0) step = 1;
	currentStep = &RunData[step];
	float currentTime = ((float)Plat_FloatTime() - startTime);
	if (currentTime > currentStep->tim) {//catching up to a fast ghost, you came in late
		unsigned int x = step + 1;
		while (++x < runsize && currentTime > RunData[x].tim);
		step = x-1;
	}
	currentStep = &RunData[step];//update it to the new step
	currentTime = ((float)Plat_FloatTime() - startTime);//update to new time
	if (step == (runsize - 1)) {//if it's on the last step
		GhostEngine::getEngine().getRun(this)->EndRun();
	} else {
		nextStep = &RunData[step+1];
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void GhostEntity::Think( void )
{
	CBaseAnimating::Think();
	if (Q_strlen(m_gName) != 0) {
		if (!IsEffectActive(EF_NODRAW)) EntityText(0, m_gName, 0);
		updateStep();
		HandleGhost();
	} else {
		EndRun(false);
	}
	SetNextThink( gpGlobals->curtime + 0.01f );
}

void GhostEntity::HandleGhost() {
	if (currentStep == NULL) {		
		if (!inReset) GhostEngine::getEngine().getRun(this)->EndRun();
	} 
	else {	
		if (!isActive) {
			if (Q_strcmp(currentStep->map, gpGlobals->mapname.ToCStr()) == 0) {
				DispatchSpawn(this);
			}
		} 
		else {
			if (nextStep != NULL) {
				if (Q_strcmp(gpGlobals->mapname.ToCStr(), currentStep->map) != 0) {
					if (!IsEffectActive(EF_NODRAW)) AddEffects(EF_NODRAW);
					return;
				}
				if (IsEffectActive(EF_NODRAW)) RemoveEffects(EF_NODRAW);
				if (Q_strcmp(currentStep->map, nextStep->map) == 0) {
					//interpolate position
					float x = currentStep->x;
					float y = currentStep->y;
					float z = currentStep->z;
					if (Q_strcmp(currentStep->name, "empty") == 0) return;
					if (Q_strcmp(nextStep->name, "empty") == 0) return;
					if (x == 0) return;
					float x2 = nextStep->x;
					float y2 = nextStep->y;
					float z2 = nextStep->z;
					float t1 = currentStep->tim;
					float t2 = nextStep->tim;
					float scalar = ((((float)Plat_FloatTime()) - startTime) - t1) / (t2 - t1); 
					float xfinal = x + (scalar * (x2 - x));
					float yfinal = y + (scalar * (y2 - y));
					float zfinal = z + (scalar * (z2 - z));
					SetAbsOrigin(Vector(xfinal, yfinal, (zfinal + 25.0f)));
				} else {//set it to the last position before it updates to the next map
					SetAbsOrigin(Vector(currentStep->x, currentStep->y, (currentStep->z + 30.0f)));
				}
			}
		}
	}
}

void GhostEntity::SetGhostName(const char * newname) {
	if (newname) {
		Q_strcpy(m_gName, newname);
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

void GhostEntity::EndRun(bool reset) {
	SetNextThink(0);
	if (trail) trail->Remove();
	inReset = reset;
	if (reset) {
		step = 0;
		startTime = 0.0f;
	}
	Remove();
	isActive = false;
}

void GhostEntity::SetRunData(std::vector<RunLine>& toSet) {
	RunData = toSet;
}

/*CON_COMMAND(gh_create_blank_ghost, "Creates an instance of the sdk model entity in front of the player.")
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
}*/