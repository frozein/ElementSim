#include "particles.h"
#include "simulation.h"
#include <cmath>

//universal constants:
#define MAX_VELOCITY 3.0f
#define GRAVITY_ACCELERATION 0.1f
#define FRICTION 0.5f

//liquid constants:
#define WATER_SPREAD_DISTANCE 4
#define ACID_SPREAD_DISTANCE 2
#define LAVA_SPREAD_DISTANCE 1

//acid constants:
const int CORROSION_CONSTANTS[13] = { 0, 0, 0, 0, 50, 50, 30, 60, 0, 0, 0, 0, 0 };

//moveable solid constants:
#define SAND_SPREAD 1.5f
#define SAND_INERTIAL_RESISTANCE 2
#define SAND_SLIP_CHANCE 1000
#define GUNPOWDER_SPREAD 2.0f
#define GUNPOWDER_INERTIAL_RESISTANCE 4
#define GUNPOWDER_SLIP_CHANCE 3000

//gas constants:
#define STEAM_BASE_HEALTH 300
#define SMOKE_BASE_HEALTH 400

//fire constants:
#define EXTINGUISH_CHANCE 20
#define SMOKE_CHANCE 30
const int FLAMMABILITY_CONSTANTS[13] = {10, -2, -1, 0, 0, 12, 60, 0, 12, 0, 0, 0, 0};
const int BASE_FIRE_HEALTH[13] = {50, 0, 0, 0, 0, 20, 200, 0, 50, 0, 0, 0, 0};

//---------------------------------------------------------------//

//liquid helper functions:
void update_liquid(int x, int y, int spreadDist); //generically updates a liquid (such as water, acid, etc.)
bool density_check(int x1, int y1, int x2, int y2); //returns true if the particle at position 1 has a greater density than that at position 2; DOES NOT CHECK FOR IN BOUNDS AND ASSUMES PARTICLE 1 IS A LIQUID
bool lava_check(int x, int y); //returns true if the particle at the given position is lava, will turn the lava to stone if so
bool corrosion_check(int x, int y); //returns true if the particle at the given position passed a random test and should be corroded

//solid helper functions:
void update_moveable_solid(int x, int y, float spread, int inrResist, int slipChance); //generically updates a moveable solid (such as sand, gunpowder, etc.)

//gas helper functions:
void update_gas(int x, int y); //generically updates a gas (steam, smoke, etc.)

//fire helper functions:
bool flammability_check(int x, int y, bool steam); //sets the given particle on fire if it passes a random test, if steam = true then water will be turned to steam

//---------------------------------------------------------------//

void update_oil(int x, int y)
{
	update_liquid(x, y, WATER_SPREAD_DISTANCE);
}

void update_water(int x, int y)
{
	update_liquid(x, y, WATER_SPREAD_DISTANCE);

	if (lava_check(x, y + 1) || lava_check(x, y - 1) ||
		lava_check(x + 1, y) || lava_check(x - 1, y))
	{
		Particle steam;
		steam.type = ParticleType::steam;
		steam.flag = ParticleFlag::gas;
		steam.color = STEAM_COLOR;
		steam.health = 300;
		*get_p(x, y) = steam;
	}
}

void update_acid(int x, int y)
{
	update_liquid(x, y, ACID_SPREAD_DISTANCE);

	if (corrosion_check(x, y + 1) ||
		corrosion_check(x + 1, y) || corrosion_check(x - 1, y))
	{
		Particle toxicGas;
		toxicGas.type = ParticleType::toxicGas;
		toxicGas.flag = ParticleFlag::gas;
		toxicGas.color = TOXIC_GAS_COLOR;
		*get_p(x, y) = toxicGas;
	}
}

void update_lava(int x, int y)
{
	update_liquid(x, y, LAVA_SPREAD_DISTANCE);

	flammability_check(x, y + 1, false);
	flammability_check(x + 1, y, false);
	flammability_check(x - 1, y, false);
}

void update_sand(int x, int y)
{
	update_moveable_solid(x, y, SAND_SPREAD, SAND_INERTIAL_RESISTANCE, SAND_SLIP_CHANCE);
}

