#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "player.h"
#include "CVehicleBase.h"
//#include "CVehicleBase_Types.h"

/***********************/
// CBaseVehicle
/***********************/

LINK_ENTITY_TO_CLASS( vehicle_base, CBaseVehicle );

void CBaseVehicle::Spawn( void )
{
	pev->solid		= SOLID_BBOX;
	pev->movetype	= MOVETYPE_PUSHSTEP;
	UTIL_SetSize( pev, Vector( -96, -96, 0 ), Vector( 96, 96, 64 ) );

	pev->effects	= 0;
	pev->friction	= 0.01;
	pev->sequence	= 0;

	VehicleInit(); // all the basic stuff goes here - wheels, engine, seats, weapons, sounds to use
	Precache(); // here we precache that stuff

	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	pev->takedamage = DAMAGE_YES;

	SetUse( &CBaseVehicle::VehicleEnterUse );
	SetThink( &CBaseVehicle::VehicleThink );
	SetTouch( &CBaseVehicle::VehicleTouch );

	VehicleBaseInit(); // and here we apply the model, hence I had to precache between Init and BaseInit

	UTIL_SetOrigin( pev, pev->origin );

	pev->nextthink = gpGlobals->time + 0.1; // They said this is a magic number. :o
}

void CBaseVehicle::Precache( void )
{
	PRECACHE_MODEL( (char*)STRING( pev->model ) );
}

void CBaseVehicle::KeyValue( KeyValueData * pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "MaxSpeed" ) )
	{
		m_flMaxSpeed = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "SeatBoneOfs" ) )
	{
		m_iSeatBoneOffset = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue( pkvd );
	}
}

void CBaseVehicle::VehicleBaseInit()
{
//	if ( v_Body.m_iszModel )
//		pev->model = v_Body.m_iszModel;

	SET_MODEL( ENT( pev ), STRING( pev->model ) );

//	pev->max_health = v_Engine.MaxHealth;
	pev->max_health = 1000; // hardcoded for now, sorry :3
	pev->health = pev->max_health;

//	v_Body.pos = pev->origin;
}

void CBaseVehicle::VehicleInit()
{
	// All the properties are initialised here.
	// Engine properties like HP, the drive, efficiency,
	// seats and their properties, and the body properties.

	// Heaven:
	// v_Body.Mass = Script_Read( "v_Body.mass" );

//	v_Body.Mass = 30; // kg
//	v_Body.Density = 670; // kg/m^3, Birch wood, European

	v_Seat.Init( Driver, pev->origin, 0 );

//	v_Engine.Efficiency = 1.0f;
//	v_Engine.GearRatios[ 0 ] = -4.2f;
//	v_Engine.GearRatios[ 1 ] = 0.0f;
//	v_Engine.GearRatios[ 2 ] = 4.2f;
//	v_Engine.GearRatios[ 3 ] = 2.8f;
//	v_Engine.GearRatios[ 4 ] = 1.8f;
//	v_Engine.GearRatios[ 5 ] = 1.2f;
//	v_Engine.GearRatios[ 6 ] = 1.0f;

//	v_Engine.Drive = Drive_NoWheels;

	v_Type = VehicleSpecialTest;
}

void CBaseVehicle::VehicleEnterUse( CBaseEntity * pActivator, CBaseEntity * pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() || ((CBasePlayer*)pActivator)->m_InVehicle != InWalking )
	{
		return;
	}
	else
	{
		static_cast<CBasePlayer*>(pActivator)->m_InVehicle = static_cast<InVehicleType>(v_Type + 1);
		
		if ( v_Seat.pSessilis == NULL && v_Seat.fExists )
		{
			v_Seat.pSessilis = (CBasePlayer *)pActivator;
			v_Seat.pSessilis->pev->movetype = MOVETYPE_NOCLIP;
			v_Seat.pSessilis->pev->solid = SOLID_NOT;
			v_Seat.SeatPlayer();
			if ( v_Seat.pSessilis )
				v_Seat.pSessilis->pev->angles = pev->angles;
		}

		else
		{
			ALERT( at_console, "\nSeat%d not free", 0 );
		}
	}
}

void CBaseVehicle::VehicleExit( void )
{
	v_Seat.Exit();
}

