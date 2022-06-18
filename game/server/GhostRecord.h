#include "cbase.h"
#include "filesystem.h"
#include "GhostUtils.h"
#include <string>

class GhostRecord {

public:

	static bool playerNameDirty;
	static bool mapNameDirty;
    static bool shouldRecord;
	static bool firstTime;

	static float startTime;
	static float nextTime;

	static FileHandle_t myFile;

	static void onNameChange(IConVar*, const char*, float);

	static void writeLine(const char* map, const char* name, float ti, float x, float y, float z, float yaw);

	static void writeHeader();

	static void endRun(float);

	static const char* getGhostName();

	static void record(const CCommand&);
	static void stop(const CCommand&);

	static char playerName[256];
	static char fileName[256];
	static char mapName[32];

};

