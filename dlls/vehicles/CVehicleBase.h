#pragma once

/*
========================
	CBaseVehicle
========================

	Partially ported VehicleAPI from ADM

	This code is uncomplete as it's partially ported from
	Advanced Development Mod :: VehicleAPI. 
	For example, VehicleEngine and VehicleWheel structs are absent.
	More of it is managed inside the entity class itself, as this
	mod only requires a boat and a motorbike. 

	For a more complete copy of the code, contact
	Admer456#2798 on Discord.

*/

//#include "vehicles/CVehicleBase_Types.h"

class CBaseVehicle;

// VehicleEngine running flags
constexpr unsigned int fEngine_Running		= 1 << 0;
constexpr unsigned int fEngine_ClutchHeld	= 1 << 1;
constexpr unsigned int fEngine_Destroyed	= 1 << 2;
constexpr unsigned int fEngine_NoFuel		= 1 << 3;
constexpr unsigned int fEngine_GasHeld		= 1 << 4;
constexpr unsigned int fEngine_BrakeHeld	= 1 << 5;
constexpr unsigned int fEngine_HBHeld		= 1 << 6;

// VehicleEngine driving states
constexpr int Driving_Reverse	= -1;
constexpr int Driving_Off		= 0;
constexpr int Driving_Lowrpm	= 1;
constexpr int Driving_Hirpm		= 2;

// Seat control bits
constexpr int fVehKey_Forward	= 1 << 0;
constexpr int fVehKey_Backward	= 1 << 1;
constexpr int fVehKey_Left		= 1 << 2;
constexpr int fVehKey_Right		= 1 << 3;
constexpr int fVehKey_Use		= 1 << 4;
constexpr int fVehKey_Fire		= 1 << 5;
constexpr int fVehKey_Combokey	= 1 << 6; // currently assigned to the reload key
constexpr int fVehKey_Handbrake = 1 << 7;

struct VehicleSeat
{
	VehicleSeatType type;
	vec3_t pos, angles;
	CBasePlayer *pSessilis; // Never thought you'd see Latin in the HL SDK, did ya?
	bool fExists = false;
	int iSitdex; // Cool name for a seat index ;)
	int iCommands; // fVehKey command flags

	int SeatPlayer();
	void SeatSwitch( VehicleSeat &nextSeat );
	void FlushSeatCommands();
	void ListenToCommands();
	void CheckCommand( int outToSet, int inToCheck );
	void Exit();
	void Init( VehicleSeatType intype, vec3_t inpos, int sitdex );
	void AttachToPos( CBaseVehicle &Vehicle, int iBoneOffset );
};

class CBaseVehicle : public CBaseAnimating
{
public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
	int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE; }

	virtual void VehicleInit( void );
	virtual void VehicleBaseInit( void );

	// VehicleEnterUse allows for trigger like trigger_once to manually place the player into the vehicle
	void EXPORT VehicleEnterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void VehicleExit( void );

	void EXPORT VehicleThink( void );
	virtual void VehicleMove( void );

	VehicleType GetVehType( void ) { return v_Type; }
	int GetNumWheels( void ) { return m_iWheels; }

//private:
	float m_flMaxSpeed; // maximum vehicle speed in units/s
	int m_iSeatBoneOffset; // the ID of the bone to attach the player to

	int m_iWheels;
	VehicleType v_Type;
	VehicleSeat v_Seat;
};

void GetBoneId( int &iBone, VehicleType vType );
