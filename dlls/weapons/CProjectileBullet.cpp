#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"

#include "CProjectileBullet.h"

#ifndef CLIENT_DLL

#include "func_break.h"

LINK_ENTITY_TO_CLASS(projectile_bullet, CProjectileBullet);

void CProjectileBullet::Precache(void)
{
	PRECACHE_MODEL("models/projectile_bullet.mdl");
	m_iWoodGib = PRECACHE_MODEL("models/woodgibs.mdl");
	//	PRECACHE_MODEL("models/crossbow_bolt.mdl");
}

void CProjectileBullet::Spawn(void)
{
	Precache();
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/shell.mdl");
//	SET_MODEL(ENT(pev), "models/crossbow_bolt.mdl");

	pev->gravity = 0.5;
	pev->angles.x *= -1;

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(-0.1, -0.1, -0.1), Vector(0.1, 0.1, 0.1));

	SetTouch(&CProjectileBullet::BulletTouch);
	SetThink(&CProjectileBullet::BulletThink);

	pev->nextthink = gpGlobals->time + 0.001;
}

int CProjectileBullet::Classify()
{
	return CLASS_NONE;
}

#define CASE(a, b) case a: flDamage = b; break;

void CProjectileBullet::BulletTouch(CBaseEntity *pOther)
{
	if (FClassnameIs(pOther->pev, "bullet"))
		return;

	bool fRemove = true;
	float flDamage;

	switch (m_WDT)
	{
		CASE(WDT_Eagle, gSkillData.plrDmgEagle);
		CASE(WDT_9mm, gSkillData.plrDmg9MM);
		CASE(WDT_Buckshot, gSkillData.plrDmgBuckshot);
		CASE(WDT_357, gSkillData.plrDmg357);
		CASE(WDT_MP5, gSkillData.plrDmgMP5);
		CASE(WDT_762, gSkillData.plrDmg762);
		CASE(WDT_556, gSkillData.plrDmg556);

		CASE(WDT_mon9mm, gSkillData.monDmg9MM);
		CASE(WDT_monMP5, gSkillData.monDmgMP5);
		CASE(WDT_mon12mm, gSkillData.monDmg12MM);
	}

	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		entvars_t *pevOwner = VARS(pev->owner);

		ClearMultiDamage();

		if (pOther->IsPlayer())
		{
			pOther->TraceAttack(pevOwner, flDamage, pev->velocity.Normalize(), &tr, DMG_NEVERGIB);
		}
		else
		{
			pOther->TraceAttack(pevOwner, flDamage, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
		}

		ApplyMultiDamage(pev, pevOwner);
	}
	else
	{
		switch (RANDOM_LONG(1, 5))
		{
		case 1:
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/ric1.wav", 0.8, ATTN_NORM, 0, 90 + RANDOM_FLOAT(-10, 20));
			break;
		case 2:
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/ric2.wav", 0.8, ATTN_NORM, 0, 90 + RANDOM_FLOAT(-10, 20));
			break;
		case 3:
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/ric3.wav", 0.8, ATTN_NORM, 0, 90 + RANDOM_FLOAT(-10, 20));
			break;
		case 4:
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/ric4.wav", 0.8, ATTN_NORM, 0, 90 + RANDOM_FLOAT(-10, 20));
			break;
		case 5:
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/ric5.wav", 0.8, ATTN_NORM, 0, 90 + RANDOM_FLOAT(-10, 20));
			break;
		}

		if (FClassnameIs(pOther->pev, "worldspawn") || 
			FClassnameIs(pOther->pev, "func_wall")  ||
			FClassnameIs(pOther->pev, "func_wall_toggle") ||
			FClassnameIs(pOther->pev, "func_door") ||
			FClassnameIs(pOther->pev, "func_door_rotating") ||
			FClassnameIs(pOther->pev, "func_train") ||
			FClassnameIs(pOther->pev, "func_tracktrain"))
		{
			m_fSpark = true;
		}
	}

	Vector vecBulletVel;
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_forward * 4, ignore_monsters, edict(), &tr);
	DecalGunshot(&tr, BULLET_PLAYER_MP5);

	if (FClassnameIs(pOther->pev, "func_breakable"))
	{
		UTIL_MakeVectors(pev->angles);
		CBreakable *pBreak = (CBreakable*)pOther;

		switch (pBreak->m_Material)
		{
		case matGlass: break;
		
		case matWood: 

			vecBulletVel = tr.vecPlaneNormal;
			vecBulletVel.x += gpGlobals->v_forward.x;
			vecBulletVel.y += gpGlobals->v_forward.y;
			vecBulletVel.z += gpGlobals->v_forward.z;

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_BREAKMODEL);
				WRITE_COORD_VECTOR(pev->origin);
			//	WRITE_COORD_VECTOR(Vector(0.00001, 0.00001, 0.00001));
				WRITE_COORD_VECTOR(Vector(1, 1, 1));
				WRITE_COORD_VECTOR(64 * vecBulletVel);
				WRITE_BYTE(1);
				WRITE_SHORT(m_iWoodGib);
				WRITE_BYTE(1);
				WRITE_BYTE(1);
				WRITE_BYTE(BREAK_WOOD);
			MESSAGE_END();

			break;
		
		case matMetal: 

//			UTIL_Sparks(pev->origin);
			m_fSpark = true;

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_ARMOR_RICOCHET);
				WRITE_COORD_VECTOR(pev->origin);
				WRITE_BYTE(6);
			MESSAGE_END();

			break;