void update_gunpowder(int x, int y)
{
	update_moveable_solid(x, y, GUNPOWDER_SPREAD, GUNPOWDER_INERTIAL_RESISTANCE, GUNPOWDER_SLIP_CHANCE);
}

void update_toxic_gas(int x, int y)
{
	update_gas(x, y);
}

void update_steam(int x, int y)
{
	Particle* p = get_p(x, y);

	//check and set updated:
	if (p->updated)
		return;
	p->updated = true;

	if (p->health <= 0)
	{
		Particle water;
		water.type = ParticleType::water;
		water.flag = ParticleFlag::liquid;
		water.color = WATER_COLOR;
		water.yVel = 0.0f;
		*p = water;

		return;
	}
	p->health--;

	update_gas(x, y);
}

void update_smoke(int x, int y)
{
	Particle* p = get_p(x, y);

	//check and set updated:
	if (p->updated)
		return;
	p->updated = true;

	if (p->health <= 0)
	{
		set_empty(x, y);
		return;
	}
	p->health--;

	update_gas(x, y);
}

void update_fire(int x, int y)
{
	Particle* p = get_p(x, y);

	//check if dead:
	if (p->health <= 0)
	{
		set_empty(x, y);
		return;
	}
	p->health--;

	//try to spread and destroy if in contact with liquid:
	if (flammability_check(x, y + 1, true) || flammability_check(x, y - 1, true) ||
		flammability_check(x + 1, y, true) || flammability_check(x - 1, y, true) ||
		flammability_check(x + 1, y + 1, true) || flammability_check(x + 1, y - 1, true) ||
		flammability_check(x - 1, y + 1, true) || flammability_check(x - 1, y - 1, true))
	{
		Particle newP;
		newP.type = p->oldType;
		newP.flag = p->oldFlag;
		newP.color = p->oldColor;
		newP.xVel = 0.0f;
		newP.yVel = 0.0f;

		*p = newP;
	}

	//try to spawn smoke:
	if (rand() % SMOKE_CHANCE == 1)
	{
		Particle smoke;
		smoke.type = ParticleType::smoke;
		smoke.flag = ParticleFlag::gas;
		smoke.color = SMOKE_COLOR;
		smoke.health = SMOKE_BASE_HEALTH;

		if (in_bounds(x, y - 1) && get_p(x, y - 1)->flag == ParticleFlag::empty)
			*get_p(x, y - 1) = smoke;
		else if (in_bounds(x, y + 1) && get_p(x, y + 1)->flag == ParticleFlag::empty)
			*get_p(x, y + 1) = smoke;
	}
}

//---------------------------------------------------------------//

void update_liquid(int x, int y, int spreadDist)
{
	int dir = rand() % 2 == 0 ? 1 : -1; //random direction for setting xVel and diagonal moving
	Particle* p = get_p(x, y);

	//updating y and yVel:
	bool fell = false; //for telling if the particle fell vertically
	p->yVel = fmin(p->yVel + GRAVITY_ACCELERATION, MAX_VELOCITY);
	for (int i = 0; i < round(p->yVel) + 1; i++)
	{
		if (in_bounds(x, y + 1) && (get_p(x, y + 1)->flag == ParticleFlag::empty || density_check(x, y, x, y + 1)))
		{
			if (density_check(x, y, x, y + 1))
				p->yVel = 0.0f;

			swap(x, y, x, y + 1);
			y++;

			p = get_p(x, y);
			fell = true;
		}
		else
		{
			p->yVel = 0.0;
			break;
		}
	}

	//checking for diagonal and lateral movement
	if (!fell)
	{
		if (in_bounds(x + dir, y + 1) && (get_p(x + dir, y + 1)->flag == ParticleFlag::empty || density_check(x, y, x + dir, y + 1)))
		{
			swap(x, y, x + dir, y + 1);
			y++;
			x += dir;
		}
		else if (in_bounds(x - dir, y + 1) && (get_p(x - dir, y + 1)->flag == ParticleFlag::empty || density_check(x, y, x - dir, y + 1)))
		{
			swap(x, y, x - dir, y + 1);
			y++;
			x -= dir;
		}
		else if (in_bounds(x + dir, y) && (get_p(x + dir, y)->flag == ParticleFlag::empty || density_check(x, y, x + dir, y)))
		{
			swap(x, y, x + dir, y);
			x += dir;

			//iterate to find furthest lateral movement location
			for (int i = 1; i < spreadDist; i++)
			{
				if (in_bounds(x + dir, y) && (get_p(x + dir, y)->flag == ParticleFlag::empty || density_check(x, y, x + dir, y)))
				{
					swap(x, y, x + dir, y);
					x += dir;
				}
				else
					break;
			}
		}
		else if (in_bounds(x - dir, y) && (get_p(x - dir, y)->flag == ParticleFlag::empty || density_check(x, y, x - dir, y)))
		{
			swap(x, y, x - dir, y);
			x -= dir;

			//iterate to find furthest lateral movement location
			for (int i = 1; i < spreadDist; i++)
			{
				if (in_bounds(x - dir, y) && (get_p(x - dir, y)->flag == ParticleFlag::empty || density_check(x, y, x - dir, y)))
				{
					swap(x, y, x - dir, y);
					x -= dir;
				}
				else
					break;
			}
		}
	}
}

