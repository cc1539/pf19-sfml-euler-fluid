#pragma once

#include "FluidGrid.h"

FluidGrid::FluidGrid(int width, int height)
{
	this->width = width;
	this->height = height;

	int length = width*height;
	u = new float[length];
	v = new float[length];
	p = new float[length];
	ink = new float[length];
	new_u = new float[length];
	new_v = new float[length];
	new_ink = new float[length];

	for (int i = 0; i < length; i++) {
		u[i] = 0;
		v[i] = 0;
		p[i] = 0;
		ink[i] = 0;
		new_u[i] = 0;
		new_v[i] = 0;
		new_ink[i] = 0;
	}

	image.create(width, height);
	texture.create(width, height);
	sprite.setTexture(texture);
	
	// default palette
	ink_palette.create(2, 1);
	ink_palette.setPixel(0, 0, sf::Color::Black);
	ink_palette.setPixel(1, 0, sf::Color::White);
}

sf::Color FluidGrid::smoothSample(sf::Image palette, float i)
{
	int palette_width = ink_palette.getSize().x - 1;

	i *= palette_width;

	int bx = (int)i;
	int px = bx + 1;
	bx = min(max(bx, 0), palette_width);
	px = min(max(px, 0), palette_width);
	float lx = i - bx;

	sf::Color p_value = palette.getPixel(px,0);
	sf::Color n_value = palette.getPixel(bx,0);

	return sf::Color(
		n_value.r*(1 - lx) + p_value.r*lx, 
		n_value.g*(1 - lx) + p_value.g*lx,
		n_value.b*(1 - lx) + p_value.b*lx
	);
}

void FluidGrid::usePalette(sf::Image* palette)
{
	ink_palette = *palette;
}

void FluidGrid::setSpeed(float value)
{
	dt = value;
}

void FluidGrid::setViscosity(float value)
{
	viscosity = value;
}

void FluidGrid::setIterations(int value)
{
	iterations = value;
}

void FluidGrid::setHeatForce(float value)
{
	heat_force = value;
}

void FluidGrid::swapBuffers()
{
	{ float* temp = u; u = new_u; new_u = temp; }
	{ float* temp = v; v = new_v; new_v = temp; }
	{ float* temp = ink; ink = new_ink; new_ink = temp; }
}

int FluidGrid::getIndex(int i, int j)
{
	return i + j*width;
}

float FluidGrid::get(float* arr, int i, int j)
{
	return arr[getIndex(i, j)];
}

float FluidGrid::getSample(float* arr, float x, float y)
{
	int bx = (int)x;
	int by = (int)y;
	int px = bx + 1;
	int py = by + 1;
	float lx = x - bx;
	float ly = y - by;
	bool bx_valid = bx >= 0 && bx < width;
	bool by_valid = by >= 0 && by < height;
	bool px_valid = px >= 0 && px < width;
	bool py_valid = py >= 0 && py < height;
	float n00 = bx_valid && by_valid ? get(arr, bx, by) : 0;
	float n10 = px_valid && by_valid ? get(arr, px, by) : 0;
	float n01 = bx_valid && py_valid ? get(arr, bx, py) : 0;
	float n11 = px_valid && py_valid ? get(arr, px, py) : 0;
	return (n00*(1 - lx) + n10*lx)*(1 - ly) + (n01*(1 - lx) + n11*lx)*ly;
}

float FluidGrid::getLaplacian(float* arr, int x, int y)
{
	float sum = 0;
	for (int u = -1; u <= 1; u++)
	{
		for (int v = -1; v <= 1; v++)
		{
			if (!(u == 0 && v == 0))
			{
				int i = x + u;
				if (i < 0 || i >= width)
				{
					continue;
				}

				int j = y + v;
				if (j < 0 || j >= height)
				{
					continue;
				}

				sum += get(arr, i, j);
			}
		}
	}
	return sum / 8.0;
}

void FluidGrid::advect()
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int index = getIndex(i, j);
			float sample_x = i - u[index] * dt;
			float sample_y = j - v[index] * dt;
			new_u[index] = getSample(u, sample_x, sample_y);
			new_v[index] = getSample(v, sample_x, sample_y);
			new_ink[index] = getSample(ink, sample_x, sample_y);
		}
	}
	swapBuffers();
}

