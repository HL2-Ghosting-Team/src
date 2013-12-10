#include "cbase.h"
#include <sstream>
#pragma once


class GhostUtils {
public:
	static void splitSpaces(const char* toSplit, CUtlVector<unsigned char> &toCopyInto) {
		char toBeSplit[1000];
		Q_strcpy(toBeSplit, toSplit);
		char* parts[100] = {0};
		unsigned int index = 0;
		parts[index] = strtok(toBeSplit, " ");
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
		splitSpaces(word, vec);
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


	static bool readHeader(IFileSystem* fs, FileHandle_t myFile, unsigned char& firstByte, unsigned char& typeGhost, 
		unsigned char& ghostRed,unsigned char& ghostGreen, unsigned char& ghostBlue, 
		unsigned char& trailRed,unsigned char& trailGreen, unsigned char& trailBlue, unsigned char& trailLength) {

			//unsigned char firstByte;
			fs->Read(&firstByte, sizeof(firstByte), myFile);
			if (firstByte == 0xAF) {
				//Msg("File is uncompressed!\n");
			} //TODO add the compressed byte check
			else {
				Msg("File is malformed!\n");
				return false;
			}
			fs->Read(&typeGhost, sizeof(typeGhost), myFile); 
			fs->Read(&ghostRed, sizeof(ghostRed), myFile); 
			fs->Read(&ghostGreen, sizeof(ghostGreen), myFile); 
			fs->Read(&ghostBlue, sizeof(ghostBlue), myFile); 
			fs->Read(&trailRed, sizeof(trailRed), myFile); 
			fs->Read(&trailGreen, sizeof(trailGreen), myFile); 
			fs->Read(&trailBlue, sizeof(trailBlue), myFile); 
			fs->Read(&trailLength, sizeof(trailLength), myFile);
			return true;
	}

	static RunLine readLine(IFileSystem* fs, FileHandle_t myFile) {
		struct RunLine l;
		unsigned char mapNameLength;
		filesystem->Read((void*)&mapNameLength, sizeof(mapNameLength), myFile);
		char* mapName = new char[mapNameLength + 1];
		filesystem->Read((void*)mapName, mapNameLength, myFile);
		mapName[mapNameLength] = 0;
		unsigned char nameLength;
		filesystem->Read((void*)&nameLength, sizeof(nameLength), myFile);
		char* playerName = new char[nameLength + 1];
		filesystem->Read((void*)playerName, nameLength, myFile);
		playerName[nameLength] = 0;
		Q_strcpy(l.map, mapName);
		Q_strcpy(l.name, playerName);
		filesystem->Read(&l.tim, sizeof(l.tim), myFile);//time
		filesystem->Read(&l.x, sizeof(l.x), myFile);//x
		filesystem->Read(&l.y, sizeof(l.y), myFile);//y
		filesystem->Read(&l.z, sizeof(l.z), myFile);//z
		delete[] mapName;
		delete[] playerName;
		return l;
	}

};


