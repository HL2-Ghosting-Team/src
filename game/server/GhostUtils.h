#include "cbase.h"
#include <sstream>
#include "filesystem.h"
#include "runline.h"
#include "Color.h"
#pragma once


class GhostUtils {
public:
	static void splitByDelimeter(const char* toSplit, const char* delim, CUtlVector<unsigned char> &toCopyInto) {
		char toBeSplit[1000];
		Q_strcpy(toBeSplit, toSplit);
		char* parts[100] = {0};
		unsigned int index = 0;
		parts[index] = strtok(toBeSplit, delim);
		while(parts[index] != 0)
		{
			toCopyInto.AddToTail(Q_atoi(parts[index]));
			++index;
			parts[index] = strtok(0, " ");
		}  
	}

	static void fixInts(CUtlVector<unsigned char> &vec) {
		if (vec.Count() == 3) {
			if (vec[0] > 255 || vec[0] < 0) {
				vec[0] = 237;
			}
			if (vec[1] > 255 || vec[1] < 0) {
				vec[1] = 133;
			}
			if (vec[2] > 255 || vec[2] < 0) {
				vec[2] = 60;
			}
		} else {
			vec.RemoveAll();
			vec[0] = 237;
			vec[1] = 133;
			vec[2] = 60;
		}
	}

	//Used to check the ghost and trail colors, and reset them back to default if not appropriate
	//and assigns the r, g, b values to the vector.
	static void getColor(const char* word, CUtlVector<unsigned char> &vec) {
		if (!word) {
			vec[0] = 237;
			vec[1] = 133;
			vec[2] = 60;
		}
		splitByDelimeter(word, " ", vec);
		if (vec.Count() == 0) {//default to orange
			vec[0] = 237;
			vec[1] = 133;
			vec[2] = 60;
		} else {
			if (vec.Count() == 3) {
				fixInts(vec);
			} else {
				vec.RemoveAll();
				vec[0] = 237;
				vec[1] = 133;
				vec[2] = 60;
			}
		}
	}

