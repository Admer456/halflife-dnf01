#include <assert.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"
#include "SDL2/SDL_opengl.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "Exports.h"

#include "dnf_kick.h"
#include "parsemsg.h"

extern Vector g_vecViewOrigin, g_vecViewAngles;
extern engine_studio_api_t IEngineStudio;
extern CGameStudioModelRenderer g_StudioRenderer;

float g_LegViewPunch;

// To save some typing uwu
#define IES IEngineStudio
// Couldn't bother typing the whole name
#define gSR g_StudioRenderer

int CKickLeg::InitLeg(void)
{
	entLeg.model = IES.Mod_ForName("models/v_kick.mdl", 0); // this model is absolutely needed
	
	return 1;
}

void CKickLeg::Kick(void)
{
	entLeg.curstate.animtime = gEngfuncs.GetClientTime();
	entLeg.curstate.sequence = LEG_KICK;

	m_flToIdle = gEngfuncs.GetClientTime() + 0.55;
}

// Original method from PARANOIA by buzer
// They implemented this method directly into the StudioModelRenderer
// instead of this separate class.
// TL;DR this way allows for a cleaner integration with the HUD.
// In hud.h I defined a CHudKick class which contains a CKickLeg,
// so it can be easily called and whatnot - instead of using global vars D:

int CKickLeg::UpdateLeg(void)
{
	alight_t m_Lighting;
	vec3_t m_vecDir;

	// update position
	VectorCopy(g_vecViewOrigin, entLeg.origin);
	VectorCopy(g_vecViewOrigin, entLeg.curstate.origin);
	VectorCopy(g_vecViewAngles, entLeg.angles);
	VectorCopy(g_vecViewAngles, entLeg.curstate.angles);

/*	gEngfuncs.Con_Printf("\norg: %i\t%i\t%i",
		(int)entLeg.curstate.origin.x,
		(int)entLeg.curstate.origin.y,
		(int)entLeg.curstate.origin.z
	); */

	gSR.m_pCurrentEntity = &entLeg;

	// get some data from the engine studio (IES), and feed it to the game studio renderer (GSR)
	IES.GetTimes(
		&gSR.m_nFrameCount,
		&gSR.m_clTime,
		&gSR.m_clOldTime);

	IES.GetViewInfo(
		gSR.m_vRenderOrigin,
		gSR.m_vUp,
		gSR.m_vRight,
		gSR.m_vNormal);

	IES.GetAliasScale(&gSR.m_fSoftwareXScale,
		&gSR.m_fSoftwareYScale);

	// set the framerate to 1x of the original
	// if your sequence is 30fps, setting this to 2 will play back at 60fps
	gSR.m_pCurrentEntity->curstate.framerate = 1;

	// set the render model and load the .mdl header
	gSR.m_pRenderModel = gSR.m_pCurrentEntity->model;
	gSR.m_pStudioHeader = (studiohdr_t *)IES.Mod_Extradata(gSR.m_pRenderModel);
	IES.StudioSetHeader(gSR.m_pStudioHeader);
	IES.SetRenderModel(gSR.m_pRenderModel);

	// set up the transformation matrices
	g_StudioRenderer.StudioSetUpTransform(0);

	// increase drawn models by 1
	(*gSR.m_pModelsDrawn)++;
	(*gSR.m_pStudioModelCount)++;

	// if we don't have bodygroups, return
	if (!gSR.m_pStudioHeader->numbodyparts)
		return 1;

	// set our bones up and save them for later use
	gSR.StudioSetupBones();
	gSR.StudioSaveBones();

	// set up lighting
	m_Lighting.plightvec = m_vecDir;
	IES.StudioDynamicLight(gSR.m_pCurrentEntity, &m_Lighting);
	IES.StudioEntityLight(&m_Lighting);

	IES.StudioSetupLighting(&m_Lighting);

	// grab top and bottom colour, remap them in case we ever do MP
	gSR.m_nTopColor = gSR.m_pCurrentEntity->curstate.colormap & 0xFF;
	gSR.m_nBottomColor = (gSR.m_pCurrentEntity->curstate.colormap & 0xFF00) >> 8;
	IES.StudioSetRemapColors(gSR.m_nTopColor, gSR.m_nBottomColor);

	// render the model
	gSR.StudioRenderModel();

	return 1;
}

void CKickLeg::Shake(float value)
{
	g_LegViewPunch += value;
}

