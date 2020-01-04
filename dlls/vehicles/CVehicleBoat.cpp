#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "player.h"
#include "CVehicleBase.h"

class CVehicleBoat : public CBaseVehicle
{
public:
	void VehicleInit( void );
	void VehicleMove( void );
};

LINK_ENTITY_TO_CLASS( vehicle_boat, CVehicleBoat );

void CVehicleBoat::VehicleInit( void )
{
	v_Seat.Init( DriverGunner, pev->origin, 0 );
	v_Type = VehicleBoat;
	m_iWheels = 0;
}

void CVehicleBoat::VehicleMove( void )
{
	CBaseVehicle::VehicleMove();
	
	static float cyclePitch = 0.0f;
	static float cycleRoll = 0.0f;
	static float cyclePitchIterator = 0.0f;
	static float cycleRollIterator = 0.0f;
	static float speedPitch = 0.0f;
	static float steerYawTarget = 0.0f;
	static int   steerYawDir = 0; // 1 left, 0 middle, -1 right

	static float floatingCycle = 0.0f;
	static float floatingCycleIterator = 0.0434f;

	float speed = pev->velocity.Length2D();

	steerYawDir = 0;

	if ( v_Seat.iCommands & fVehKey_Left )
		steerYawDir = 1;
	else if ( v_Seat.iCommands & fVehKey_Right )
		steerYawDir = -1;

	if ( !cyclePitchIterator ) // initialise static iterators and set initial angles
	{
		cyclePitchIterator = 0.031f;
		cycleRollIterator = 0.0434f;

		pev->angles.x = -4;
		pev->angles.z = -4;
	}

	cyclePitch += cyclePitchIterator;
	cycleRoll += cycleRollIterator;
	floatingCycle += floatingCycleIterator;

	// Cycle limiting
	if ( cyclePitch > 1.0f || cyclePitch < -1.0f )
		cyclePitchIterator *= -1;

	if ( cycleRoll > 1.0f || cycleRoll < -1.0f )
		cycleRollIterator *= -1;

	if ( floatingCycle > 1.0f || floatingCycle < -1.0f )
		floatingCycleIterator *= -1;

	// Make it go up'n'down
	if ( pev->waterlevel )
	{
		pev->velocity.z += floatingCycle * 6 + 2;
	}

	// Rock the boat
	pev->angles.x += cyclePitch * 0.25f / (speed*0.01 + 1);
	pev->angles.z += cycleRoll  * 0.25f / (speed*0.01 + 1);

	// Increase the pitch as we go faster
	if ( v_Seat.iCommands & fVehKey_Forward )
	{
		speedPitch = speedPitch * 0.8 + 15 * 0.2;
		pev->angles.x = pev->angles.x * 0.5 + speedPitch * 0.5;
	}
	else
	{
		speedPitch /= 1.01;
		pev->angles.x /= 1.015;
	}

	// Steer the boat
	steerYawTarget = steerYawTarget * 0.7 + (float)steerYawDir * 0.3;

	if ( speed < 200 )
		pev->angles.y += steerYawTarget * (speed * 0.005);
	else if ( speed > 100 )
		pev->angles.y += steerYawTarget * 1;

	pev->angles.x /= 1.01;
	
	// TO-DO: update the bone controller for the steering wheel
}

