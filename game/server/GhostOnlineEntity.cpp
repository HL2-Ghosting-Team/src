#include "cbase.h"
#include "GhostOnlineEntity.h"
#include "GhostEngine.h"
#include "ghosthud.h"
#include "GhostOnlineEngine.h"
#include "GhostOnlineRun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( ghost_online_entity, GhostOnlineEntity );

BEGIN_DATADESC(GhostOnlineEntity)

	END_DATADESC()


	void GhostOnlineEntity::Precache( void )
{
	BaseClass::Precache();
}

void GhostOnlineEntity::Spawn( void )
{
	Precache();
	if (GhostEngine::getEngine()->shouldDrawTrails()) {
		if (ghostData.trailLength > 0) {
			CreateTrail();
		}
	}
	AddEffects(EF_NODRAW);
	SetModel("models/cone.mdl");
	SetSolid( SOLID_NONE );
	SetRenderMode(kRenderTransColor);
	SetRenderColor(ghostData.ghostRed, ghostData.ghostGreen, ghostData.ghostBlue);
	SetRenderColorA(75);
	SetMoveType( MOVETYPE_NOCLIP );
	isActive = true;
	startTime = 0;
}

void GhostOnlineEntity::CreateTrail(){
	trail = CreateEntityByName("env_spritetrail");
	trail->SetAbsOrigin(GetAbsOrigin());
	trail->SetParent(this);
	trail->KeyValue("rendermode", "5");
	trail->KeyValue("spritename", "materials/sprites/laser.vmt");
	trail->KeyValue("lifetime", ghostData.trailLength);
	trail->SetRenderColor(ghostData.trailRed, ghostData.trailGreen, ghostData.trailBlue);
	trail->KeyValue("renderamt", "75");
	trail->KeyValue("startwidth", "9.5");
	trail->KeyValue("endwidth", "1.05");
	DispatchSpawn(trail);
}

void GhostOnlineEntity::Think( void )
{
	CBaseAnimating::Think();
	if (Q_strlen(m_gName) > 0) {
		HandleGhost();
		if (!IsEffectActive(EF_NODRAW)) EntityText(0, m_gName, 0);
	} else {
		Remove();
	}
	SetNextThink( gpGlobals->curtime + 0.01f );
}

void GhostOnlineEntity::SetGhostName(const char * newname) {
	if (newname) {
		Q_strcpy(m_gName, newname);
	}
}

void GhostOnlineEntity::HandleGhost() {
	if (Q_strlen(currentStep.name) > 0) {
		//if it's not on the map
		GhostOnlineRun* run = GhostOnlineEngine::getEngine()->getRun(this);
		if (run) GhostHud::hud()->UpdateGhost((size_t)run, 0, currentStep.map);
		if (Q_strcmp(STRING(gpGlobals->mapname), currentStep.map) != 0) {
			if (!IsEffectActive(EF_NODRAW)) AddEffects(EF_NODRAW);
			return;
		}
		if (IsEffectActive(EF_NODRAW)) RemoveEffects(EF_NODRAW);
	}//TODO else { AddEffect(EF_NODRAW)
}

void GhostOnlineEntity::updateStep(OnlineRunLine step) {
	if (Q_strlen(currentStep.map) > 0) {
		if (Vector(currentStep.locX, currentStep.locY, currentStep.locZ) == Vector(step.locX, step.locY, step.locZ)) {
			SetAbsVelocity(Vector(0,0,0));
			return;
		}
	}
	currentStep = step;
	SetAbsOrigin(Vector(currentStep.locX, currentStep.locY, currentStep.locZ - 15.0f));
	SetLocalVelocity(Vector(currentStep.velX, currentStep.velY, currentStep.velZ));
}

void GhostOnlineEntity::StartRun() {
	SetNextThink(gpGlobals->curtime + 0.005f);
}

void GhostOnlineEntity::EndRun() {
	SetNextThink(0.0f);
	if (trail) trail->Remove();
	Remove();
	isActive = false;
}