bool density_check(int x1, int y1, int x2, int y2)
{
	return get_p(x1, y1)->type > get_p(x2, y2)->type;
}

bool lava_check(int x, int y)
{
	if (in_bounds(x, y) && get_p(x, y)->type == ParticleType::lava)
	{
		Particle stone;
		stone.type = ParticleType::stone;
		stone.flag = ParticleFlag::solid;
		stone.color = STONE_COLOR;
		*get_p(x, y) = stone;
		return true;
	}

	return false;
}

bool corrosion_check(int x, int y)
{
	if (in_bounds(x, y) && CORROSION_CONSTANTS[(int)get_p(x, y)->type] > 0 &&
		rand() % CORROSION_CONSTANTS[(int)get_p(x, y)->type] == 1)
	{
		set_empty(x, y);
		return true;
	}

	return false;
}

void update_moveable_solid(int x, int y, float spread, int inrResist, int slipChance)
{
	int dir = rand() % 2 == 0 ? 1 : -1; //random direction for setting xVel and diagonal moving
	Particle* p = get_p(x, y);

	//setting last pos:
	p->lastX = x;
	p->lastY = y;

	//updating x and xVel:
	int xVelSign = (p->xVel > 0) - (p->xVel < 0);
	for (int i = 0; i < abs(round(p->xVel)); i++)
	{
		if (in_bounds(x + xVelSign, y) && get_p(x + xVelSign, y)->flag != ParticleFlag::solid)
		{
			swap(x, y, x + xVelSign, y);
			x += xVelSign;

			p = get_p(x, y);
			if (in_bounds(x, y + 1) && get_p(x, y + 1)->flag == ParticleFlag::solid)
				p->xVel -= FRICTION;
		}
		else
		{
			p->xVel = 0.0f;
		}
	}

	//updating y and yVel:
	bool fell = false; //for telling if the particle fell vertically
	p->yVel = fmin(p->yVel + GRAVITY_ACCELERATION, MAX_VELOCITY);
	for (int i = 0; i < round(p->yVel) + 1; i++)
	{
		if (in_bounds(x, y + 1) && get_p(x, y + 1)->flag != ParticleFlag::solid)
		{
			swap(x, y, x, y + 1);
			y++;

			p = get_p(x, y);
			fell = true;
		}
		else
		{
			p->xVel = p->yVel * dir / spread;
			p->yVel = 0.0;
			break;
		}
	}

	//checking for diagonal movement (add random chance to slip):
	if (!fell && (p->freeFall || (rand() % slipChance) == 1))
	{
		if (in_bounds(x + dir, y + 1) && get_p(x + dir, y + 1)->flag != ParticleFlag::solid)
		{
			swap(x, y, x + dir, y + 1);
			y++;
			x += dir;
		}
		else if (in_bounds(x - dir, y + 1) && get_p(x - dir, y + 1)->flag != ParticleFlag::solid)
		{
			swap(x, y, x - dir, y + 1);
			y++;
			x -= dir;
		}
	}

	//setting freeFall:
	p = get_p(x, y);
	if (p->lastX == x && p->lastY == y)
		p->freeFall = false;
	else
		p->freeFall = true;

	//setting freeFall for nearby particles:
	if (p->freeFall)
	{
		bool change = rand() % inrResist == 1;

		if (in_bounds(x, y + 1) && get_p(x, y + 1)->flag == ParticleFlag::solid && !get_p(x, y + 1)->freeFall)
			get_p(x, y + 1)->freeFall = change;
		if (in_bounds(x + 1, y) && get_p(x + 1, y)->flag == ParticleFlag::solid && !get_p(x + 1, y)->freeFall)
			get_p(x + 1, y)->freeFall = change;
		if (in_bounds(x - 1, y) && get_p(x - 1, y)->flag == ParticleFlag::solid && !get_p(x - 1, y)->freeFall)
			get_p(x - 1, y)->freeFall = change;
	}
}