/*
=========================
buzer's additional model renderer experiment
=========================

#define SHIELD_ON	0
#define SHIELD_OFF	1
#define SHIELD_TURNING_ON	2
#define SHIELD_TURNING_OFF	3

#define SHIELDANIM_IDLE 0
#define SHIELDANIM_DRAW 1
#define SHIELDANIM_HOLSTER 2

cl_entity_t g_HeadShieldEnt;
extern vec3_t v_origin, v_angles;
int g_HeadShieldState;
float g_HeadShieldSwitchTime;*/
/*
int CStudioModelRenderer::StudioDrawHeadShield(void)
{
	alight_t lighting;
	vec3_t dir;
	//	int savedDoInterp;

	g_HeadShieldEnt.model = IEngineStudio.Mod_ForName("models/v_headshield.mdl", 1);
	VectorCopy(v_origin, g_HeadShieldEnt.origin);
	VectorCopy(v_origin, g_HeadShieldEnt.curstate.origin);
	VectorCopy(v_angles, g_HeadShieldEnt.angles);
	VectorCopy(v_angles, g_HeadShieldEnt.curstate.angles);
	g_HeadShieldEnt.angles[PITCH] *= -1;
	g_HeadShieldEnt.curstate.angles[PITCH] *= -1;

	m_pCurrentEntity = &g_HeadShieldEnt;
	IEngineStudio.GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	IEngineStudio.GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	IEngineStudio.GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	m_pRenderModel = m_pCurrentEntity->model;
	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_pRenderModel);
	IEngineStudio.StudioSetHeader(m_pStudioHeader);
	IEngineStudio.SetRenderModel(m_pRenderModel);
	if (cv_renderer->value && cv_nsr->value) GL_SetupTextureHeader(); // buz

	StudioSetUpTransform(0);
*/
/*
	(*m_pModelsDrawn)++;
	(*m_pStudioModelCount)++; // render data cache cookie

	if (m_pStudioHeader->numbodyparts == 0)
		return 1;

	StudioSetupBones();
	StudioSaveBones();

	lighting.plightvec = dir;
	IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting);
	IEngineStudio.StudioEntityLight(&lighting);

	// model and frame independant
	IEngineStudio.StudioSetupLighting(&lighting);

	// get remap colors
	m_nTopColor = m_pCurrentEntity->curstate.colormap & 0xFF;
	m_nBottomColor = (m_pCurrentEntity->curstate.colormap & 0xFF00) >> 8;

	IEngineStudio.StudioSetRemapColors(m_nTopColor, m_nBottomColor);

	StudioRenderModel();

	return 1;
}*/
// buz - head shield message
/*
int __MsgFunc_HeadShield(const char *pszName, int iSize, void *pbuf)
{
	studiohdr_t *pStudioHeader;
	mstudioseqdesc_t *pseq;

	BEGIN_READ(pbuf, iSize);

	// 0 is OFF; 1 is ON; 2 is fast switch to ON
	switch (READ_BYTE())
	{
	case 0:
		g_HeadShieldState = SHIELD_TURNING_OFF;
		g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
		g_HeadShieldEnt.curstate.sequence = SHIELDANIM_HOLSTER;

		// get animation length in seconds
		pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(IEngineStudio.Mod_ForName("models/v_headshield.mdl", 1));
		pseq = (mstudioseqdesc_t *)((byte *)pStudioHeader + pStudioHeader->seqindex) + SHIELDANIM_HOLSTER;
		g_HeadShieldSwitchTime = gEngfuncs.GetClientTime() + (pseq->numframes / pseq->fps);
		break;

	case 1:
		g_HeadShieldState = SHIELD_TURNING_ON;
		g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
		g_HeadShieldEnt.curstate.sequence = SHIELDANIM_DRAW;

		// get animation length in seconds
		pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(IEngineStudio.Mod_ForName("models/v_headshield.mdl", 1));
		pseq = (mstudioseqdesc_t *)((byte *)pStudioHeader + pStudioHeader->seqindex) + SHIELDANIM_DRAW;
		g_HeadShieldSwitchTime = gEngfuncs.GetClientTime() + (pseq->numframes / pseq->fps);
		break;

	case 2:
		g_HeadShieldState = SHIELD_ON;
		g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
		g_HeadShieldEnt.curstate.sequence = SHIELDANIM_IDLE;
	}
	return 1;
}*/
/*
void ResetHeadShield(void) // buz
{
	memset(&g_HeadShieldEnt, 0, sizeof(cl_entity_t));
	g_HeadShieldEnt.curstate.framerate = 1;
	g_HeadShieldState = SHIELD_OFF;
	g_HeadShieldSwitchTime = 0;
}

void InitHeadShield(void) // buz
{
	HOOK_MESSAGE(HeadShield);
}

int HeadShieldThink(void) // buz: return 1 to draw shield
{
	//	gEngfuncs.Con_Printf("shield state: %d\n", g_HeadShieldState); // test
	float time = gEngfuncs.GetClientTime();

	switch (g_HeadShieldState)
	{
	case SHIELD_OFF:
	default:
		return 0;

	case SHIELD_ON:
		return 1;

	case SHIELD_TURNING_ON:
		if (time > g_HeadShieldSwitchTime)
		{
			g_HeadShieldState = SHIELD_ON;
			g_HeadShieldEnt.curstate.animtime = time;
			g_HeadShieldEnt.curstate.sequence = SHIELDANIM_IDLE;
		}
		return 1;

	case SHIELD_TURNING_OFF:
		if (time > g_HeadShieldSwitchTime)
		{
			g_HeadShieldState = SHIELD_OFF;
			return 0;
		}
		else
			return 1;
	}
}
*/
/*
// buz
int CStudioModelRenderer::StudioDrawCustomEntity(cl_entity_t *ent, int amb, int shade)
{
	alight_t lighting;
	vec3_t dir;
	//	int savedDoInterp;
	//	savedDoInterp = m_fDoInterp;
	//	m_fDoInterp = 0;

	m_pCurrentEntity = ent;
	IEngineStudio.GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	IEngineStudio.GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	IEngineStudio.GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	m_pRenderModel = m_pCurrentEntity->model;
	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_pRenderModel);
	IEngineStudio.StudioSetHeader(m_pStudioHeader);
	IEngineStudio.SetRenderModel(m_pRenderModel);
	if (cv_renderer->value && cv_nsr->value) GL_SetupTextureHeader(); // buz

	StudioSetUpTransform(0);

	(*m_pModelsDrawn)++;
	(*m_pStudioModelCount)++; // render data cache cookie

	if (m_pStudioHeader->numbodyparts == 0)
		return 1;

	StudioSetupBones();
	StudioSaveBones();

	if (cv_renderer->value && cv_nsr->value)
	{
		GL_SetupLighting(); // buz
	}
	else
	{
		lighting.plightvec = dir;
		IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting);
		IEngineStudio.StudioEntityLight(&lighting);

		if (amb != -1) lighting.ambientlight = amb;
		if (shade != -1) lighting.shadelight = shade;
		//	CONPRINT("amb: %d, shade: %d\n", lighting.ambientlight, lighting.shadelight);

		// model and frame independant
		IEngineStudio.StudioSetupLighting(&lighting);

		// get remap colors
		m_nTopColor = m_pCurrentEntity->curstate.colormap & 0xFF;
		m_nBottomColor = (m_pCurrentEntity->curstate.colormap & 0xFF00) >> 8;

		IEngineStudio.StudioSetRemapColors(m_nTopColor, m_nBottomColor);
	}

	//	gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
	StudioRenderModel();
	//	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	//	m_fDoInterp = savedDoInterp;

	return 1;
}
*/

