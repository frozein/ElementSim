#pragma once
#include "particles.h"
#include "SDL.h"

//global constants:
#define PARTICLE_SIZE 4 //the size in pixels of every particle on the screen
#define WIDTH 256 //the width of the grid
#define HEIGHT 128 //the height of the grid
extern bool running; //whether or not the simulation is currently running

//color vars:
const SDL_Color OIL_COLOR = { 162, 109, 63 };
const SDL_Color WATER_COLOR = { 51, 136, 222 };
const SDL_Color ACID_COLOR = { 90, 181, 82 };
const SDL_Color LAVA_COLOR = { 222, 93, 58 };
const SDL_Color SAND_COLOR = { 216, 200, 90 };
const SDL_Color GUNPOWDER_COLOR = { 121, 117, 117 };
const SDL_Color WOOD_COLOR = { 110, 76, 48, };
const SDL_Color STONE_COLOR = { 190, 190, 190 };
const SDL_Color TOXIC_GAS_COLOR = { 157, 230, 78 };
const SDL_Color STEAM_COLOR = { 204, 209, 229 };
const SDL_Color SMOKE_COLOR = { 17, 14, 12 };
const SDL_Color FIRE_COLOR = { 233, 133, 55 };
const SDL_Color EMPTY_COLOR = { 40, 40, 41 };

//---------------------------------------------------------------//

bool init_simulation(SDL_Window* newWindow); //initializes the simulation; returns true on success, false on failure
void close_simulation(); //ends the simulation and cleans up memory
void run_simulation(); //runs one frame of the simulation

void render(); //renders one frame of the simulation
void handle_input(); //grabs and handles the user input
void add_particles(ParticleType type, int brushSize, int x, int y); //adds a large amount of particles to the simulation based on the parameters

Particle* get_p(int x, int y); //returns the particle at the given position; DOES NOT CHECK IF IN BOUNDS
bool in_bounds(int x, int y); //returns true if the position is in bounds, false otherwise
void swap(int x1, int y1, int x2, int y2); //swaps the particles at the given positions
void set_empty(int x, int y); //sets the particle at the given position to an empty one
unsigned int get_color(SDL_Color color); //returns the properly formatted color for the given SDL_Color