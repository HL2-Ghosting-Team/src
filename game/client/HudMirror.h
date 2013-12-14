#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <vgui_controls/ImagePanel.h>

//-----------------------------------------------------------------------------
// Purpose: HUD Mirror panel
//-----------------------------------------------------------------------------
class CHudMirror : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudMirror, vgui::Panel );
 
public:
	CHudMirror( const char *pElementName );

	static bool shouldDrawMirror();
 
protected: 
	virtual void Paint();
	virtual bool ShouldDraw();
 
private:
	vgui::ImagePanel *m_Mirror;
};	