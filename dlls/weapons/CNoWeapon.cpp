/*
	CNoWeapon implementation

	It's a weapon which is used on ladders.
	The player switches to NoWeapon when climbing a ladder,
	and then switches back to the original weapon when off the ladder.
*/

#include "extdll.h" 
#include "util.h" 
#include "cbase.h" 
#include "monsters.h" 
#include "weapons.h" 
#include "nodes.h" 
#include "player.h" 
#include "gamerules.h"

/*
class CNoWeapon : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);

	int iItemSlot(void) { return 1; } //inline function!
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer(CBasePlayer *pPlayer);

	void PrimaryAttack(void) { return; }
	void SecondaryAttack(void) { return; }

	BOOL Deploy(void);
	void Holster(void);
	void WeaponIdle(void);
};
*/

LINK_ENTITY_TO_CLASS(weapon_nothing, CNoWeapon);

void CNoWeapon::Spawn()
{
	Precache();
	m_iId = WEAPON_EMPTYHANDS;
	SET_MODEL(ENT(pev), "models/w_crowbar.mdl");
	m_iClip = -1;

	FallInit(); // droppin' like a bomb
}

void CNoWeapon::Precache()
{
//	PRECACHE_MODEL("models/v_null.mdl");
}

int CNoWeapon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = WEAPON_EMPTYHANDS;
	p->iWeight = CROWBAR_WEIGHT;

	return 1;
}

int CNoWeapon::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
			WRITE_BYTE(m_iId);
			MESSAGE_END();
		return TRUE;
	}

	return FALSE;
}

BOOL CNoWeapon::Deploy(void)
{
	return DefaultDeploy("models/v_null.mdl", "models/v_null.mdl", 1, "crowbar");
}

void CNoWeapon::Holster(int skiplocal)
{
	// nothing
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CNoWeapon::WeaponIdle(void)
{
	//m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
	//m_flNextSecondaryAttack = GetNextAttackDelay(0.5);
	m_flTimeWeaponIdle = GetNextAttackDelay(3.0);
}
