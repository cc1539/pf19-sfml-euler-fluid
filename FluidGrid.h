#pragma once

#ifndef __FLUIDGRID_H__
#define __FLUIDGRID_H__

#include <SFML/Graphics.hpp>

#define min(A,B) (A<B?A:B)
#define max(A,B) (A>B?A:B)

class FluidGrid
{
private:

	float dt; // speed of the sim
	float viscosity; // diffuse rate
	int iterations; // for pressure solving
	float heat_force; // how much acceleration one unit of ink applies

	int width;
	int height;

	float* u; // x velocity
	float* v; // y velocity
	float* p; // pressure
	float* ink; // ink

	float* new_u; // new x velocity
	float* new_v; // new y velocity
	float* new_ink; // new ink

	sf::Image image;
	sf::Texture texture;
	sf::Sprite sprite;

	sf::Image ink_palette;

	sf::Color smoothSample(sf::Image palette, float i);

public:
	
	FluidGrid(int width, int height);

	void usePalette(sf::Image* palette);

	void setSpeed(float value);
	void setViscosity(float value);
	void setIterations(int value);
	void setHeatForce(float value);

	void swapBuffers();

	int getIndex(int i, int j);
	float get(float* arr, int i, int j);
	float getSample(float* arr, float x, float y);
	float getLaplacian(float* arr, int x, int y);

	void advect();
	void diffuse();
	void updatePressure();
	void subtractPressureGradient();
	void applyPressure();
	void clearBorders();
	void applyHeatForce();
	void timeStep();

	void draw(sf::RenderWindow* window, float scale);
	void draw(sf::RenderWindow* window);

	void paintOnArray(float* arr, float x, float y, float radius, float value);
	void paint(float x, float y, float radius);
	void erase(float x, float y, float radius);
	void paintVelocity(float x, float y, float radius, float vx, float vy);
};

#endif