void CBaseVehicle::VehicleThink( void )
{
	v_Seat.AttachToPos( *this, m_iSeatBoneOffset );
	v_Seat.SeatPlayer();
	v_Seat.ListenToCommands();

	// Listen to very basic commands for all seats and execute them
	if ( v_Seat.iCommands & fVehKey_Use && 
		 v_Seat.iCommands & fVehKey_Combokey )
		VehicleExit();

	// Trick the player into thinking he's on the ground
	if ( v_Seat.pSessilis )
		v_Seat.pSessilis->pev->flags |= FL_ONGROUND;

	// This forms a 128x128x96 box around the vehicle, but offset diagonally a little bit
	// I don't know why the bounding box gets offset from the vehicle, must be an engine thing
	// Until we figure out how to do this automatically, each vehicle class should define a custom 
	// AABB via UTIL_SetSize() in its Think function
	// Also, calling UTIL_SetSize periodically is mandatory for complex collision via hitboxes
	UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 96, 96, 96 ) );

	VehicleMove();

	// Dunno if SetOrigin is required at this point - certainly breaks MOVETYPE_FLY
	UTIL_SetOrigin( pev, pev->origin );
	pev->nextthink = gpGlobals->time + 0.01;
}

void CBaseVehicle::VehicleTouch( CBaseEntity *pOther )
{
	ALERT( at_console, "CBaseVehicle::VehicleTouch()\n" );

	// bounce if we hit something solid
	if ( pOther->pev->solid == SOLID_BSP )
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		// UNDONE, do a real bounce
		pev->velocity = pev->velocity + tr.vecPlaneNormal * (pev->velocity.Length() + 200);
	}
}

void CBaseVehicle::VehicleMove( void )
{
	static float flAccelerate = 0.0f;

	if ( v_Seat.iCommands & fVehKey_Forward )
		flAccelerate += 0.1;
	else
		flAccelerate /= 1.01;

	pev->angles.x *= -1;
	UTIL_MakeAimVectors( pev->angles );
	pev->angles.x *= -1;
	
	if ( v_Type != VehicleBoat )
	{
		if ( pev->flags & FL_ONGROUND )
			pev->velocity = pev->velocity + (gpGlobals->v_forward * flAccelerate);
		else
			pev->velocity = pev->velocity / 1.005;
	}
	else
	{
		//if ( !(pev->flags & FL_ONGROUND) )
			pev->velocity = pev->velocity + (gpGlobals->v_forward * flAccelerate);
		//else
			pev->velocity = pev->velocity + Vector( 0, 0, 0.1 );
	}

}

/***********************/
// VehicleSeat
/***********************/

int VehicleSeat::SeatPlayer()
{
	if ( pSessilis == nullptr )
		return 0; // Fail, no sitting player

	UTIL_SetOrigin( pSessilis->pev, pos );
	pSessilis->pev->angles = angles;

	return 1; // Success
}

void VehicleSeat::SeatSwitch( VehicleSeat &nextSeat )
{
	if ( pSessilis == nullptr )
	{
		ALERT( at_aiconsole, "\nSeatSwitch() fail at Seat%d - no sitting player", iSitdex );
		return; // crashy crashy gamey if this check don't worky
	}
	nextSeat.pSessilis = pSessilis;
	pSessilis = nullptr;

	FlushSeatCommands();

	ALERT( at_aiconsole, "\nSeat switched to Seat%d!", nextSeat.iSitdex );
}

void VehicleSeat::FlushSeatCommands() // NOT to be associated with TOILET FLUSHING, bastards - toilets are a separate entity
{
	iCommands = 0;
}

void VehicleSeat::ListenToCommands()
{
	if ( pSessilis == nullptr )
	{
		// I'm deaf to all the commands. I ain't gonna listen.
		// In other words, we don't have a player in this seat.
		return;
	}

	if ( type == Passenger )
	{
		// Don't listen to anything else and don't bother for the rest of the ifs.
		// In other words, a passenger doesn't have to do much. :p
		return;
	}

	if ( type == Driver || type == DriverGunner )
	{
		CheckCommand( fVehKey_Forward,	IN_FORWARD );
		CheckCommand( fVehKey_Backward, IN_BACK );
		CheckCommand( fVehKey_Left,		IN_MOVELEFT );
		CheckCommand( fVehKey_Right,	IN_MOVERIGHT );
		CheckCommand( fVehKey_Handbrake,IN_JUMP );
		CheckCommand( fVehKey_Use,		IN_USE );
		CheckCommand( fVehKey_Combokey,	IN_RELOAD );

	}

	if ( type == Gunner || type == DriverGunner )
	{
		CheckCommand( fVehKey_Fire, IN_ATTACK );
	}
}

