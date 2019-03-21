#include <SFML/Graphics.hpp>
#include "FluidGrid.h"

int main()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(480, 640), "sample text", sf::Style::Default, settings);

	float deRes = 3;

	sf::Vector2u window_size = window.getSize();
	FluidGrid fg(ceil(window_size.x / deRes), ceil(window_size.y / deRes));
	fg.setViscosity(.01);
	fg.setSpeed(1);
	fg.setIterations(40);
	fg.setHeatForce(.2);
	sf::Image ink_palette;
	ink_palette.loadFromFile("fire-palette.png");
	fg.usePalette(&ink_palette);

	sf::Vector2i past_mouse_pos;

	while (window.isOpen())
	{
		
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			fg.paint(
				mouse_pos.x / deRes,
				mouse_pos.y / deRes,
				10);
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			fg.paintVelocity(
				mouse_pos.x / deRes,
				mouse_pos.y / deRes,
				10,
				mouse_pos.x - past_mouse_pos.x,
				mouse_pos.y - past_mouse_pos.y);
		}
		//window.clear(sf::Color::Black);
		
		
		fg.timeStep();
		fg.draw(&window, deRes);

		// test
		/*
		sf::Image image;
		image.create(100, 100);
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++) {
				image.setPixel(i, j, sf::Color(i,j,0));
			}
		}
		sf::Sprite sprite;
		sf::Texture texture;
		texture.loadFromImage(image);
		sprite.setTexture(texture);
		window.draw(sprite);
		*/

		window.display();

		past_mouse_pos = mouse_pos;
	}

	return 0;
}