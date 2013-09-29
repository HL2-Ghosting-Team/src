#include "cbase.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
// Spawnflags

bool shouldPlay = false;
const char*  m_gModel;
const char*  m_gName;
bool	m_bActive;
int idx = -1;
int lineIndex = -1;
double startTime = 0.00;
double nextTime = 0.00;
std::ifstream myFile;
std::string strings[999999];//TODO make this determined on file size

class CGhostEntity : public CBaseAnimating
{
public:
	DECLARE_CLASS( CGhostEntity, CBaseAnimating );
	DECLARE_DATADESC();
 
	CGhostEntity()
	{
		SetMoveType(MOVETYPE_NOCLIP );
		m_bActive = false;
		m_gName = "Ghost Entity";
		m_gModel = "models/cone.mdl";
	}
	CGhostEntity(char* name, char* model) {
		m_bActive = false;
		m_gName = name;
		m_gModel = model;
	}
	void Spawn( void );
	void Precache( void );
	void MoveThink( void );
	void SetGhostName( char* );
	void ChangeModel( char* );
	void Move( float*, float*, float*);
	
	// Input function
	void InputToggle( inputdata_t &inputData );
 
private:
	float	m_flNextChangeTime;
	
};

CBaseEntity ghosts[50];

LINK_ENTITY_TO_CLASS( ghost_entity, CGhostEntity );

// Start of our data description for the class
BEGIN_DATADESC( CGhostEntity )
 
	// Save/restore our active state
	DEFINE_FIELD( m_flNextChangeTime, FIELD_TIME ),
 
	// Links our input name from Hammer to our input member function
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
 
	// Declare our think function
	DEFINE_THINKFUNC( MoveThink ),
 
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CGhostEntity::Precache( void )
{
	PrecacheModel( m_gModel );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CGhostEntity::Spawn( void )
{
	Precache();
	SetModel( m_gModel );
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NOCLIP );
}

unsigned int split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    unsigned int pos = txt.find( ch );
    unsigned int initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos + 1 ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
	strs.push_back( txt.substr( initialPos, min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void CGhostEntity::MoveThink( void )
{
	EntityText(0, m_gName, 0);
	std::string first, map, name, state;
	double time_ = Plat_FloatTime() - startTime;
	double curTime = ((int)((time_ + .005) * 100)) / 100.0;
	// See if we should change direction again
	if ( nextTime >= (curTime - .01) || nextTime <= (curTime + .01)) //a +- .01 interpolation, compensation for rounding
	{
		std::vector<std::string> vec;
		split(strings[lineIndex + 1], vec, ' ');
		first = vec[0];
		std::string stop = "STOP";
		if ( first.find(stop) != std::string::npos ) {
			//we're done, it found the "STOP"
			SetThink(NULL);
			myFile.close();
			return;
		}
		map = vec[1];
		name = vec[2]; // TODO check to see if it's actually the right ghost
		/*if ( map.find(gpGlobals->mapname.ToCStr()) != std::string::npos) {
		    TODO check maps and spawn accordingly
		}*/
		double gh_time;
		float loc_x, loc_y, loc_z;
		std::istringstream ss(vec[4]);
		if (!(ss >> gh_time)) {
			Msg("Cannot assign gh_time!\n");
		}
		std::istringstream ss1(vec[5]);
		if (!(ss1 >> loc_x)) {
			//error
		}
		std::istringstream ss2(vec[6]);
		if (!(ss2 >> loc_y)) {
			//error
		}
		std::istringstream ss3(vec[7]);
		if (!(ss3 >> loc_z)) {
			//error
		}
		Vector loc(loc_x, loc_y, loc_z);
		SetAbsOrigin(loc);
		/*Vector vecNewVelocity = RandomVector( -64.0f, 64.0f );
		SetAbsVelocity( vecNewVelocity );*/
 
		// Randomly change it again within one to three seconds
		//TODO change this to not be random
		nextTime = curTime + 0.05;
	}
 
	// Think at 100Hz
	//TODO make a global var for setting the frequency
	SetNextThink( nextTime );
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the movement of the entity
//-----------------------------------------------------------------------------
void CGhostEntity::InputToggle( inputdata_t &inputData )
{
	// Toggle our active state
	if ( !m_bActive )
	{
		// Start thinking
		SetThink( &CGhostEntity::MoveThink );
		// set our next think
		SetNextThink( gpGlobals->curtime + 0.01f );
 
		// Not affected by anything but moves through the world
		SetMoveType( MOVETYPE_NOCLIP );
 
		// Force MoveThink() to choose a new speed and direction immediately
		m_flNextChangeTime = gpGlobals->curtime;
 
		// Update m_bActive to reflect our new state
		m_bActive = true;
	}
	else
	{
		// Stop thinking
		SetThink( NULL );
 
		// Stop moving
		SetAbsVelocity( vec3_origin );
 		SetMoveType( MOVETYPE_NONE );
 
		m_bActive = false;
	}
}

void CGhostEntity::SetGhostName(char * newname) {
	if (newname) {
		m_gName = newname;
	}
}

void CGhostEntity::ChangeModel(char * newmodel) {
	if (newmodel) {
		m_gModel = newmodel;
		PrecacheModel(m_gModel);
		SetModel(m_gModel);
	}
}

void CGhostEntity::Move(float * newx, float * newy, float* newz) {



}

void setArray() {
	if (myFile) {
		for(int i = 0; myFile.good(); i++)
		{
			std::getline(myFile, strings[i]);
			std::cout << strings << std::endl;    
		}
	}
}

CBaseEntity * create_ghost_entity(const char * name, const char* model) {
	if (model == NULL) {
		m_gModel = "models/cone.mdl";
	} else {
		if ( model ) {
			m_gModel = model;
		}
	}
	if (name == NULL) {
		m_gName = "Ghosting Entity";
	} else {
		if (name) {
			m_gName = name;
		}
	}
	Vector vecForward;
	CBaseEntity *pEnt = CreateEntityByName( m_gName );
	CBasePlayer * pPlayer = UTIL_GetLocalPlayer();
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
		DispatchSpawn(pEnt);
	}
	return pEnt;
}

void startRun_f (const CCommand &args) {
	if ( args.ArgC() < 1 || args.Arg(1) == "" || shouldPlay) {
		return;
	}
	if (args.ArgC() < 2 || args.Arg(2) == "") {
		m_gName = args.Arg(1);
	}
	if (args.Arg(2) != "") {
		m_gName = args.Arg(2);
	}
	myFile = std::ifstream(args.Arg(1));
	if ( myFile ) {
		myFile.open(args.Arg(1), std::ios::in);
		CBaseEntity * ent = create_ghost_entity(m_gName, NULL);
		if ( ent ) {
			setArray();
			ghosts[idx + 1] = ent;
			startTime = Plat_FloatTime();//TODO make each ghost have its own start
			ent->SetThink(&CGhostEntity::MoveThink);
			idx++;
		}
	} else {
		Msg("Could not load %s for reading!\n", args.Arg(1));
		return;
	}
	
}

ConCommand start("gh_play", startRun_f, "Play back a run you did.", 0);

CON_COMMAND(gh_create_blank_ghost, "Creates an instance of the sdk model entity in front of the player.")
{
	Vector vecForward;
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if(!pPlayer)
	{
		Warning("Could not determine calling player!\n");
		return;
	}
	AngleVectors( pPlayer->EyeAngles(), &vecForward );
	create_ghost_entity(NULL, NULL);
}
