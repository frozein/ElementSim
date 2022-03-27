#include "simulation.h"
#include "SDL_image.h"
#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char** argv)
{
	//declare window and start running:
	SDL_Window* window;
	running = true;

	//initialize SDL and check for errors along the way:
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window = SDL_CreateWindow("ElementSim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * PARTICLE_SIZE, HEIGHT * PARTICLE_SIZE, SDL_WINDOW_SHOWN);
		if(!window)
			return 0;

		SDL_Surface* icon = IMG_Load("assets/icon.png");
		SDL_SetWindowIcon(window, icon);
		SDL_FreeSurface(icon);
	}
	else
		return 0;

	//initialize the simulation:
	if (!init_simulation(window))
		return 0;

	//declare timestepping variables:
	using clock = std::chrono::steady_clock;

	const double frameDelay = 1000 / 60; //fps
	clock::time_point start = clock::now();
	clock::time_point last = clock::now();

	//timestep until running = false:
	while (running)
	{
		start = clock::now();
		std::chrono::duration<double, std::milli> workTime = start - last;
		//std::cout << workTime.count() << std::endl;

		if (workTime.count() < frameDelay)
		{
			std::chrono::duration<double, std::milli> deltaMs(frameDelay - workTime.count());
			auto deltaMsDuration = std::chrono::duration_cast<std::chrono::milliseconds>(deltaMs);
			std::this_thread::sleep_for(std::chrono::milliseconds(deltaMsDuration.count())); //sleep to keep fps capped
		}

		last = clock::now();

		handle_input();
		run_simulation();
		render();
	}

	//clean up before exiting:
	close_simulation();
	SDL_DestroyWindow(window);

	return 0;
}