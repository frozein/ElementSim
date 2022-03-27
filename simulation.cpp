#include "simulation.h"
#include "particles.h"
#include "SDL_image.h"
#include <iostream>
#include <cmath>
#include <time.h>

//global vars:
static Particle* grid; //the entire grid of simulated particles
SDL_Window* window; //the SDL window
bool running;

//ui surfaces:
SDL_Surface* particleNames;
SDL_Rect namesSrcRect;
SDL_Surface* brushSizes;
SDL_Rect brushSizeSrcRect;
SDL_Surface* instructions;
bool displayInstructions;

//---------------------------------------------------------------//

void inner_sim_loop(int x); //used to allow for alternating iteration direction

bool init_simulation(SDL_Window* newWindow)
{
	//set window:
	window = newWindow;

	//generate surfaces:
	particleNames = IMG_Load("assets/elementNames.png");
	brushSizes = IMG_Load("assets/brushSizes.png");
	instructions = IMG_Load("assets/instructions.png");
	namesSrcRect.x = 0;
	namesSrcRect.y = 28;
	namesSrcRect.w = 63;
	namesSrcRect.h = 7;
	brushSizeSrcRect.x = 0;
	brushSizeSrcRect.y = 7;
	brushSizeSrcRect.w = 89;
	brushSizeSrcRect.h = 7;

	//seed rng:
	srand((unsigned int)time(NULL));

	//initialize map:
	displayInstructions = true;
	grid = new Particle[WIDTH * HEIGHT];
	if (!grid)
		return false;

	//default everything to empty:
	for (int x = 0; x < WIDTH; x++)
		for (int y = 0; y < HEIGHT; y++)
			set_empty(x, y);

	return true;
}

void close_simulation()
{
	delete[] grid;
	SDL_FreeSurface(particleNames);
	SDL_FreeSurface(brushSizes);
	SDL_FreeSurface(instructions);
}

void run_simulation()
{
	static bool dir = true; //for alternating iteration direction
	dir = !dir;

	//iterate either left->right or right->left to ensure sand/water spreads evenly:
	if(dir)
	{
		for (int x = 0; x < WIDTH; x++)
			inner_sim_loop(x);
	}
	else
	{
		for (int x = WIDTH - 1; x >= 0; x--)
			inner_sim_loop(x);
	}

	//reset all updated variables:
	for (int x = 0; x < WIDTH; x++)
		for (int y = 0; y < HEIGHT; y++)
			grid[x + y * WIDTH].updated = false;
}

void inner_sim_loop(int x)
{
	//iterate over each cell and switch over its type:
	for (int y = HEIGHT - 1; y >= 0; y--)
	{
		switch (grid[x + y * WIDTH].type)
		{
		case ParticleType::oil:
			update_oil(x, y);
			break;
		case ParticleType::water:
			update_water(x, y);
			break;
		case ParticleType::acid:
			update_acid(x, y);
			break;
		case ParticleType::lava:
			update_lava(x, y);
			break;
		case ParticleType::sand:
			update_sand(x, y);
			break;
		case ParticleType::gunpowder:
			update_gunpowder(x, y);
			break;
		case ParticleType::toxicGas:
			update_toxic_gas(x, y);
			break;
		case ParticleType::steam:
			update_steam(x, y);
			break;
		case ParticleType::smoke:
			update_smoke(x, y);
			break;
		case ParticleType::fire:
			update_fire(x, y);
			break;
		default:
			break;
		}
	}
}