void FluidGrid::diffuse()
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int index = getIndex(i, j);
			new_u[index] = u[index] + (getLaplacian(u, i, j) - u[index]) * viscosity;
			new_v[index] = v[index] + (getLaplacian(v, i, j) - v[index]) * viscosity;
			new_ink[index] = ink[index];
		}
	}
	swapBuffers();
}

void FluidGrid::updatePressure()
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			float nx = i - 1 >= 0 ? get(u, i - 1, j) : 0;
			float px = i + 1 < width ? get(u, i + 1, j) : 0;
			float ny = j - 1 >= 0 ? get(v, i, j - 1) : 0;
			float py = j + 1 < height ? get(v, i, j + 1) : 0;
			p[getIndex(i, j)] = ((nx - px) + (ny - py)) / 4.0;
		}
	}
}

void FluidGrid::subtractPressureGradient()
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int index = getIndex(i, j);
			float nx = i - 1 >= 0 ? get(p, i - 1, j) : 0;
			float px = i + 1 < width ? get(p, i + 1, j) : 0;
			float ny = j - 1 >= 0 ? get(p, i, j - 1) : 0;
			float py = j + 1 < height ? get(p, i, j + 1) : 0;
			new_u[index] = u[index] + (nx - px);
			new_v[index] = v[index] + (ny - py);
			new_ink[index] = ink[index];
		}
	}
	swapBuffers();
}

void FluidGrid::applyPressure()
{
	for (int i = 0; i < iterations; i++)
	{
		updatePressure();
		subtractPressureGradient();
	}
}

void FluidGrid::clearBorders()
{
	for (int i = 0; i < width; i++)
	{
		int far_index = i + (height - 1)*width;
		u[i] = 0;
		u[far_index] = 0;
		v[i] = 0;
		v[far_index] = 0;
	}
	for (int j = 0; j < height; j++)
	{
		int far_index = j*width + (width - 1);
		u[j*width] = 0;
		u[far_index] = 0;
		v[j*width] = 0;
		v[far_index] = 0;
	}
}

void FluidGrid::applyHeatForce()
{
	for (int i = 0; i < width*height; i++)
	{
		v[i] -= ink[i] * heat_force;
	}
}

void FluidGrid::timeStep()
{
	advect();
	diffuse();
	applyPressure();
	clearBorders();
	applyHeatForce();
}

void FluidGrid::draw(sf::RenderWindow* window, float scale)
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			//float magnitude = min(255, ink[getIndex(i, j)] * 255);
			//float magnitude = min(255, pow(u[getIndex(i, j)], 2) + pow(v[getIndex(i, j)], 2));
			//image.setPixel(i, j, sf::Color(magnitude, magnitude, magnitude));
			image.setPixel(i, j, smoothSample(ink_palette, ink[getIndex(i, j)]));
		}
	}
	texture.update(image);

	sprite.setScale(scale, scale);
	window->draw(sprite);
}

void FluidGrid::draw(sf::RenderWindow* window)
{
	draw(window, 1);
}

void FluidGrid::paintOnArray(float* arr, float x, float y, float radius, float value)
{
	for (int i = (int)max(0, x - radius); i <= min(width - 1, x + radius); i++)
	{
		for (int j = (int)max(0, y - radius); j <= min(height - 1, y + radius); j++)
		{
			if (pow(i - x, 2) + pow(j - y, 2) <= radius*radius) {
				arr[getIndex(i, j)] = value;
			}
		}
	}
}

void FluidGrid::paint(float x, float y, float radius)
{
	paintOnArray(ink, x, y, radius, 1);
}

void FluidGrid::erase(float x, float y, float radius)
{
	paintOnArray(ink, x, y, radius, 0);
}

void FluidGrid::paintVelocity(float x, float y, float radius, float vx, float vy)
{
	paintOnArray(u, x, y, radius, vx);
	paintOnArray(v, x, y, radius, vy);
}
