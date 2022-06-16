#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>

#include "vgui_helpers.h"

static ConVar gh_hud("gh_hud", "1",
					 FCVAR_CLIENTDLL | FCVAR_ARCHIVE, 
					 "Turn the Ghosts display on/off.");

static ConVar gh_hud_custom("gh_hud_custom", "0",
									   FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
									   "Turns on HudLayout.res options for GhostHud." );

class GhostHud : public CHudElement, public Panel {

	DECLARE_CLASS_SIMPLE(GhostHud, Panel);

public:
	GhostHud(const char *pElementName);
	virtual void Init();
	virtual bool ShouldDraw()
	{
		return gh_hud.GetBool() && CHudElement::ShouldDraw();
	}
	void MsgFunc_GhostHud_AddGhost(bf_read &msg);
	void MsgFunc_GhostHud_RemoveGhost(bf_read &msg);
	void MsgFunc_GhostHud_UpdateGhost(bf_read &msg);

	virtual void Paint();

private:
	int initialTall;
	struct GhostData {
		size_t runPtr;
		char name[32];
		char map[32];
		int step;
		float flYawDelta;
		float flDistance;
	};
	CUtlVector<GhostData> ghosts;

protected:
	CPanelAnimationVar(float, m_flBlur, "Blur", "0");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");
	CPanelAnimationVar(Color, m_Ammo2Color, "Ammo2Color", "FgColor");

	CPanelAnimationVar(HFont, m_hNumberFont, "NumberFont", "HudNumbers");
	CPanelAnimationVar(HFont, m_hNumberGlowFont, "NumberGlowFont", 
		"HudNumbersGlow");
	CPanelAnimationVar(HFont, m_hSmallNumberFont, "SmallNumberFont", 
		"HudNumbersSmall");
	CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");

	CPanelAnimationVar(Color, m_GhostNameColor, "GhostNameColor", "FgColor");
	CPanelAnimationVar(Color, m_GhostDirColor, "GhostDirColor", "255 255 255 150");
	CPanelAnimationVar(Color, m_GhostDistColor, "GhostDistColor", "255 255 255 150");
	CPanelAnimationVar(Color, m_GhostMapColor, "GhostMapColor", "255 255 255 150");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8", 
		"proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20", 
		"proportional_float");
	CPanelAnimationVarAliasType(float, digit_xpos, "digit_xpos", "50", 
		"proportional_float");
	CPanelAnimationVarAliasType(float, digit_ypos, "digit_ypos", "2", 
		"proportional_float");

};

DECLARE_HUDELEMENT(GhostHud);
DECLARE_HUD_MESSAGE(GhostHud, GhostHud_AddGhost);
DECLARE_HUD_MESSAGE(GhostHud, GhostHud_RemoveGhost);
DECLARE_HUD_MESSAGE(GhostHud, GhostHud_UpdateGhost);

GhostHud::GhostHud(const char *pElementName) :
	CHudElement(pElementName), Panel(NULL, "GhostHud")
{
	SetParent(g_pClientMode->GetViewport());
}

void GhostHud::Init()
{
	HOOK_HUD_MESSAGE(GhostHud, GhostHud_AddGhost);
	HOOK_HUD_MESSAGE(GhostHud, GhostHud_RemoveGhost);
	HOOK_HUD_MESSAGE(GhostHud, GhostHud_UpdateGhost);
	initialTall = 48;
}


void GhostHud::MsgFunc_GhostHud_RemoveGhost(bf_read &msg)  {
	size_t ptr = msg.ReadLong();

	int size = ghosts.Count();
	for (int i = 0; i < size; i++)
		if (ghosts[i].runPtr == ptr) {
			ghosts.Remove(i);
			break;
		}
}

void splitByDelimiter(const char* toSplit, const char* delim, CUtlVector<const char*> &toCopyInto) {
	char toBeSplit[1000];
	strcpy(toBeSplit, toSplit);
	char* parts[100] = {0};
	unsigned int index = 0;
	parts[index] = strtok(toBeSplit, delim);
	while(parts[index] != 0)
	{
		toCopyInto.AddToTail(parts[index]);
		++index;
		parts[index] = strtok(0, delim);
	}  
}

void GhostHud::MsgFunc_GhostHud_AddGhost(bf_read &msg) {
	struct GhostData gd;
	memset(&gd, 0, sizeof(gd));

	gd.runPtr = msg.ReadLong();
	msg.ReadString(gd.name, sizeof(gd.name));
	msg.ReadString(gd.map, sizeof(gd.map));

	ghosts.AddToTail(gd);
}