void update_gas(int x, int y)
{
	int dir = rand() % 2 == 0 ? 1 : -1; //random direction for diagonal and lateral motion
	bool rose = false; //for telling if the gas already rose

	//check for upward and diagonal movement (allow for both at once for a fluttery effect):
	if (in_bounds(x, y - 1) && (get_p(x, y - 1)->flag == ParticleFlag::empty || get_p(x, y - 1)->flag == ParticleFlag::liquid))
	{
		swap(x, y, x, y - 1);
		y--;
		rose = true;
	}

	if (in_bounds(x + dir, y - 1) && (get_p(x + dir, y - 1)->flag == ParticleFlag::empty || get_p(x + dir, y - 1)->flag == ParticleFlag::liquid))
	{
		swap(x, y, x + dir, y - 1);
		y--;
		x += dir;
		rose = true;
	}
	else if (in_bounds(x - dir, y - 1) && (get_p(x - dir, y - 1)->flag == ParticleFlag::empty || get_p(x - dir, y - 1)->flag == ParticleFlag::liquid))
	{
		swap(x, y, x - dir, y - 1);
		y--;
		x -= dir;
		rose = true;
	}

	//if the gas didnt rise, move laterally (like water):
	if (!rose)
	{
		if (in_bounds(x + dir, y) && (get_p(x + dir, y)->flag == ParticleFlag::empty || get_p(x + dir, y)->flag == ParticleFlag::liquid))
			swap(x, y, x + dir, y);
		if (in_bounds(x - dir, y) && (get_p(x - dir, y)->flag == ParticleFlag::empty || get_p(x - dir, y)->flag == ParticleFlag::liquid))
			swap(x, y, x - dir, y);
	}
}

bool flammability_check(int x, int y, bool steam)
{
	if (in_bounds(x, y))
	{
		switch (FLAMMABILITY_CONSTANTS[(int)get_p(x, y)->type])
		{
		case 0: //do nothing due to 0 flammability chance
			return false;
		case -1: //destroy due to liquid contact
			if (steam && rand() % EXTINGUISH_CHANCE == 1)
			{
				return true;
			}
			return false;
		case -2: //destroy and spawn steam due to water contact
			if (steam && rand() % EXTINGUISH_CHANCE == 1)
			{
				Particle steam;
				steam.type = ParticleType::steam;
				steam.flag = ParticleFlag::gas;
				steam.color = STEAM_COLOR;
				steam.health = STEAM_BASE_HEALTH;
				*get_p(x, y) = steam;

				return true;
			}
			return false;
		default: //check for random spread chance and set to fire
		{
			if (rand() % FLAMMABILITY_CONSTANTS[(int)get_p(x, y)->type] == 1)
			{
				Particle* oldP = get_p(x, y);
				Particle newFire;
				newFire.type = ParticleType::fire;
				newFire.flag = ParticleFlag::solid;
				newFire.color = FIRE_COLOR;
				newFire.health = BASE_FIRE_HEALTH[(int)oldP->type];
				newFire.oldType = oldP->type;
				newFire.oldFlag = oldP->flag;
				newFire.oldColor = oldP->color;

				*oldP = newFire;
			}
			return false;
		}
		}
	}

	return false;
}