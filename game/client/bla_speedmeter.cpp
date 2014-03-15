#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "vphysics_interface.h"
#include "c_prop_vehicle.h"

using namespace vgui;

//Credit CZF!

static ConVar gh_speedmeter("gh_speedmeter", "1", 
							FCVAR_CLIENTDLL | FCVAR_ARCHIVE, 
							"Turn the speedmeter on/off");

class CHudSpeedMeter : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE(CHudSpeedMeter, CHudNumericDisplay);

public:
	CHudSpeedMeter(const char *pElementName);
	virtual void Init()
	{
		Reset();
	}
	virtual void VidInit()
	{
		Reset();
	}
	virtual void Reset()
	{
		SetLabelText(L"UPS");
		SetDisplayValue(0);
	}
	virtual bool ShouldDraw()
	{
		return gh_speedmeter.GetBool() && CHudElement::ShouldDraw();
	}
	virtual void OnThink();
};

DECLARE_HUDELEMENT(CHudSpeedMeter);

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName) :
	CHudElement(pElementName), CHudNumericDisplay(NULL, "HudSpeedMeter")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

void CHudSpeedMeter::OnThink()
{
	Vector velocity(0, 0, 0);
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player) {
		if (player->IsInAVehicle()) {
			
			C_PropVehicleDriveable* veh = (C_PropVehicleDriveable*)player->GetVehicle();
			if (veh) {
				velocity = veh->GetSpeed();
			}
		} else {
			velocity = player->GetLocalVelocity();
		}
		SetDisplayValue((int)velocity.Length());
	}
}