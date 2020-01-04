#pragma once

enum WeaponDamageType
{
	WDT_Eagle,
	WDT_9mm,
	WDT_Buckshot,
	WDT_357,
	WDT_MP5,
	WDT_762,
	WDT_556,

	WDT_mon9mm,
	WDT_monMP5,
	WDT_mon12mm
};

extern void EjectBrass(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype);

#ifndef CLIENT_DLL

class CProjectileBullet : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void) override;
	int Classify();
	void EXPORT BulletTouch(CBaseEntity *pOther);
	void EXPORT BulletThink();
	void EXPORT BulletRemove();

	static CProjectileBullet* LaunchProjectile(WeaponDamageType wdt, CBaseEntity *pOwner);

	WeaponDamageType m_WDT;
	CBaseEntity *m_pOwner;

	int m_iWoodGib;
	bool m_fSpark;
};

#endif // !CLIENT_DLL
