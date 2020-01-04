#pragma once

enum InVehicleType
{
	InWalking,
	InChair,
	InCar,
	InBike,
	InPlane,
	InHelicopter,
	InShip,
	InBoat,
	InNPC, // Ahem, ON the NPC, unless it vored you, YOU FURRY PRICK
	InSpecialTest,
};

enum VehicleType
{
	VehicleChair = 0,
	VehicleCar, // automobiles, trucks
	VehicleBike, // bicycles, motorbikes
	VehiclePlane, // airplanes, jets
	VehicleHelicopter, // helicopters, drones
	VehicleShip, // spaceships, UFOs
	VehicleBoat, // boats, bath tubs, sleds
	VehicleNPC, // special, rideable NPCs. Think of horses, or no... GARGANTUAS >:D
	VehicleSpecialTest // chair that is like a car, needs no wheels
};

enum VehicleSeatType
{
	Driver = 0,
	Gunner,
	Passenger,
	DriverGunner
};

enum RubberType
{
	Stock = 0,
	Comfort,
	Performance,
	Rally
};

enum VehicleDrive
{
	Drive_FWD = 0,
	Drive_RWD,
	Drive_AWD,
	Drive_NoWheels
};

struct VehicleTorqueCurve // What torque in Nm do we have, at which RPM
{
	float Torque;
	float Rpm;
};