void render()
{
	//grab window stuff:
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
	SDL_PixelFormat* format = windowSurface->format;
	unsigned int* texture = (unsigned int*)windowSurface->pixels;

	//iterate over every cell, grab its color and render the rectangle:
	for (int x = 0; x < WIDTH; x++)
		for (int y = 0; y < HEIGHT; y++)
		{
			unsigned int pixel = get_color(grid[x + y * WIDTH].color);

			for (int i = 0; i < PARTICLE_SIZE; i++)
				for (int j = 0; j < PARTICLE_SIZE; j++)
					texture[(x * PARTICLE_SIZE + i) + (y * PARTICLE_SIZE + j) * WIDTH * PARTICLE_SIZE] = pixel;
		}

	//render the element and brush size:
	SDL_Rect namesRect;
	namesRect.x = 10;
	namesRect.y = 10;
	namesRect.w = 189;
	namesRect.h = 21;
	SDL_Rect brushSizeRect;
	brushSizeRect.x = 10;
	brushSizeRect.y = 40;
	brushSizeRect.w = 178;
	brushSizeRect.h = 14;

	SDL_BlitScaled(particleNames, &namesSrcRect, windowSurface, &namesRect);
	SDL_BlitScaled(brushSizes, &brushSizeSrcRect, windowSurface, &brushSizeRect);

	//display the instructions:
	if (displayInstructions)
	{
		SDL_Rect instructionRect;// = { (WIDTH * PARTICLE_SIZE / 2) - (447 / 2), (WIDTH * PARTICLE_SIZE / 2) - (447 / 2), 447, 171 };
		instructionRect.w = 596;
		instructionRect.h = 228;
		instructionRect.x = (WIDTH * PARTICLE_SIZE / 2) - instructionRect.w / 2;
		instructionRect.y = (HEIGHT * PARTICLE_SIZE / 2) - instructionRect.h / 2;
		SDL_BlitScaled(instructions, NULL, windowSurface, &instructionRect);
	}

	SDL_UpdateWindowSurface(window);
}

void handle_input()
{
	//declare static variables to hold persistent data about current brush:
	static SDL_Event event;
	static ParticleType particleType = ParticleType::sand;
	static int brushSize = 1; //can be 0, 1, 2

	//switch over event and grab input:
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYUP:
		{
			//switch over keys and set appropriate particle:
			switch (event.key.keysym.sym)
			{
			case SDLK_RETURN:
				displayInstructions = false;
				break;
			case SDLK_1:
				particleType = ParticleType::oil;
				namesSrcRect.y = 0;
				break;
			case SDLK_2:
				particleType = ParticleType::water;
				namesSrcRect.y = 7;
				break;
			case SDLK_3:
				particleType = ParticleType::acid;
				namesSrcRect.y = 14;
				break;
			case SDLK_4:
				particleType = ParticleType::lava;
				namesSrcRect.y = 21;
				break;
			case SDLK_5:
				particleType = ParticleType::sand;
				namesSrcRect.y = 28;
				break;
			case SDLK_6:
				particleType = ParticleType::gunpowder;
				namesSrcRect.y = 35;
				break;
			case SDLK_7:
				particleType = ParticleType::wood;
				namesSrcRect.y = 42;
				break;
			case SDLK_8:
				particleType = ParticleType::stone;
				namesSrcRect.y = 49;
				break;
			case SDLK_9:
				particleType = ParticleType::fire;
				namesSrcRect.y = 56;
				break;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			if (event.wheel.y > 0 && brushSize < 2)
			{
				brushSize++;
				brushSizeSrcRect.y += 7;
			}
			else if (event.wheel.y < 0 && brushSize > 0)
			{
				brushSize--;
				brushSizeSrcRect.y -= 7;
			}
			break;
		}
		case SDL_QUIT:
		{
			running = false; //will quit the application
			break;
		}
		}
	}

	//grab mouse input and send command to add particles if it is held:
	if (!displayInstructions)
	{
		int mouseX;
		int mouseY;
		if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT))
			add_particles(particleType, brushSize, mouseX / PARTICLE_SIZE, mouseY / PARTICLE_SIZE);
		else if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_RIGHT))
			add_particles(ParticleType::empty, brushSize, mouseX / PARTICLE_SIZE, mouseY / PARTICLE_SIZE);
	}
}

