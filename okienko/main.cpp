#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <random>
#include "VerletEngine.cpp"
#include <future>
#include <iostream>
#include <fstream>

using namespace sf;

Vector2u w_size = {640, 640};
VerletEngine engine(w_size);
const int frameRate = 144;
const float dt = 1.0f / frameRate;

sf::Color hsv(int hue, float sat, float val);
void handleEvents(RenderWindow &window, Event &event);
void updatePhysicsAndDraw(RenderWindow &window);

int main()
{
	ContextSettings settings;
	settings.antialiasingLevel = 4;

	RenderWindow window(VideoMode(w_size.x, w_size.y), "Balls", Style::Default, settings);
	window.setFramerateLimit(144);

	Clock clock;

	while (window.isOpen())
	{
		Event event;
		handleEvents(window, event);
		window.clear(Color(18, 18, 18));
		updatePhysicsAndDraw(window);
		window.display();
	}

	return 0;
}

void handleEvents(RenderWindow &window, Event &event)
{
	while (window.pollEvent(event))
	{
		if (event.type == Event::Closed)
		{
			window.close();
		}
		else if (event.type == Event::MouseButtonPressed)
		{
			for (auto &ball : engine.circles)
			{
				ball->applyForce(Vector2f((rand() % 1000 - 500) * 1000, -(rand() % 1000) * 1000));
				// window.draw(ball->ball);
			}
			// dt += 0.06;
		}
		else if (event.type == Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::N)
			{
				std::ofstream plik("xd.txt");
				for (int i = 0; i < engine.circles.size(); i++)
				{
					plik << engine.circles[i]->position_current.x << " " << engine.circles[i]->position_current.y << "\n";
				}
				plik.close();
			}
		}
		else if (event.type == Event::Resized)
		{
			window.setSize(Vector2u(event.size.width, event.size.height));
			window.setView(View(FloatRect(0, 0, event.size.width, event.size.height)));

			w_size.x = event.size.width;
			w_size.y = event.size.height;
			engine.w_size = w_size;
			engine.updateSize(w_size);
		}
	}
}

void updatePhysicsAndDraw(RenderWindow &window)
{
	if (engine.circles.size() < engine.num_balls)
	{
		for (int i = 0; i < 20; i++)
		{
			engine.circles.push_back(std::unique_ptr<Ball>(new Ball(10, 80 + i * 6, engine.size, hsv(engine.circles.size() / 20, .85, 1))));
			engine.circles.back()->applyForce({1500000, 500000});
		}
	}
	engine.updatePhysics(dt);

	for (auto &ball : engine.circles)
	{
		window.draw(ball->ball);
	}
}

sf::Color hsv(int hue, float sat, float val)
{
	hue %= 360;
	while (hue < 0)
		hue += 360;

	if (sat < 0.f)
		sat = 0.f;
	if (sat > 1.f)
		sat = 1.f;

	if (val < 0.f)
		val = 0.f;
	if (val > 1.f)
		val = 1.f;

	int h = hue / 60;
	float f = float(hue) / 60 - h;
	float p = val * (1.f - sat);
	float q = val * (1.f - sat * f);
	float t = val * (1.f - sat * (1 - f));

	switch (h)
	{
	default:
	case 0:
	case 6:
		return sf::Color(val * 255, t * 255, p * 255);
	case 1:
		return sf::Color(q * 255, val * 255, p * 255);
	case 2:
		return sf::Color(p * 255, val * 255, t * 255);
	case 3:
		return sf::Color(p * 255, q * 255, val * 255);
	case 4:
		return sf::Color(t * 255, p * 255, val * 255);
	case 5:
		return sf::Color(val * 255, p * 255, q * 255);
	}
}