void VehicleSeat::CheckCommand( int outToSet, int inToCheck )
{
	if ( pSessilis->pev->button & inToCheck )
		iCommands |= outToSet;
	else
		iCommands &= ~(outToSet);
}

void VehicleSeat::Exit()
{
	if ( pSessilis == nullptr )
		return;

	FlushSeatCommands();
	pSessilis->pev->origin.z	+= 64;				// YEET! Throw the player out of the seat
	pSessilis->pev->movetype	= MOVETYPE_WALK;	// TO-DO: algorithm to figure out the most optimal exit position
	pSessilis->pev->solid		= SOLID_BBOX;
	pSessilis->m_InVehicle		= InWalking;

	pSessilis					= nullptr;
}

void VehicleSeat::Init( VehicleSeatType intype, vec3_t inpos, int sitdex )
{
	type		= intype;
	fExists		= true;
	pSessilis	= nullptr;
	pos			= inpos;
	pos.z		-= 48;
	iSitdex		= sitdex;

	FlushSeatCommands();
}

void VehicleSeat::AttachToPos( CBaseVehicle &Vehicle, int iBoneOffset )
{
	int iBone = iSitdex + iBoneOffset + Vehicle.GetNumWheels();

	auto v_Type = Vehicle.GetVehType();

	GetBoneId( iBone, v_Type );
	GET_BONE_POSITION( ENT( Vehicle.pev ), iBone, pos, angles );

	#ifdef DEBUG
	ALERT( at_aiconsole, "\nAttachToPos::iBone = %d", iBone );
	#endif
}

void GetBoneId( int &iBone, VehicleType vType )
{
	switch ( vType )
	{
	case VehicleCar:
		iBone += 1; // 1 bone for the steering wheel + m_iWheels usually 4
		break;

	case VehicleBike:
		iBone += 1; // engine/pedal bone + m_iWheels usually 2
		break;

	case VehiclePlane:
		iBone += 4; // 2 propeller bones + 2 gear

	case VehicleHelicopter:
		iBone += 2; // 2 propeller bones
		break;

	case VehicleBoat:
		iBone += 1; // 1 propeller bone
		break;

	case VehicleChair:
		// It is already at root, so
	case VehicleShip:
		// space ships shouldn't really have any bones other than the root and seats
	case VehicleNPC:
		// NO
	case VehicleSpecialTest:
		// No, we cannot predict which bone is which :v
		// Instead, let the mapper define the bone offset
		break;
	}
}

void UTIL_Deg2Rad( float &in )
{
	in *= M_PI / 180;
}

// Not actually used, can cause bugs
Vector AlignToGround( Vector origin, Vector dirangles, float radius, edict_t *pentIgnore )
{
	TraceResult trDown;

	UTIL_TraceLine( origin, origin - Vector( 0, 0, 256 ), ignore_monsters, pentIgnore, &trDown );

	Vector vecAngles, vecFinalAngles, angdir, angdiry;

	vecAngles = dirangles;

	UTIL_Deg2Rad( vecAngles.x );
	UTIL_Deg2Rad( vecAngles.y );
	UTIL_Deg2Rad( vecAngles.z );

	angdir.x = cos( vecAngles.y ) * cos( vecAngles.x );
	angdir.y = sin( vecAngles.y ) * cos( vecAngles.x );
	angdir.z = -sin( vecAngles.z );

	angdiry.x = sin( vecAngles.y ) * cos( vecAngles.x );
	angdiry.y = sin( vecAngles.y ) * cos( vecAngles.x );
	angdiry.z = -sin( vecAngles.x );

	vecFinalAngles = UTIL_VecToAngles( angdir - DotProduct( angdir, trDown.vecPlaneNormal ) * trDown.vecPlaneNormal );
	vecFinalAngles.z = -UTIL_VecToAngles( angdiry - DotProduct( angdiry, trDown.vecPlaneNormal ) * trDown.vecPlaneNormal ).x;

	return vecFinalAngles;
}
