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
					 FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_DEMO, 
					 "Turn the Ghosts display on/off.");

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
	char string[256];
	msg.ReadString(string, sizeof(string), true);
	size_t ptr = Q_atoi(string);
	int size = ghosts.Count();
	for (int i = 0; i < size; i++) {
		if (ghosts[i].runPtr == ptr) {
			ghosts.Remove(i);
			break;
		}
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
	char string[255];
	msg.ReadString(string, sizeof(string), true);
	CUtlVector<const char*> vec;
	splitByDelimiter(string, ":", vec);
	size_t ptr = Q_atoi(vec[0]);
	gd.runPtr = ptr;
	Q_strcpy(gd.name, vec[1]);
	Q_strcpy(gd.map, vec[2]);
	gd.step = 0;
	ghosts.AddToTail(gd);
}

void GhostHud::MsgFunc_GhostHud_UpdateGhost(bf_read &msg) {
	//TODO update map and step (maybe calculate time)
	char string[255];
	msg.ReadString(string, sizeof(string), true);
	CUtlVector<const char*> vec;
	splitByDelimiter(string, ":", vec);
	size_t ptr = Q_atoi(vec[0]);
	int size = ghosts.Count();
	for (int i = 0; i < size; i++ ) {
		if (ghosts[i].runPtr == ptr) {
			Q_strcpy(ghosts[i].map, vec[2]);
			ghosts[i].step = Q_atoi(vec[1]);
			break;
		}
	}
}


void GhostHud::Paint(void) {
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());
	float yPos = text_ypos;
	int size = ghosts.Count();
	if (size > 0) {
		if (size > 3) {
			SetTall(initialTall + (25 * (size - 3)));
		} else {
			SetTall(initialTall);
		}
		for (int i = 0; i < size; i++) {
			wchar_t name[32U];
			wchar_t map[32U];
			surface()->DrawSetTextPos(text_xpos, yPos);
			g_pVGuiLocalize->ConvertANSIToUnicode(ghosts[i].name, name, sizeof(name));
			g_pVGuiLocalize->ConvertANSIToUnicode(ghosts[i].map, map, sizeof(map));
			surface()->DrawPrintText(name, wcslen(name));
			int x = 0, y = 0;
			surface()->DrawGetTextPos(x, y);
			surface()->DrawSetTextPos(x + 10.0f, yPos);
			surface()->DrawPrintText(map, wcslen(map));
			yPos += 15.0f;
		}
	}
}