int __MsgFunc_LegDoKick(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.m_Kick.MsgFunc_Kick(pszName, iSize, pbuf);
}

int __MsgFunc_LegShake(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.m_Kick.MsgFunc_KickShake(pszName, iSize, pbuf);
}

int CHudKick::Init(void)
{
	HOOK_MESSAGE(LegDoKick);
	HOOK_MESSAGE(LegShake);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
}

int CHudKick::VidInit(void)
{
	return 1;
}

int CHudKick::Draw(float flTime)
{
	int iCurAnim = m_Leg.entLeg.curstate.sequence;

#ifdef DEBUG
	if (iCurAnim == LEG_IDLE)
		gHUD.DrawHudString(20, 60, 9, "LEG_IDLE", 255, 160, 0);
	else if (iCurAnim == LEG_KICK)
		gHUD.DrawHudString(20, 60, 9, "LEG_KICK", 50, 50, 255);
#endif

	return 1;
}

void CHudKick::Think(void)
{
	float flCurtime = gEngfuncs.GetClientTime();

	if (flCurtime > m_Leg.m_flToIdle &&
		m_Leg.entLeg.curstate.sequence != LEG_IDLE)
		m_Leg.entLeg.curstate.sequence = LEG_IDLE;
}

int CHudKick::MsgFunc_Kick(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_Leg.Kick();
	return 1;
}

int CHudKick::MsgFunc_KickShake(const char *pszName, int iSize, void *pbuf)
{
	float value;

	BEGIN_READ(pbuf, iSize);

	value = READ_COORD();

	m_Leg.Shake(value);
	return 1;
}
