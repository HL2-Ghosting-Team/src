#ifndef TIMER_H_
#define TIMER_H_

#include "cbase.h"
#include "fasttimer.h"
#include "filesystem.h"
#include "utlbuffer.h"
#include "GhostEngine.h"

extern IFileSystem *filesystem;

#ifdef WIN32
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

class BlaTimer
{
public:
	BlaTimer() : m_bIsRunning(false), m_flSecondsRecord(0.0f)
	{
		m_vStart.Init();
		m_vGoal.Init();
	}
	~BlaTimer() {}

	static BlaTimer *timer()
	{
		static BlaTimer *timer = new BlaTimer();
		return timer;
	}

	void Init()
	{
		m_ftTimer.End();
		m_ftTimer = CFastTimer(); // CFastTimer has no reset method.
		m_bIsRunning = false;

		// Make sure the maps directory is there (you never know...).
		filesystem->CreateDirHierarchy("maps", "MOD");

		CUtlBuffer buffer;
		char *pszPath = GetMapFilePath();
		DevMsg("Trying to get record: %s\n", pszPath);
		if (filesystem->ReadFile(pszPath, "MOD", buffer))
		{
			char *pszRecord = new char[buffer.Size() + 1];
			buffer.GetString(pszRecord);
			pszRecord[buffer.Size()] = '\0';
			if (sscanf(pszRecord, "%f %f %f" NEWLINE "%f %f %f" NEWLINE "%f", 
				&m_vStart.x, &m_vStart.y, &m_vStart.z,
				&m_vGoal.x, &m_vGoal.y, &m_vGoal.z,
				&m_flSecondsRecord) != 7)
				Warning("Record file %s is malformed\n", pszPath);
			delete[] pszRecord;
		}
		delete[] pszPath;

		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		if (!pPlayer)
			return;
		//pPlayer->SetStartPosition(m_vStart);
		//pPlayer->SetGoalPosition(m_vGoal);
	}

	void Start()
	{
		m_ftTimer.Start();
		m_bIsRunning = true;
		DispatchStateChangeMessage();
	}

	void Stop()
	{
		m_ftTimer.End();
		m_bIsRunning = false;
		DispatchStateChangeMessage();
		float flSecondsTime = GetCurrentTime();
		if (flSecondsTime < m_flSecondsRecord || m_flSecondsRecord == 0.0f)
		{
			m_flSecondsRecord = flSecondsTime;
			DevMsg("New map record: %.4f seconds\n", m_flSecondsRecord);
			WriteMapFile();
		}
	}

	bool IsRunning()
	{
		return m_bIsRunning;
	}

	float GetCurrentTime()
	{
		CCycleCount ccCycles;
		if (m_bIsRunning)
			ccCycles = m_ftTimer.GetDurationInProgress();
		else
			ccCycles = m_ftTimer.GetDuration();
		return static_cast<float>(ccCycles.GetSeconds());;
	}

	void DispatchTimeToBeatMessage()
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "BlaTimer_TimeToBeat");
		WRITE_FLOAT(m_flSecondsRecord);
		MessageEnd();
	}

	void DispatchTimeMessage()
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "BlaTimer_Time");
		WRITE_FLOAT(GetCurrentTime());
		MessageEnd();
	}

	void RemoveGhost(size_t ptr) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "BlaTimer_RemoveGhost");
		char string[255];
		Q_snprintf(string, sizeof(string), "%i", ptr);
		WRITE_STRING(string);
		MessageEnd();
	}

	void AddGhost(size_t ptr, const char* name, const char* map) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "BlaTimer_AddGhost");
		char totalString[255];
		Q_snprintf(totalString, sizeof(totalString), "%i:%s:%s", ptr, name, map);
		WRITE_STRING(totalString);
		MessageEnd();
	}

	void UpdateGhost(size_t ptr, int currentStep, const char* map) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "BlaTimer_UpdateGhost");
		char totalString[255];
		Q_snprintf(totalString, sizeof(totalString), "%i:%i:%s", ptr, currentStep, map);
		WRITE_STRING(totalString);
		MessageEnd();
	}

	void SetStartPosition(Vector start)
	{
		//UTIL_GetLocalPlayer()->SetStartPosition(start);
		m_vStart = start;
		WriteMapFile();
	}

	void SetGoalPosition(Vector goal)
	{
		// UTIL_GetLocalPlayer()->SetGoalPosition(goal);
		m_vGoal = goal;
		WriteMapFile();
	}

private:
	Vector m_vStart;
	Vector m_vGoal;
	CFastTimer m_ftTimer;
	CCycleCount m_ccCycles;
	bool m_bIsRunning;
	bool m_IsPaused;
	float m_flSecondsRecord;

	// Inform the HUD about status changes of the timer so it can fire up some
	// fancy animation effects.
	void DispatchStateChangeMessage()
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();

		UserMessageBegin(user, "BlaTimer_StateChange");
		WRITE_BOOL(m_bIsRunning);
		MessageEnd();
	}

	void WriteMapFile()
	{
		char *pszPath = GetMapFilePath();
		FileHandle_t fh = filesystem->Open(pszPath, "w", "MOD");
		if (fh)
		{
			filesystem->FPrintf(fh, "%f %f %f%s", 
				m_vStart.x, m_vStart.y, m_vStart.z, NEWLINE);
			filesystem->FPrintf(fh, "%f %f %f%s", 
				m_vGoal.x, m_vGoal.y, m_vGoal.z, NEWLINE);
			filesystem->FPrintf(fh, "%f", m_flSecondsRecord);
			filesystem->Close(fh);
		}
		delete[] pszPath;
	}

	// Caller is responsible for delete[]'ing the array.
	char *GetMapFilePath()
	{
		const char *pszMapname = (gpGlobals->mapname).ToCStr();
		size_t sz = Q_strlen("maps/") + strlen(pszMapname) + Q_strlen(".bla") + 1;
		char *pszPath = new char[sz];
		Q_strncpy(pszPath, "maps/", sz);
		Q_strncat(pszPath, pszMapname, sz);
		Q_strncat(pszPath, ".bla", sz);
		Q_FixSlashes(pszPath);
		return pszPath;
	}
};

#undef NEWLINE

#endif // TIMER_H_