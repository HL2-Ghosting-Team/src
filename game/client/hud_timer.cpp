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

#define BUFSIZE (sizeof("00:00:00.0000")+1)

static ConVar bla_timer("gh_timer", "1",
						FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_DEMO, 
						"Turn the timer display on/off");

static ConVar timer_mode("gh_timer_mode", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED,
						 "Set what type of timer you want.\n0 = Generic Timer (no splits)\n1 = Splits by Chapter\n2 = Splits by Level");

class CHudTimer : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudTimer, Panel);

public:
	CHudTimer(const char *pElementName);
	virtual void Init();
	virtual void VidInit()
	{
		
		/*const char* charray[68] = {
			"d1_trainstation_01",
			"d1_trainstation_02",
			"d1_trainstation_03",
			"d1_trainstation_04",
			"d1_trainstation_05",
			"d1_trainstation_06",
			"d1_canals_01",
			"d1_canals_01a",
			"d1_canals_02",
			"d1_canals_03",
			"d1_canals_05",
			"d1_canals_06",
			"d1_canals_07",
			"d1_canals_08",
			"d1_canals_09",
			"d1_canals_10",
			"d1_canals_11",
			"d1_canals_12",
			"d1_canals_13",
			"d1_eli_01",
			"d1_eli_02",
			"d1_town_01",
			"d1_town_01a",
			"d1_town_02",
			"d1_town_02a",
			"d1_town_03",
			"d1_town_04",
			"d1_town_05",
			"d2_coast_01",
			"d2_coast_03",
			"d2_coast_04",
			"d2_coast_05",
			"d2_coast_07",
			"d2_coast_08",
			"d2_coast_09",
			"d2_coast_10",
			"d2_coast_11",
			"d2_coast_12",
			"d2_prison_01",
			"d2_prison_02",
			"d2_prison_03",
			"d2_prison_04",
			"d2_prison_05",
			"d2_prison_06",
			"d2_prison_07",
			"d2_prison_08",
			"d3_c17_01",
			"d3_c17_02",
			"d3_c17_03",
			"d3_c17_04",
			"d3_c17_05",
			"d3_c17_06a",
			"d3_c17_06b",
			"d3_c17_07",
			"d3_c17_08",
			"d3_c17_09",
			"d3_c17_10a",
			"d3_c17_10b",
			"d3_c17_11",
			"d3_c17_12",
			"d3_c17_12b",
			"d3_c17_13",
			"d3_citadel_01",
			"d3_citadel_02",
			"d3_citadel_03",
			"d3_citadel_04",
			"d3_citadel_05",
			"d3_breen_01"
		};*/
		//Reset();
	}
	virtual void Reset();
	virtual bool ShouldDraw()
	{
		return bla_timer.GetBool() && CHudElement::ShouldDraw();
	}
	void MsgFunc_BlaTimer_TimeToBeat(bf_read &msg);
	void MsgFunc_BlaTimer_Time(bf_read &msg);
	void MsgFunc_BlaTimer_StateChange(bf_read &msg);

	//int getPos(const char*);

	virtual void Paint();

private:
	int initialTall;
	float m_flSecondsRecord;
	float m_flSecondsTime;
	wchar_t m_pwCurrentTime[BUFSIZE];
	char m_pszString[BUFSIZE];
	CUtlMap<const char*, float> map;

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

CHudTimer::CHudTimer(const char *pElementName) :
	CHudElement(pElementName), Panel(NULL, "HudTimer")
{
	SetParent(g_pClientMode->GetViewport());
}

void CHudTimer::Init()
{
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_TimeToBeat);
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_Time);
	HOOK_HUD_MESSAGE(CHudTimer, BlaTimer_StateChange);
	initialTall = 48;
	//Reset();
}

void CHudTimer::Reset()
{
	m_flSecondsTime = 0.0f;
}

/*int CHudTimer::getPos(const char* map) {
	switch (timer_mode.GetInt())
	{
	case 0://generic timer



		break;
	case 1://splits by chapter



		break;
	case 2://splits by level


		break;

	default:
		return 0;
	}
}*/

void CHudTimer::MsgFunc_BlaTimer_TimeToBeat(bf_read &msg)
{
	m_flSecondsRecord = msg.ReadFloat();
	DevMsg("CHudTimer: map record is %03d:%02d.%04d\n",
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
	int hours = (int)(m_flSecondsTime / 3600.0f);
	int minutes = (int)(((m_flSecondsTime / 3600.0f) - hours) * 60.0f);
	int seconds = (int)(((((m_flSecondsTime / 3600.0f) - hours) * 60.0f) - minutes) * 60.0f);
	int millis = (int)(((((((m_flSecondsTime / 3600.0f) - hours) * 60.0f) - minutes) * 60.0f) - seconds) * 10000.0f);

	Q_snprintf(m_pszString, sizeof(m_pszString), "%02d:%02d:%02d.%04d",
				hours,//hours
				minutes, //minutes
				seconds,//seconds
				millis);//millis

	// msg.ReadString(m_pszString, sizeof(m_pszString));
	g_pVGuiLocalize->ConvertANSIToUnicode(
		m_pszString, m_pwCurrentTime, sizeof(m_pwCurrentTime));

	// Draw the text label.
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());
	//current map can be found with:    g_pGameRules->MapName()

	//surface()->DrawPrintText(L"TIME", wcslen(L"TIME"));
	// Draw current time.
	surface()->DrawSetTextFont(surface()->GetFontTall(m_hTextFont));
	surface()->DrawSetTextPos(digit_xpos, digit_ypos);
	surface()->DrawPrintText(m_pwCurrentTime, wcslen(m_pwCurrentTime));
}



