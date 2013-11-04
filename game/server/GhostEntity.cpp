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
	BaseClass::Precache();
}

void splitSpaces(const char* toSplit, std::vector<char*> &toCopyInto) {
	char toBeSplit[1000];
	strcpy(toBeSplit, toSplit);
	char* parts[100] = {0};
	unsigned int index = 0;
	parts[index] = strtok(toBeSplit, " ");
	while(parts[index] != 0)
	{
		//Msg("Adding the value: %s\n", parts[index]);
		toCopyInto.push_back(parts[index]);
		++index;
		parts[index] = strtok(0, " ");
    }  
}

static ConVar drawtrails("gh_draw_trails", "1", 
						 FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						 "Draws a trail on each ghost.");

static ConVar ghostColor("gh_ghost_color", "237 133 60", 
						 FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						 "The R G B (0 - 255) values for color for your ghost.");

static ConVar ghostType("gh_ghost_type", "0", 
						FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, 
						"Sets the type of ghost model.\n0 = Solid-fill arrow | 1 = Translucent-style arrow");

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
	if (ghostType.GetBool()) {
		//Msg("Setting the model to the translucent kind!\n");
		SetModel("models/conet.mdl");
	} else {
		//Msg("Setting the model to the solid fill kind!\n");
		SetModel("models/cone.mdl");
	}//TODO look into a gh_set_ghost_model con command
	SetSolid( SOLID_NONE );
	std::vector<char*> vec;
	splitSpaces(ghostColor.GetString(), vec);
	if (vec.empty()) {
		//Msg("VECTOR IS EMPTY!\n");
	} else {
		if (vec.size() == 3) {
			//Msg("Setting color to be: R: %i G: %i B: %i A: %i\n", atoi(vec[0]), atoi(vec[1]), atoi(vec[2]), atoi(vec[3]));
		} else {
			//Msg("Vector not full: size is %i ... Resetting to default!\n", vec.size());
			ghostColor.SetValue(ghostColor.GetDefault());
			vec.clear();
			splitSpaces(ghostColor.GetString(), vec);
			//Msg("Setting color to be: R: %i G: %i B: %i A: %i\n", atoi(vec[0]), atoi(vec[1]), atoi(vec[2]), atoi(vec[3]));
		}
		SetRenderColor(atoi(vec[0]), atoi(vec[1]), atoi(vec[2]));
	}
	SetMoveType( MOVETYPE_NOCLIP );
	isActive = true;
}

void GhostEntity::StartRun() {
	//Msg("Starting run with Rundata: %i, Step %i, Name %s, Starttime: %f, This: %i\n", RunData.size(), step, m_gName, startTime, this);
	SetNextThink(gpGlobals->curtime + 0.005f);
}

static ConVar spriteLength("gh_trail_length", "5", FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, "How long the trail of the ghost lasts in seconds.");
static ConVar spriteColor("gh_trail_color", "237 133 60", FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED, "The R G B values for the color of the ghost's trail.");

void GhostEntity::CreateTrail(){
	trail = CreateEntityByName("env_spritetrail");
	trail->SetAbsOrigin(GetAbsOrigin());
	trail->SetParent(this);
	trail->KeyValue("rendermode", "5");
	trail->KeyValue("spritename", "materials/sprites/laser.vmt");
	trail->KeyValue("lifetime", spriteLength.GetInt());
	trail->KeyValue("rendercolor", spriteColor.GetString());
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
		Q_strcpy(m_gModel, newmodel);
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
	SetNextThink(0.0f);
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
void GhostEntity::clearRunData() {
	RunData.clear();	
}