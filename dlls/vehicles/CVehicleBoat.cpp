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
	
	static float cycle_pitch = 0.0f;
	static float cycle_roll = 0.0f;
	static float cycle_pitch_iterator = 0.0f;
	static float cycle_roll_iterator = 0.0f;
	static float speed_pitch = 0.0f;
	static float steer_yaw_target = 0.0f;
	static int steer_yaw_factor = 0; // 1 left, 0 middle, -1 right

	static float floating_cycle = 0.0f;
	static float floating_cycle_iterator = 0.0434f;

	float speed = pev->velocity.Length2D();

	steer_yaw_factor = 0;

	if ( v_Seat.iCommands & fVehKey_Left )
		steer_yaw_factor = 1;
	else if ( v_Seat.iCommands & fVehKey_Right )
		steer_yaw_factor = -1;

//	if ( (v_Seat.iCommands & fVehKey_Left) && (v_Seat.iCommands & fVehKey_Right) )
//		steer_yaw_factor = 0;

	if ( !cycle_pitch_iterator ) // initialise static iterators and set initial angles
	{
		cycle_pitch_iterator = 0.031f;
		cycle_roll_iterator = 0.0434f;

		pev->angles.x = -4;
		pev->angles.z = -4;
	}

	cycle_pitch += cycle_pitch_iterator;
	cycle_roll += cycle_roll_iterator;
	floating_cycle += floating_cycle_iterator;

	if ( cycle_pitch > 1.0f || cycle_pitch < -1.0f )
		cycle_pitch_iterator *= -1;

	if ( cycle_roll > 1.0f || cycle_roll < -1.0f )
		cycle_roll_iterator *= -1;

	if ( floating_cycle > 1.0f || floating_cycle < -1.0f )
		floating_cycle_iterator *= -1;

	if ( pev->waterlevel )
	{
		pev->velocity.z += floating_cycle * 6 + 2;
	}

	pev->angles.x += cycle_pitch * 0.25f / (speed*0.01 + 1);
	pev->angles.z += cycle_roll  * 0.25f / (speed*0.01 + 1);

	if ( v_Seat.iCommands & fVehKey_Forward )
	{
		speed_pitch = speed_pitch * 0.8 + 15 * 0.2;
		pev->angles.x = pev->angles.x * 0.5 + speed_pitch * 0.5;
	}
	else
	{
		speed_pitch /= 1.01;
		pev->angles.x /= 1.015;
	}

	steer_yaw_target = steer_yaw_target * 0.7 + (float)steer_yaw_factor * 0.3;

//	ALERT( at_console, "target %f \t factor %d \t float %f\n", steer_yaw_target, steer_yaw_factor, floating_cycle );

	if ( speed < 200 )
		pev->angles.y += steer_yaw_target * (speed * 0.005);
	else if ( speed > 100 )
		pev->angles.y += steer_yaw_target * 1;

	pev->angles.x /= 1.01;
	
}

