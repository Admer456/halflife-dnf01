#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "player.h"
#include "CVehicleBase.h"

constexpr char szBikeIdleSound[] = "vehicles/bike_idle.wav";
constexpr char szBikeRevSound[]  = "vehicles/bike_rev.wav";

class CVehicleBike : public CBaseVehicle
{
public:
	void VehicleInit( void );
	void VehicleMove( void );
};

LINK_ENTITY_TO_CLASS( vehicle_bike, CVehicleBike );

void CVehicleBike::VehicleInit( void )
{
	v_Seat.Init( DriverGunner, pev->origin, 0 );
	v_Type = VehicleBike;
	m_iWheels = 2;

	PRECACHE_SOUND( (char*)szBikeIdleSound );
	PRECACHE_SOUND( (char*)szBikeRevSound );
}

void CVehicleBike::VehicleMove( void )
{
	static float steerAngle = 0.0;
	static float wheelAngle = 0.0;
	static bool isRevving = false;

	// Basic movement
	pev->friction = 0.5;
	CBaseVehicle::VehicleMove();

	if ( v_Seat.iCommands & fVehKey_Forward || v_Seat.iCommands & fVehKey_Backward )
	{
		pev->velocity.z += 0.1; // The vehicle is constantly above ground by 0.1 units, otherwise it's too much friction on the ground
	}

	// Braking
	if ( v_Seat.iCommands & fVehKey_Backward )
	{
		pev->friction = 0.8;

		if ( pev->flags & FL_ONGROUND )
		{
			pev->velocity.x /= 1.05;
			pev->velocity.y /= 1.05;
		}
	}

	// Steering
	if ( v_Seat.iCommands & fVehKey_Left )
	{
		if ( pev->velocity.Length2D() < 400.0 )
		{
			steerAngle += 4.0;
		}
		else
		{
			steerAngle += 4.0 / (pev->velocity.Length2D() / 400.0);
		}
	}
	if ( v_Seat.iCommands & fVehKey_Right )
	{
		if( pev->velocity.Length2D() < 400.0 )
		{
			steerAngle -= 4.0;
		}
		else
		{
			steerAngle -= 4.0 / (pev->velocity.Length2D() / 400.0);
		}
	}

	if ( abs( steerAngle ) > 50.0 )
		steerAngle /= 1.01;
	
	steerAngle /= 1.05;

	// Modifying the actual vehicle angles, depending on the steering angle
	if ( pev->velocity.Length2D() > 200.0 )
		pev->angles.y += steerAngle / 10.0;
	else
		pev->angles.y += (steerAngle * (pev->velocity.Length2D() / 200)) / 10.0;

	SetBoneController( 0, steerAngle );
	
	// Pitch adjustment
	float newPitch = pev->angles.x;

	if ( pev->velocity.Length2D() > 20.0 && abs( pev->velocity.z ) > 15.0 )
	{
		newPitch = UTIL_VecToAngles( pev->velocity ).x / 2.0;
	}
	else
	{
		newPitch /= 1.1;
	}

	// One way to fix the stupid Quake bug
	// else our model gets flipped upside down
	if ( abs( newPitch ) > 120 )
	{
		newPitch -= 180.0;
	}

	pev->angles.x = pev->angles.x * 0.94 + newPitch * 0.06;

	pev->angles.z = -steerAngle / 4.0;

	// Speed limit
	if ( pev->velocity.Length2D() > m_flMaxSpeed && pev->flags & FL_ONGROUND )
		pev->velocity = pev->velocity / 1.07;

	// Spinning of wheels
	if ( pev->flags & FL_ONGROUND )
		wheelAngle += pev->velocity.Length() * 0.03;
	else
		wheelAngle += pev->velocity.Length() * 0.2;
	
	if ( wheelAngle > 360.0 )
	{
		wheelAngle -= 360.0;
	}

	SetBoneController( 1, wheelAngle );

	// Sound management
	// TO-DO: Move to another function and have different sounds for different speeds
	if ( v_Seat.iCommands & fVehKey_Forward )
	{
		if ( !isRevving )
		{
			STOP_SOUND( ENT( pev ), CHAN_STATIC, szBikeIdleSound );
			EMIT_SOUND( ENT( pev ), CHAN_STATIC, szBikeRevSound, 1.0, ATTN_NORM );
			isRevving = true;
		}
	}
	else
	{
		if ( isRevving )
		{
			STOP_SOUND( ENT( pev ), CHAN_STATIC, szBikeRevSound );
			EMIT_SOUND( ENT( pev ), CHAN_STATIC, szBikeIdleSound, 1.0, ATTN_NORM );
			isRevving = false;
		}
	}
}