void add_particles(ParticleType type, int brushSize, int x, int y)
{
	//set universally default values:
	Particle pToAdd;
	pToAdd.type = type;
	pToAdd.xVel = 0.0f;
	pToAdd.yVel = 0.0f;
	pToAdd.updated = false;

	//switch over the type and set type-sepcific default values:
	switch (type)
	{
	case ParticleType::oil:
	{
		pToAdd.flag = ParticleFlag::liquid;
		pToAdd.color = OIL_COLOR;
		break;
	}
	case ParticleType::water:
	{
		pToAdd.flag = ParticleFlag::liquid;
		pToAdd.color = WATER_COLOR;
		break;
	}
	case ParticleType::acid:
	{
		pToAdd.flag = ParticleFlag::liquid;
		pToAdd.color = ACID_COLOR;
		break;
	}
	case ParticleType::lava:
	{
		pToAdd.flag = ParticleFlag::liquid;
		pToAdd.color = LAVA_COLOR;
		break;
	}
	case ParticleType::sand:
	{
		pToAdd.flag = ParticleFlag::solid;
		pToAdd.color = SAND_COLOR;
		pToAdd.freeFall = false;
		break;
	}
	case ParticleType::gunpowder:
	{
		pToAdd.flag = ParticleFlag::solid;
		pToAdd.color = GUNPOWDER_COLOR;
		pToAdd.freeFall = false;
		break;
	}
	case ParticleType::wood:
	{
		pToAdd.flag = ParticleFlag::solid;
		pToAdd.color = WOOD_COLOR;
		break;
	}
	case ParticleType::stone:
	{
		pToAdd.flag = ParticleFlag::solid;
		pToAdd.color = STONE_COLOR;
		break;
	}
	case ParticleType::fire:
	{
		pToAdd.flag = ParticleFlag::solid;
		pToAdd.color = FIRE_COLOR;
		pToAdd.health = 5;
		pToAdd.oldType = ParticleType::empty;
		pToAdd.oldFlag = ParticleFlag::empty;
		pToAdd.oldColor = EMPTY_COLOR;
		break;
	}
	case ParticleType::empty:
	{
		pToAdd.flag = ParticleFlag::empty;
		pToAdd.color = EMPTY_COLOR;
		break;
	}
	}

	//add the particles to the grid:
	if (brushSize == 0)
		grid[x + y * WIDTH] = pToAdd;
	else
	{
		//iterate over a square in the grid and add the particles if they are in bounds:
		for (int i = x - brushSize; i <= x + brushSize; i++)
			for (int j = y - brushSize; j <= y + brushSize; j++)
				if (in_bounds(i, j) && (type == ParticleType::empty || grid[i + j * WIDTH].type == ParticleType::empty)) //don't add if they are the same type, avoids the particles getting stuck in the air due to the velocity resetting
				{
					pToAdd.lastX = x;
					pToAdd.lastY = y;
					grid[i + j * WIDTH] = pToAdd;
				}
	}
}

Particle* get_p(int x, int y)
{
	return &grid[x + y * WIDTH];
}

bool in_bounds(int x, int y)
{
	if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1))
		return false;

	return true;
}

void swap(int x1, int y1, int x2, int y2)
{
	Particle temp = grid[x1 + y1 * WIDTH];
	grid[x1 + y1 * WIDTH] = grid[x2 + y2 * WIDTH];
	grid[x2 + y2 * WIDTH] = temp;
}

void set_empty(int x, int y)
{
	int idx = x + y * WIDTH;
	grid[idx].type = ParticleType::empty;
	grid[idx].flag = ParticleFlag::empty;
	grid[idx].color = EMPTY_COLOR;
}

unsigned int get_color(SDL_Color color)
{
	static SDL_PixelFormat* format = SDL_GetWindowSurface(window)->format;
	return SDL_MapRGB(format, color.r, color.g, color.b);
}