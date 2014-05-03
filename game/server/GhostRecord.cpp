#include "cbase.h"
#include "GhostRecord.h"

#include "tier0/memdbgon.h"

bool GhostRecord::playerNameDirty = true;
bool GhostRecord::mapNameDirty = true;
bool GhostRecord::firstTime = true;
bool GhostRecord::shouldRecord = false;
FileHandle_t GhostRecord::myFile = NULL;
float GhostRecord::startTime = 0.0f;
float GhostRecord::nextTime = 0.0f;
char GhostRecord::playerName[256];
char GhostRecord::fileName[256];
char GhostRecord::mapName[32];

void GhostRecord::onNameChange(IConVar *var, const char* pOldValue, float fOldValue) {
	if (!((ConVar*)var)->GetString()) return;
	if (!pOldValue) return;
	if (Q_strcmp(((ConVar*)var)->GetString(), pOldValue) == 0) {
		return;
	}
	std::string test = std::string(((ConVar*)var)->GetString());
	char playerNameTest[32];
	bool changedName = false;
	while (test.find(' ') != std::string::npos) {
		Q_strcpy(playerNameTest, test.replace(test.find(' '), 1, "").c_str());
		changedName = true;
	}
	
	if (changedName) { 
		playerNameDirty = true;
		var->SetValue(playerNameTest);
		Q_strcpy(playerName, playerNameTest);
	}
}

static ConVar ghName("gh_name", "Ghost", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Sets the name of your ghost.\nThis can also be used as the base name of your files!", GhostRecord::onNameChange);

const char* GhostRecord::getGhostName() {
	return ghName.GetString();
}

void GhostRecord::writeLine(const char* map, const char* name, float ti, float x, float y, float z) {
	if (!myFile) return;
	unsigned char mapLength = strlen(map);
	unsigned char nameLength = strlen(name);
	filesystem->Write((void*)&mapLength, sizeof(mapLength), myFile);
	filesystem->Write((void*)map, mapLength, myFile);//map
	filesystem->Write((void*)&nameLength, sizeof(nameLength), myFile);
	filesystem->Write((void*)name, nameLength, myFile);
	filesystem->Write(&ti, sizeof(ti), myFile);//time
	filesystem->Write(&x, sizeof(x), myFile);//x
	filesystem->Write(&y, sizeof(y), myFile);//y
	filesystem->Write(&z, sizeof(z), myFile);//z
}

void GhostRecord::writeHeader() {
	if (!myFile) return;
	unsigned char first = 0xAF;//This is HL2. 0xAE = portal.
	filesystem->Write(&first, sizeof(first), myFile); 
	unsigned char version = 0x01;
	filesystem->Write(&version, sizeof(version), myFile); 
	unsigned char game = 0x00;//HL2
	filesystem->Write(&game, sizeof(game), myFile);
	unsigned char gr = gpGlobals->ghostRed;
	filesystem->Write(&gr, sizeof(gr), myFile);//ghost red
	unsigned char gg = gpGlobals->ghostGreen;
	filesystem->Write(&gg, sizeof(gg), myFile);//ghost green
	unsigned char gb = gpGlobals->ghostBlue;
	filesystem->Write(&gb, sizeof(gb), myFile);//ghost blue
	unsigned char tr = gpGlobals->trailRed;
	filesystem->Write(&tr, sizeof(tr), myFile);//trail red
	unsigned char tg = gpGlobals->trailGreen;
	filesystem->Write(&tg, sizeof(tg), myFile);//trail green
	unsigned char tb = gpGlobals->trailBlue;
	filesystem->Write(&tb, sizeof(tb), myFile);//trail blue
	unsigned char tl = gpGlobals->trailLength;
	filesystem->Write(&tl, sizeof(tl), myFile);//trail length
}

void GhostRecord::endRun(float finalTickCount) {
	shouldRecord = false;
	if (myFile) {
		Msg("Run Complete!\n");
		writeLine("DONE", "DONE", finalTickCount, 0, 0, 0);
		filesystem->Flush(myFile);
		filesystem->Close(myFile);
		char newName[MAX_PATH];
		//Due to the possibility of you getting the same time
		//as another run, I must make the files have the number retained
		//in the name, to provide uniqueness. You can remove it
		//any time you want (which I really recommend).
		V_StripExtension(fileName, newName, sizeof(newName));
		Q_strcat(newName, "-", MAX_PATH);
		char finalTime[32];
		GhostUtils::getFinalTime(NULL, fileName, true, true, finalTime);
		Q_strcat(newName, finalTime, MAX_PATH);
		Q_strcat(newName, ".run", MAX_PATH);
		filesystem->Close(myFile);
		if (filesystem->RenameFile(fileName, newName, "MOD")) {
			Msg("File %s renamed to %s!\n",fileName, newName);
		}
		fileName[0] = 0;
		mapName[0] = 0;
		myFile = NULL;
	}
}


void GhostRecord::record(const CCommand &args) {
	if (shouldRecord) {
		Msg("Already recording!\n");
		return;
	}
	std::string test = std::string(ghName.GetString());
	while (test.find(' ') != std::string::npos) {
		Q_strcpy(playerName, test.replace(test.find(' '), 1, "").c_str());
		ghName.SetValue(playerName);
	}
	Q_strcpy(playerName, ghName.GetString());
	Q_strcpy(fileName, "runs/");
	if (args.ArgC() > 1 && (Q_strcmp(args.Arg(1), "") != 0)) {
		Q_strcat(fileName, args.Arg(1), sizeof(fileName));//specified name
	} else {
		//gets the next avaliable filename for starting to record.
		char fileNameGenerated[MAX_PATH];
		GhostUtils::generateFileName(playerName, fileNameGenerated);
		if (fileNameGenerated) {
			Q_strcpy(fileName, fileNameGenerated);
		}
	}
	V_SetExtension(fileName, ".run", sizeof(fileName));
	Msg("File name is %s\n", fileName);
	if (!(filesystem->FileExists(fileName, "MOD"))) {//this is for the user-specified file
		Msg("Recording to %s...\n", fileName);
		myFile = filesystem->Open(fileName, "w+b", "MOD");
		shouldRecord = true;
		firstTime = true;
		startTime = (float) Plat_FloatTime();
		nextTime = 0.0f;
		mapNameDirty = true;
		playerNameDirty = true;
	} else {
		Msg("File aready exists!\n");
	}
}

static ConCommand rec( "gh_record", GhostRecord::record, "Records a run.", 0);

void GhostRecord::stop(const CCommand &args) {
	shouldRecord = false;
	if (myFile) {
		Msg("Stopping recording...\n");
		filesystem->Close(myFile);
	}
	mapName[0] = 0;
	myFile = NULL;
}

static ConCommand stop( "gh_stop", GhostRecord::stop, "Stops recording a run.", 0);
