#pragma once

enum kick_e
{
	LEG_IDLE,
	LEG_KICK
};

class CKickLeg
{
public:
	cl_entity_t entLeg;

	int InitLeg(void);
	void Kick(void);
	int UpdateLeg(void);

	void Shake(float value);

	float m_flToIdle = 0;

//	int MsgFunc_KickLeg(const char *pszName, int iSize, void *pbuf);
};