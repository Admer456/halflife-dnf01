#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "parsemsg.h"

#include <string>

DECLARE_MESSAGE( m_PlayerMonologue, PlayerTalk );

int CHudPlayerMonologue::Init()
{
	HOOK_MESSAGE( PlayerTalk );
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );

	return 1;
}

int CHudPlayerMonologue::VidInit( void )
{
	return 1;
}

int CHudPlayerMonologue::MsgFunc_PlayerTalk( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	PlaySound( READ_STRING(), 1 );
	
	return 1;
}