void GhostHud::MsgFunc_GhostHud_UpdateGhost(bf_read &msg) {
	//TODO update map and step (maybe calculate time)

	size_t ptr = msg.ReadLong();
	int size = ghosts.Count();
	for (int i = 0; i < size; i++ ) {
		GhostData &gh = ghosts[i];

		if (gh.runPtr == ptr) {
			gh.step = msg.ReadLong();
			msg.ReadString(gh.map, sizeof(gh.map));
			gh.flYawDelta = msg.ReadFloat();
			gh.flDistance = msg.ReadFloat();
			break;
		}
	}
}

template<class T, size_t nSize>
void EllipsString(T(&newString)[nSize], char *pszString, int nMaxLen, bool bBack = false)
{
	int nStrLen = strlen(pszString);

	memset(newString, NULL, nSize);

	if (nMaxLen > nStrLen)
	{
		memcpy_s(newString, nSize, pszString, nStrLen);
		return;
	}

	char pszNewString[256];
	memset(pszNewString, NULL, sizeof(pszNewString));

	if (bBack)
	{
		memset(pszNewString, '.', 3);

		for (int i = nStrLen - nMaxLen, i2 = 0; i < nStrLen; i++, i2++)
			pszNewString[i2 + 3] = pszString[i];
	}
	else
		for (int i = 0; i < nStrLen; i++)
			if (i >= nMaxLen && i < nMaxLen + 3)
				pszNewString[i] = '.';
			else if (i < nMaxLen)
				pszNewString[i] = pszString[i];
			else
				pszNewString[i] = NULL;

			memcpy_s(newString, nSize, pszNewString, sizeof(newString));
}

template<class T, size_t nSize>
void GetCurrentMap(T(&mapName)[nSize])
{
	memset(mapName, NULL, nSize);
	memcpy(mapName, engine->GetLevelName() + 5, nSize);

	int nStrLen = strlen(mapName);

	memset(mapName + nStrLen - 4, NULL, 4);
}

void GhostHud::Paint(void) {
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());
	float yPos = text_ypos;
	int size = ghosts.Count();
	if (size > 0) {
		if (!gh_hud_custom.GetBool())
			if (size >= 3) {
				SetTall(initialTall + (15 * (size - 2)));
			} else {
				SetTall(initialTall);
			}
		for (int i = 0; i < size; i++) {
			wchar_t name[32U];

			char nameEl[256];
			int nMaxNameLength = 10, nMaxMapLength = 15;

			EllipsString(nameEl, ghosts[i].name, nMaxNameLength, false);
			g_pVGuiLocalize->ConvertANSIToUnicode(nameEl, name, sizeof(name));

			surface()->DrawSetTextPos(text_xpos, yPos);
			surface()->DrawSetTextColor(m_GhostNameColor);
			surface()->DrawPrintText(name, wcslen(name));

			char szCurMap[128];
			GetCurrentMap(szCurMap);

			if (!Q_strcmp(szCurMap, ghosts[i].map))
			{
				wchar_t wszChar = L'?';
				float flAdy = ghosts[i].flYawDelta;

				if ((flAdy >= 0 && flAdy <= 50) || (flAdy <= 0 && flAdy >= -50))
					wszChar = L'↑';
				else if ((flAdy >= 130 && flAdy <= 180) || (flAdy <= -130 && flAdy >= -180))
					wszChar = L'↓';
				else if (flAdy < 0)
					wszChar = L'←';
				else if (flAdy > 0)
					wszChar = L'→';

				surface()->DrawSetTextPos(nMaxNameLength * 10 + 25, yPos);
				surface()->DrawSetTextColor(m_GhostDirColor);
				surface()->DrawUnicodeChar(wszChar);	// arrow

				surface()->DrawSetTextPos(nMaxNameLength * 10 + 45, yPos);

				wchar_t wszDist[16];
				swprintf(wszDist, L"%im", (int)(ghosts[i].flDistance * 0.019f));

				surface()->DrawSetTextColor(m_GhostDistColor);
				surface()->DrawUnicodeString(wszDist);	// distance in meters
			}
			else
			{
				wchar_t map[32U];
				char mapEl[256];

				EllipsString(mapEl, ghosts[i].map, nMaxMapLength, true);
				g_pVGuiLocalize->ConvertANSIToUnicode(mapEl, map, sizeof(map));

				surface()->DrawSetTextPos(nMaxNameLength * 10 + 45, yPos);
				surface()->DrawSetTextColor(m_GhostMapColor);
				surface()->DrawPrintText(map, wcslen(map));		// map name
			}

			yPos += 15.0f;
		}
	}
}