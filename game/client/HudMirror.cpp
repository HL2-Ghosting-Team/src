#include "cbase.h"
#include "HudMirror.h"
 
using namespace vgui;
 
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
 
//.vmt location + filename
#define IMAGE_MIRROR "mirror/mirror"
 

static ConVar drawMirror("gh_hud_mirror", "0", FCVAR_CLIENTDLL | FCVAR_REPLICATED | FCVAR_CHEAT, "(Cheat Protected) Draws a rear view mirror. 0 = off, 1 = on.");

extern CHud gHUD;
 
DECLARE_HUDELEMENT( CHudMirror );
 
bool CHudMirror::shouldDrawMirror() {
	return drawMirror.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudMirror::CHudMirror( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel(NULL, "HudMirror")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );	//Our parent is the screen itself.	
 
	m_Mirror = new ImagePanel(this, "RearViewMirror");
	m_Mirror->SetImage( IMAGE_MIRROR );
 
	SetPaintBackgroundEnabled(false);
}

bool CHudMirror::ShouldDraw() {
	return CHudElement::ShouldDraw() && shouldDrawMirror();
}
 
//-----------------------------------------------------------------------------
// Purpose: Paint
//-----------------------------------------------------------------------------
void CHudMirror::Paint() 
{
        //Set position Top Right corner
	SetPos( ScreenWidth() - 270 , 25 );
 
        //Set Mirror to 256x128 pixels
	m_Mirror->SetSize( 256, 128 );
	m_Mirror->SetVisible( true );
}