//		case matFlesh: 
//			break;
		
//		case matCinderBlock: 
//			break;
		
//		case matCeilingTile: 
//			break;

		case matComputer: 

			m_fSpark = true;
			
			break;
//		case matUnbreakableGlass: 
//			break;
		
//		case matNone: 
//			break;

		default: break;
		}

		if (pOther->pev->health < 250 && FBitSet(pOther->pev->spawnflags, SF_BREAK_PINCH))
		{
			pOther->pev->angles.x += RANDOM_FLOAT(-0.5, 0.5);
			pOther->pev->angles.y += RANDOM_FLOAT(-0.5, 0.5);
			pOther->pev->angles.z += RANDOM_FLOAT(-0.5, 0.5);
		}

		pev->nextthink = gpGlobals->time + 0.001;
		pev->origin = pev->origin + gpGlobals->v_forward * 8;
		UTIL_SetOrigin(pev, pev->origin);

		if (!FBitSet(pOther->pev->spawnflags, SF_BREAK_BULCLIP))
			fRemove = false;
	}

	if (fRemove)
	{
		SetTouch(NULL);
		SetThink(NULL);

		SetThink(&CProjectileBullet::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.001;
	}
}

#undef CASE

void CProjectileBullet::BulletThink()
{
	pev->angles.x *= -1;
	UTIL_MakeVectors(pev->angles);
	pev->angles.x *= -1;

	pev->velocity = gpGlobals->v_forward * 4000;

	if (pev->waterlevel)
	{
		UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
	}

	pev->nextthink = gpGlobals->time + 1.0;
}

void CProjectileBullet::BulletRemove()
{
	if (m_fSpark)
		UTIL_Sparks(pev->origin);

	SetThink(&CProjectileBullet::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.001;
}

CProjectileBullet* CProjectileBullet::LaunchProjectile(WeaponDamageType wdt, CBaseEntity *pOwner)
{
	CProjectileBullet *pBullet = GetClassPtr((CProjectileBullet*)NULL);
	pBullet->pev->classname = MAKE_STRING("bullet");
	pBullet->m_pOwner = pOwner;
	pBullet->m_WDT = wdt;
	pBullet->Spawn();

	return pBullet;
}

#endif // !CLIENT_DLL
