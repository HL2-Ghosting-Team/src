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
				//Msg("Setting color to be: R: %i G: %i B: %i A: %i\n", atoi(vec[0]), atoi(vec[1]), atoi(vec[2]), atoi(vec[3]));
			} else {
				//Msg("Vector not full: size is %i ... Resetting to default!\n", vec.size());
				vec.RemoveAll();
				vec[0] = 237;
				vec[1] = 133;
				vec[2] = 60;
				//Msg("Setting color to be: R: %i G: %i B: %i A: %i\n", atoi(vec[0]), atoi(vec[1]), atoi(vec[2]), atoi(vec[3]));
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
		unsigned char trailLength;
		unsigned char trailRed;
		unsigned char trailGreen;
		unsigned char trailBlue;
		unsigned char ghostRed;
		unsigned char ghostGreen;
		unsigned char ghostBlue;
		unsigned char firstByte;
		CUtlVector<RunLine> RunData;
	};

	static void copyGhostData(GhostData* old, GhostData* newG) {
		newG->ghostBlue = old->ghostBlue;
		newG->ghostGreen = old->ghostGreen;
		newG->ghostRed = old->ghostRed;
		newG->trailBlue = old->trailBlue;
		newG->trailGreen = old->trailGreen;
		newG->trailRed = old->trailRed;
		newG->trailLength = old->trailLength;
		newG->RunData = old->RunData;
	}


	static bool readHeader(IFileSystem* fs, FileHandle_t myFile, unsigned char& firstByte, 
		unsigned char& ghostRed,unsigned char& ghostGreen, unsigned char& ghostBlue, 
		unsigned char& trailRed,unsigned char& trailGreen, unsigned char& trailBlue, unsigned char& trailLength) {

			fs->Read(&firstByte, sizeof(firstByte), myFile);
			if (firstByte == 0xAF) {//HL2
				fs->Read(&ghostRed, sizeof(ghostRed), myFile); 
				fs->Read(&ghostGreen, sizeof(ghostGreen), myFile); 
				fs->Read(&ghostBlue, sizeof(ghostBlue), myFile); 
				fs->Read(&trailRed, sizeof(trailRed), myFile); 
				fs->Read(&trailGreen, sizeof(trailGreen), myFile); 
				fs->Read(&trailBlue, sizeof(trailBlue), myFile); 
				fs->Read(&trailLength, sizeof(trailLength), myFile);
				return true;
			} else if (firstByte == 0xAE) {
				Warning("This is a ghost file for Portal!\n");
				return false;
			}//Portal will be 0xAE
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
		if (!(readHeader(filesystem, myFile, ghostData->firstByte, ghostData->ghostRed, ghostData->ghostGreen, ghostData->ghostBlue, 
			ghostData->trailRed, ghostData->trailGreen, ghostData->trailBlue, ghostData->trailLength))) {
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
		if (isEndRun) {//for the filename
			Q_snprintf(m_pszString, sizeof(m_pszString), "%02dh%02dm%02ds",
				(int)(m_flSecondsTime / 3600),//hours
				(int)(m_flSecondsTime / 60), //minutes
				((int)m_flSecondsTime) % 60);//seconds
		} else {
			Q_snprintf(m_pszString, sizeof(m_pszString), "%02d:%02d:%02d.%04d",
				(int)(m_flSecondsTime / 3600),//hours
				(int)(m_flSecondsTime / 60), //minutes
				((int)m_flSecondsTime) % 60,//seconds
				(int)((m_flSecondsTime - (int)m_flSecondsTime) * 10000));//millis
		}
		Q_strcpy(into, m_pszString);
	}

	//the isEndRun boolean just makes it round off and use '-'s instead of ':'s
	static void getFinalTime(GhostData* toReadFrom, const char* fileName, bool isEndRun, bool loadLess, char* into) {
		char intoTime[32];
		if (toReadFrom == NULL) {
			struct GhostData gd;
			if (openRun(fileName, &gd)) {
				float finalTime = 0.0f;
				if (loadLess) {
					int size = gd.RunData.Count();
					float offset = 0.0f;
					for (int i = 0; i < (size - 1); i++) {
						RunLine cS = gd.RunData[i];
						RunLine nS = gd.RunData[i + 1];
						float difference = (nS.tim - cS.tim);
						if (difference >= 1.0f) {//this provides a rough detection of loads
							offset += difference;
						}
					}
					finalTime = gd.RunData[gd.RunData.Count() - 1].tim - offset;
					//subtract the offsets from loads to get actual ingame time
				} else {
					finalTime = gd.RunData[gd.RunData.Count() - 1].tim;
				}
				formatTime(finalTime, isEndRun, intoTime);
				Q_strcpy(into, intoTime);
			}
		} else {
			float finalTime = 0.0f;
			if (loadLess) {
				int size = toReadFrom->RunData.Count();
				float offset = 0.0f;
				for (int i = 0; i < (size - 1); i++) {
					RunLine cS = toReadFrom->RunData[i];
					RunLine nS = toReadFrom->RunData[i + 1];
					float difference = (nS.tim - cS.tim);
					if (difference >= 1.0f) {//this provides a rough detection of loads
						offset += difference;
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


