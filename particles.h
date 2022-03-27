#pragma once
#include "SDL.h"

enum class ParticleType //represents all of the types of particles simulated
{
	oil = 0,
	water = 1,
	acid = 2,
	lava = 3,
	sand = 4,
	gunpowder = 5,
	wood = 6,
	stone = 7,
	toxicGas = 8,
	steam = 9,
	smoke = 10,
	fire = 11,
	empty = 12
};

enum class ParticleFlag //represents all possible states of matter for the particles, used for easy updating
{
	liquid,
	solid,
	gas,
	empty
};

struct Particle //represents a single particle in the simuation
{
	ParticleType type;
	ParticleFlag flag;
	SDL_Color color;
	float xVel, yVel;
	bool updated;

	union
	{
		struct //for moveablesolids
		{
			int lastX, lastY;
			bool freeFall;
		};
		struct //for fire
		{
			int health;
			ParticleType oldType;
			ParticleFlag oldFlag;
			SDL_Color oldColor;
		};
	};
};

//---------------------------------------------------------------//

void update_oil(int x, int y); //updates the oil particle at the given position
void update_water(int x, int y); //updates the water particle at the given position
void update_acid(int x, int y); //updates the acid particle at the given position
void update_lava(int x, int y); //updates the lava particle at the given position
void update_sand(int x, int y); //updates the sand particle at the given position
void update_gunpowder(int x, int y); //updates the gunpowder particle at the given position
void update_toxic_gas(int x, int y); //updates the toxic gas particle at the given position
void update_steam(int x, int y); //updates the steam particle at the given position
void update_smoke(int x, int y); //updates the smoke particle at the given position
void update_fire(int x, int y); //updates the fire particle at the given position