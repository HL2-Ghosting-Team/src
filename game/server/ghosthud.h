#include "cbase.h"
#include "GhostEngine.h"


class GhostHud {
public:
	GhostHud() {}
	~GhostHud() {}

	static GhostHud * hud() {
		static GhostHud* hud = new GhostHud();
		return hud;
	}

	void RemoveGhost(size_t ptr) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "GhostHud_RemoveGhost");
		char string[255];
		Q_snprintf(string, sizeof(string), "%i", ptr);
		WRITE_STRING(string);
		MessageEnd();
	}

	void AddGhost(size_t ptr, const char* name, const char* map) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "GhostHud_AddGhost");
		char totalString[255];
		Q_snprintf(totalString, sizeof(totalString), "%i:%s:%s", ptr, name, map);
		WRITE_STRING(totalString);
		MessageEnd();
	}

	void UpdateGhost(size_t ptr, int currentStep, const char* map) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "GhostHud_UpdateGhost");
		char totalString[255];
		Q_snprintf(totalString, sizeof(totalString), "%i:%i:%s", ptr, currentStep, map);
		WRITE_STRING(totalString);
		MessageEnd();
	}

};