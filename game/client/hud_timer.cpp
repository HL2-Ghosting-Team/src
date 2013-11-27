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

#define BUFSIZE (sizeof("00:00.0000")+1)

static ConVar bla_timer("bla_timer", "1",
						FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_DEMO, 
						"Turn the timer display on/off");

class CHudTimer : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudTimer, Panel);

public:
	CHudTimer(const char *pElementName);
	virtual void Init();
	virtual void VidInit()
	{
		Reset();
	}
	virtual void Reset();
	virtual bool ShouldDraw()
	{
		return bla_timer.GetBool() && CHudElement::ShouldDraw();
	}
	void MsgFunc_BlaTimer_TimeToBeat(bf_read &msg);
	void MsgFunc_BlaTimer_Time(bf_read &msg);
	void MsgFunc_BlaTimer_StateChange(bf_read &msg);
	void MsgFunc_BlaTimer_AddGhost(bf_read &msg);
	void MsgFunc_BlaTimer_RemoveGhost(bf_read &msg);
	void MsgFunc_BlaTimer_UpdateGhost(bf_read &msg);
	virtual void Paint();

private:
	int initialTall;
	float m_flSecondsRecord;
	float m_flSecondsTime;
	wchar_t m_pwCurrentTime[BUFSIZE];
	char m_pszString[BUFSIZE];

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
	CPanelAnimationVarAliasType(float, digit2_xpos, "digit2_xpos", "98", 
		"proportional_float");
	CPanelAnimationVarAliasType(float, digit2_ypos, "digit2_ypos", "16", 
		"proportional_float");
};

DECLARE_HUDELEMENT(CHudTimer);
DECLARE_HUD_MESSAGE(CHudTimer, BlaTimer_TimeToBeat);
DECLARE_HUD_MESSAGE(CHudTimer, BlaTimer_Time);
DECLARE_HUD_MESSAGE(CHudTimer, BlaTimer_StateChange);
DECLARE_HUD_MESSAGE(CHudTimer, BlaTimer_AddGhost);
DECLARE_HUD_MESSAGE(CHudTimer, BlaTimer_RemoveGhost);
DECLARE_HUD_MESSAGE(CHudTimer, BlaTimer_UpdateGhost);

CHudTimer::CHudTimer(const char *pElementName) :
	CHudElement(pElementName), Panel(NULL, "HudTimer")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

void CHudTimer::Init()
{
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_TimeToBeat);
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_Time);
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_StateChange);
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_AddGhost);
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_RemoveGhost);
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_UpdateGhost);
	initialTall = 48;
	Reset();
}

void CHudTimer::Reset()
{
	m_flSecondsTime = 0.0f;
}

void CHudTimer::MsgFunc_BlaTimer_RemoveGhost(bf_read &msg)  {
	char string[256];
	msg.ReadString(string, sizeof(string), true);
	size_t ptr = atoi(string);
	int size = ghosts.Size();
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

void CHudTimer::MsgFunc_BlaTimer_AddGhost(bf_read &msg) {
	struct GhostData gd;
	char string[255];
	msg.ReadString(string, sizeof(string), true);
	CUtlVector<const char*> vec;
	splitByDelimiter(string, ":", vec);
	size_t ptr = atoi(vec[0]);
	gd.runPtr = ptr;
	Q_strcpy(gd.name, vec[1]);
	Q_strcpy(gd.map, vec[2]);
	gd.step = 0;
	ghosts.AddToTail(gd);
}

void CHudTimer::MsgFunc_BlaTimer_UpdateGhost(bf_read &msg) {
	//TODO update map and step (maybe calculate time)
	char string[255];
	msg.ReadString(string, sizeof(string), true);
	CUtlVector<const char*> vec;
	splitByDelimiter(string, ":", vec);
	size_t ptr = atoi(vec[0]);
	int size = ghosts.Size();
	for (int i = 0; i < size; i++ ) {
		if (ghosts[i].runPtr == ptr) {
			Q_strcpy(ghosts[i].map, vec[2]);
			ghosts[i].step = atoi(vec[1]);
			break;
		}
	}
}

void CHudTimer::MsgFunc_BlaTimer_TimeToBeat(bf_read &msg)
{
	m_flSecondsRecord = msg.ReadFloat();
	DevMsg("CHudTimer: map record is %02d:%02d.%04d\n",
		(int)(m_flSecondsRecord / 60), ((int)m_flSecondsRecord) % 60,
		(int)((m_flSecondsRecord - (int)m_flSecondsRecord) * 10000));
}

void CHudTimer::MsgFunc_BlaTimer_Time(bf_read &msg)
{
	m_flSecondsTime = msg.ReadFloat();
}

void CHudTimer::MsgFunc_BlaTimer_StateChange(bf_read &msg)
{
	bool started = msg.ReadOneBit();
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
	DevMsg("TODO: run fancy effects for state `%s'\n", 
		started ? "started" : "stopped");
	if (started)
	{
		//VGUI_ANIMATE("TimerStart");
		pPlayer->EmitSound("blamod.StartTimer");
	}
	else // stopped
	{
		// Compare times.
		//VGUI_ANIMATE("TimerStop");
		pPlayer->EmitSound("blamod.StopTimer");
	}
}

void CHudTimer::Paint(void)
{
	// Convert the current time to a string.
	// Q_snprintf(m_pszString, sizeof(m_pszString), "%02d:%02d.%04d",
	//            (int)(m_flSecondsTime / 60), ((int)m_flSecondsTime) % 60,
	//           (int)((m_flSecondsTime - (int)m_flSecondsTime) * 10000));

	// msg.ReadString(m_pszString, sizeof(m_pszString));
	//g_pVGuiLocalize->ConvertANSIToUnicode(
	//        m_pszString, m_pwCurrentTime, sizeof(m_pwCurrentTime));

	// Draw the text label.
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());

	float yPos = text_ypos;
	//surface()->DrawPrintText(L"TIME", wcslen(L"TIME"));
	int size = ghosts.Size();
	if (size > 3) {
		SetTall(initialTall + (15 * (size - 3)));
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
	// Draw current time.
	//surface()->DrawSetTextFont(surface()->GetFontTall(m_hTextFont));
	//surface()->DrawSetTextPos(digit_xpos, digit_ypos);
	//surface()->DrawPrintText(m_pwCurrentTime, wcslen(m_pwCurrentTime));
}



