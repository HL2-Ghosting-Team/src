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
		{
			WRITE_LONG(ptr);
		}
		MessageEnd();
	}

	void AddGhost(size_t ptr, const char* name, const char* map) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();

		UserMessageBegin(user, "GhostHud_AddGhost");
		{
			WRITE_LONG(ptr);
			WRITE_STRING(name);
			WRITE_STRING(map);
		}
		MessageEnd();
	}

	void UpdateGhost(size_t ptr, int currentStep, const char* map, float flYawDelta = .0f, float flDistance = .0f) {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();

		UserMessageBegin(user, "GhostHud_UpdateGhost");
		{
			WRITE_LONG(ptr);
			WRITE_LONG(currentStep);
			WRITE_STRING(map);
			WRITE_FLOAT(flYawDelta);
			WRITE_FLOAT(flDistance);
		}
		MessageEnd();
	}

};