	static int FileAutoCompleteList ( char const *partial, 
		char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
	{
		char fileDir[MAX_PATH];
		int toReturn = 0;
		char part[MAX_PATH];
		char* toSearch[2] = {0};
		Q_strcpy(part, partial);
		toSearch[0] = strtok(part, " ");
		toSearch[1] = strtok(0, " ");
		if (UTIL_GetModDir(fileDir, MAX_PATH)) {
			FileFindHandle_t findHandle; // note: FileFINDHandle
			std::stringstream ss1;
			ss1 << (toSearch[1] == 0 ? "" : toSearch[1]) << "*.run";
			const char *pFilename = filesystem->FindFirstEx( ss1.str().c_str(), "MOD", &findHandle );
			for(int i = 0; pFilename; i++) {
				std::stringstream ss;
				ss << "gh_listrun " << pFilename;
				Q_strcpy(commands[i], ss.str().c_str());
				pFilename = filesystem->FindNext(findHandle);
				toReturn = i + 1;
			}
			filesystem->FindClose(findHandle);
		}
		return toReturn; // number of entries
	}


	static int FileAutoCompleteLoad ( char const *partial, 
		char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
	{
		char fileDir[MAX_PATH];
		int toReturn = 0;
		char part[MAX_PATH];
		char* toSearch[2] = {0};
		Q_strcpy(part, partial);
		toSearch[0] = strtok(part, " ");
		toSearch[1] = strtok(0, " ");
		if (UTIL_GetModDir(fileDir, MAX_PATH)) {
			FileFindHandle_t findHandle; // note: FileFINDHandle
			std::stringstream ss1;
			ss1 << (toSearch[1] == 0 ? "" : toSearch[1]) << "*.run";
			const char *pFilename = filesystem->FindFirstEx( ss1.str().c_str(), "MOD", &findHandle );
			for(int i = 0; pFilename; i++) {
				std::stringstream ss;
				ss << "gh_load " << pFilename;
				Q_strcpy(commands[i], ss.str().c_str());
				pFilename = filesystem->FindNext(findHandle);
				toReturn = i + 1;
			}
			filesystem->FindClose(findHandle);
		}
		return toReturn; // number of entries
	}

	static int FileAutoComplete ( char const *partial, 
		char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
	{
		char fileDir[MAX_PATH];
		int toReturn = 0;
		char part[MAX_PATH];
		char* toSearch[2] = {0};
		Q_strcpy(part, partial);
		toSearch[0] = strtok(part, " ");
		toSearch[1] = strtok(0, " ");
		if (UTIL_GetModDir(fileDir, MAX_PATH)) {
			FileFindHandle_t findHandle; // note: FileFINDHandle
			std::stringstream ss1;
			ss1 << (toSearch[1] == 0 ? "" : toSearch[1]) << "*.run";
			const char *pFilename = filesystem->FindFirstEx( ss1.str().c_str(), "MOD", &findHandle );
			for(int i = 0; pFilename; i++) {
				std::stringstream ss;
				ss << "gh_play " << pFilename;
				Q_strcpy(commands[i], ss.str().c_str());
				pFilename = filesystem->FindNext(findHandle);
				toReturn = i + 1;
			}
			filesystem->FindClose(findHandle);
		}
		return toReturn; // number of entries
	}

	static struct GhostData {
		unsigned char version;
		unsigned char game;
		unsigned char trailLength;
		unsigned char trailRed;
		unsigned char trailGreen;
		unsigned char trailBlue;
		unsigned char ghostRed;
		unsigned char ghostGreen;
		unsigned char ghostBlue;
		unsigned char firstByte;
		CUtlVector<RunLine> RunData;

		GhostData(){};

		GhostData(const GhostData& other) {
			firstByte = other.firstByte;
			version = other.version;
			game = other.game;
			ghostRed = other.ghostRed;
			ghostGreen = other.ghostGreen;
			ghostBlue = other.ghostBlue;
			trailLength = other.trailLength;
			trailRed = other.trailRed;
			trailGreen = other.trailGreen;
			trailBlue = other.trailBlue;
			RunData = other.RunData;
		};

		GhostData& operator=(const GhostData& other) {
			firstByte = other.firstByte;
			version = other.version;
			game = other.game;
			ghostRed = other.ghostRed;
			ghostGreen = other.ghostGreen;
			ghostBlue = other.ghostBlue;
			trailLength = other.trailLength;
			trailRed = other.trailRed;
			trailGreen = other.trailGreen;
			trailBlue = other.trailBlue;
			RunData = other.RunData;
			return *this;
		};
	};

	static bool readHeader(IFileSystem* fs, FileHandle_t myFile, GhostData* data) {
		fs->Read(&data->firstByte, sizeof(data->firstByte), myFile);//useless
		fs->Read(&data->version, sizeof(data->version), myFile);
		fs->Read(&data->game, sizeof(data->game), myFile);
		if (data->game == 0x00) {//HL2
			fs->Read(&data->ghostRed, sizeof(data->ghostRed), myFile); 
			fs->Read(&data->ghostGreen, sizeof(data->ghostGreen), myFile); 
			fs->Read(&data->ghostBlue, sizeof(data->ghostBlue), myFile); 
			fs->Read(&data->trailRed, sizeof(data->trailRed), myFile); 
			fs->Read(&data->trailGreen, sizeof(data->trailGreen), myFile); 
			fs->Read(&data->trailBlue, sizeof(data->trailBlue), myFile); 
			fs->Read(&data->trailLength, sizeof(data->trailLength), myFile);
			return true;
		} else if (data->game == 0x01) {
			Warning("This is a ghost file for Portal!\n");
			return false;
		}
		else {
			Warning("File is malformed!\n");
			return false;
		}
	}

	static RunLine readLine(IFileSystem* fs, FileHandle_t myFile) {
		struct RunLine l;
		unsigned char mapNameLength;
		fs->Read((void*)&mapNameLength, sizeof(mapNameLength), myFile);
		char* mapName = new char[mapNameLength + 1];
		fs->Read((void*)mapName, mapNameLength, myFile);
		mapName[mapNameLength] = 0;
		unsigned char nameLength;
		fs->Read((void*)&nameLength, sizeof(nameLength), myFile);
		char* playerName = new char[nameLength + 1];
		fs->Read((void*)playerName, nameLength, myFile);
		playerName[nameLength] = 0;
		Q_strcpy(l.map, mapName);
		Q_strcpy(l.name, playerName);
		fs->Read(&l.tim, sizeof(l.tim), myFile);//time
		fs->Read(&l.x, sizeof(l.x), myFile);//x
		fs->Read(&l.y, sizeof(l.y), myFile);//y
		fs->Read(&l.z, sizeof(l.z), myFile);//z
		delete[] mapName;
		delete[] playerName;
		return l;
	}

	static RunLine createLine(const char* name, const char* map, float tim, float x, float y, float z) {
		struct RunLine l;
		Q_strcpy(l.name, name);
		Q_strcpy(l.map, map);
		l.tim = tim;
		l.x = x;
		l.y = y;
		l.z = z;
		return l;
	}

	static bool openRun(const char* fileName, GhostData* ghostData) {
		if (!fileName) {//this is just incase
			Warning("The file name is null!\n");
			return false;
		}
		char file[256];
		Q_strcpy(file, fileName);
		V_SetExtension(file, ".run", sizeof(file));
		FileHandle_t myFile = filesystem->Open(file, "rb", "MOD");
		if (myFile == NULL) {
			Warning("File is null!\n");
			return false;
		}
		if (!(readHeader(filesystem, myFile, ghostData))) {
			Warning("Could not read the header!\n");
			return false;
		}
		while (!filesystem->EndOfFile(myFile)) {
			struct RunLine l = readLine(filesystem, myFile);
			ghostData->RunData.AddToTail(l);
		}
		return true;
	}


	static int getFileCount(const char* searchkey) {
		char fileName[256];
		Q_strcpy(fileName, searchkey);
		Q_strcat(fileName, "_*.run", 256);
		int toReturn = 0;
		FileFindHandle_t findHandle; // note: FileFINDHandle
		const char *pFilename = filesystem->FindFirstEx(fileName, "MOD", &findHandle );
		for(int i = 0; pFilename; i++) {
			pFilename = filesystem->FindNext(findHandle);
			toReturn = i + 1;
		}
		filesystem->FindClose(findHandle);
		return toReturn; // number of entries
	}

	static void generateFileName(const char* nameKey, char* into) {
		std::stringstream sstr;
		int count = GhostUtils::getFileCount(nameKey);
		char fileish[MAX_PATH];
		count++;
		Q_snprintf(fileish, (sizeof("000") + 1), "%03d", (count));
		sstr << nameKey << "_" << fileish;
		char fileName[MAX_PATH];
		Q_strcpy(fileName, sstr.str().c_str());
		V_SetExtension(fileName, ".run", sizeof(fileName));
		while (filesystem->FileExists(fileName, "MOD")) {
			std::stringstream loopStream;
			//increment the _### suffix by 1
			count++;					
			Q_snprintf(fileish, (sizeof("000") + 1), "%03d", count);
			loopStream << nameKey << "_" << fileish;
			Q_strcpy(fileName, loopStream.str().c_str());
			V_SetExtension(fileName, ".run", sizeof(fileName));
		}
		Q_strcpy(into, fileName);
	}

	static void formatTime(float m_flSecondsTime, bool isEndRun, char* into) {
		char m_pszString[32];
		int hours = (int)(m_flSecondsTime / 3600);
		int minutes = (int)(((m_flSecondsTime / 3600) - hours) * 60);
		int seconds = (int)(((((m_flSecondsTime / 3600) - hours) * 60) - minutes) * 60);
		if (isEndRun) {//for the filename
			if (hours > 0) {
				if (minutes > 0) {
					if (seconds > 0) {
						Q_snprintf(m_pszString, sizeof(m_pszString), "%02dh%02dm%02ds",
							hours,//hours
							minutes, //minutes
							seconds);//seconds
					} else {
						Q_snprintf(m_pszString, sizeof(m_pszString), "%02dh%02dm",
						hours,//hours
						minutes); //minutes
					}
				} else {
					if (seconds > 0) {
						Q_snprintf(m_pszString, sizeof(m_pszString), "%02dh%02ds",
							hours,//hours
							seconds);//seconds
					} else {
						Q_snprintf(m_pszString, sizeof(m_pszString), "%02dh",
						hours);//hours
					}
				}
			} else {//hours is 0
				if (minutes > 0) {
					if (seconds > 0) {
						Q_snprintf(m_pszString, sizeof(m_pszString), "%02dm%02ds",
							minutes, //minutes
							seconds);//seconds
					} else {
						Q_snprintf(m_pszString, sizeof(m_pszString), "%02dm",
						minutes); //minutes
					}
				} else {
					if (seconds > 0) {
						Q_snprintf(m_pszString, sizeof(m_pszString), "%02ds",
							seconds);//seconds
					} else {
						int millis = (int)(((((((m_flSecondsTime / 3600) - hours) * 60) - minutes) * 60) - seconds) * 10000);
						Q_snprintf(m_pszString, sizeof(m_pszString), "%04dmillis", millis);
					}
				}
			}
		} else {
			int millis = (int)(((((((m_flSecondsTime / 3600) - hours) * 60) - minutes) * 60) - seconds) * 10000);
			Q_snprintf(m_pszString, sizeof(m_pszString), "%02d:%02d:%02d.%04d",
				hours,//hours
				minutes, //minutes
				seconds,//seconds
				millis);//millis
		}
		Q_strcpy(into, m_pszString);
	}

	//the isEndRun boolean just makes it round off and use '-'s instead of ':'s
	static void getFinalTime(GhostData* toReadFrom, const char* fileName, bool isEndRun, bool loadLess, char* into) {
		char intoTime[32];
		struct GhostData gd;
		if (toReadFrom == NULL) {
			if (openRun(fileName, &gd)) {
				toReadFrom = &gd;
			} else {
				Warning("Could not open run!\n");
				return;
			}
		}
		float finalTime = 0.0f;
		if (loadLess) {
			int size = toReadFrom->RunData.Count();
			float offset = 0.0f;
			for (int i = 0; i < (size - 1); i++) {
				RunLine cS = toReadFrom->RunData[i];
				RunLine nS = toReadFrom->RunData[i + 1];
				//we're only detecting loads, not pause time
				//this means we're comparing the maps to see when they're different,
				//indicating a load.
				if (Q_strlen(cS.map) == 0) {
					if (Q_strlen(nS.map) > 0) {
						float difference = (nS.tim - cS.tim);
						offset += difference;
					}
				}
			}
			finalTime = toReadFrom->RunData[toReadFrom->RunData.Count() - 1].tim - offset;
			//subtract the offsets from loads to get actual ingame time
		} else {
			finalTime = toReadFrom->RunData[toReadFrom->RunData.Count() - 1].tim;
		}
		formatTime(finalTime, isEndRun, intoTime);
		Q_strcpy(into, intoTime);
	}


	static void listRun(const char* fileName) {
		GhostData gd;
		if (openRun(fileName, &gd)) {
			Msg("Run by: %s\n", gd.RunData[0].name);
			ConColorMsg(Color((int)gd.ghostRed, (int)gd.ghostGreen, (int)gd.ghostBlue, 255), "Ghost Color\n");
			ConColorMsg(Color((int)gd.trailRed, (int)gd.trailGreen, (int)gd.trailBlue, 255), "Trail Color\n");
			if (gd.trailLength > 0) {
				Msg("Trail length: %i seconds\n", gd.trailLength);
			} else {
				Msg("No trail recorded.\n");
			}
			Msg("\n");
			char realtime[32];
			getFinalTime(&gd, NULL, false, false, realtime);
			char loadless[32];
			getFinalTime(&gd, NULL, false, true, loadless);
			Msg("Total time (realtime): %s\n", realtime);
			Msg("Total time (loadless): %s\n", loadless);
		}
	}

};


