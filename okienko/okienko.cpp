#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <random>
#include "kulka.cpp"
#include <future>
#include <iostream>
#include <fstream>

using namespace sf;

const int steps = 9;
const int num_balls = 6000;
Color colors[6000];
const Vector2u w_size = { 640,640 };
std::vector<kulka> circles;
std::vector<std::vector<std::vector<kulka*>>> grid;
int frames = 0;

//std::vector <std::mutex> mm(num_balls+1);

float size = 3;
int grid_size = 1 + size * 2;
int numCellsX;
int numCellsY;
int threadX;
int threadY;
int tX = 4;
int tY = 4;
std::vector<std::future<void>> futures(tX*tY);
Uint16 grid_lookup[800];

void collide(kulka* a, kulka* b) {
	float collision_axis_x = (*a).position_current.x - (*b).position_current.x;
	float collision_axis_y = (*a).position_current.y - (*b).position_current.y;
	float dist = sqrt((collision_axis_x * collision_axis_x) + (collision_axis_y * collision_axis_y));
	float main_dist = (*a).radius + (*b).radius;
	if (dist < main_dist) {
		Vector2f n = Vector2f(collision_axis_x / dist, collision_axis_y / dist);
		float delta = main_dist - dist;
		(*a).position_current.x += 0.5f * delta * n.x;
		(*a).position_current.y += 0.5f * delta * n.y;
		(*b).position_current.x -= 0.5f * delta * n.x;
		(*b).position_current.y -= 0.5f * delta * n.y;
	}
}

void worker(int startX, int startY, int endX, int endY)
{
	for (int i = startX; i < endX; ++i) {
		for (int j = startY; j < endY; ++j) {
			for (auto& obj : grid[i][j]) {
				for (int ii = std::max(i - 1, 0); ii <= std::min(i + 1, numCellsX - 1); ++ii) {
					for (int jj = std::max(j - 1, 0); jj <= std::min(j + 1, numCellsY - 1); ++jj) {
						for (auto& other : grid[ii][jj]) {
							if (obj == other) {
								continue;
							}
							collide(obj, other);
						}
					}
				}
				obj->applyForce(Vector2f(0, 1000));
				obj->floor(w_size.x,w_size.y);
			}
		}
	}
}

void updatePhysics(float dt) {
	for (int k = 0; k < steps; k++)
	{
		//clear grid
		for (int x = 0; x < grid.size(); x++)
		{
			for (int y = 0; y < grid[x].size(); y++)
			{
				grid[x][y].clear();
			}
		}
		// Populate grid
		for (auto& ball : circles){
			Uint16 posX = (Uint16)ball.position_current.x;
			Uint16 posY = (Uint16)ball.position_current.y;
			if(posX < w_size.x && posY < w_size.y)
				grid[grid_lookup[posX]][grid_lookup[posY]].push_back(&ball);
			else
				grid[0][0].push_back(&ball);
		}
		//callculate collisions and constraints async
		int futureidx = 0;
		for (int x = 0; x < tX; x++)
		{
			for (int y = 0; y < tY; y++) {
				futures[futureidx++] = std::async(std::launch::async,worker, threadX * x, threadY * y, threadX * (x+1), threadY * (y+1));
				//worker(threadX * x, threadY * y, threadX * (x + 1), threadY * (y + 1));
			}
		}
		////wait for futures to finish
		for (auto& function : futures)
		{
			function.wait();
		}
		//update physics
		for (int i = 0; i < circles.size(); i++)
		{
			circles[i].update((dt / steps));
		}
	}
}

sf::Color hsv(int hue, float sat, float val)
{
	hue %= 360;
	while (hue < 0) hue += 360;

	if (sat < 0.f) sat = 0.f;
	if (sat > 1.f) sat = 1.f;

	if (val < 0.f) val = 0.f;
	if (val > 1.f) val = 1.f;

	int h = hue / 60;
	float f = float(hue) / 60 - h;
	float p = val * (1.f - sat);
	float q = val * (1.f - sat * f);
	float t = val * (1.f - sat * (1 - f));

	switch (h)
	{
	default:
	case 0:
	case 6: return sf::Color(val * 255, t * 255, p * 255);
	case 1: return sf::Color(q * 255, val * 255, p * 255);
	case 2: return sf::Color(p * 255, val * 255, t * 255);
	case 3: return sf::Color(p * 255, q * 255, val * 255);
	case 4: return sf::Color(t * 255, p * 255, val * 255);
	case 5: return sf::Color(val * 255, p * 255, q * 255);
	}
}

Uint8 pixels[640 * 640 * 4];
Texture xd;
Sprite buffer(xd);


int main()
{
	//srand(time(NULL));
	ContextSettings settings;
	settings.antialiasingLevel = 4;

	RenderWindow window(VideoMode(w_size.x, w_size.y), "Balls", Style::Close, settings);
	window.setFramerateLimit(144);
	Vector2i n_size = Vector2i(std::ceil(w_size.x / grid_size),w_size.y / grid_size);
	Clock clock;
	float dt;

	numCellsX = std::ceil(w_size.x / grid_size)+1;
	numCellsY = std::ceil(w_size.y / grid_size)+1;
	threadX = numCellsX / tX;
	threadY = numCellsY / tY;
	std::vector<std::vector<std::vector<kulka*>>> list(numCellsX, std::vector<std::vector<kulka*>>(numCellsY));
	grid.swap(list);

	for (int i = 0; i < 640; i++)
	{
		grid_lookup[i] = std::floor(i / grid_size);
	}

	//std::string str;

	//std::ifstream MyReadFile("xd2.txt");
	//std::istringstream iss();
	//int idx = 0;
	//while (getline(MyReadFile, str)) {
	//	int start = 0;
	//	int end = str.find(" ");
	//	int vals[3];
	//	for (int i = 0; i < 2; i++) {
	//		vals[i] = std::stoi(str.substr(start, end - start));
	//		start = end + 1;
	//		end = str.find(" ", start);
	//	}
	//	vals[2] = std::stoi(str.substr(start, end - start));
	//	colors[idx++] = Color(vals[0], vals[1], vals[2]);
	//}

	xd.create(640, 640);

	while (window.isOpen())
	{
		dt = 0.007f;
		//dt = 0.016f;
		//float dt = clock.restart().asMicroseconds() / 1000000.0f;
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				//running = false;
				//for (int i = 0; i < threads.size(); i++) {
				//    threads[i].join();
				//}
				window.close();
			}
			else if (event.type == Event::MouseButtonPressed) {
				dt += 0.06;

			}
			else if (event.type == Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::N) {
					std::ofstream plik("xd.txt");
					for (int i = 0; i < circles.size(); i++)
					{
						plik << circles[i].position_current.x << " " << circles[i].position_current.y << "\n";
					}
					plik.close();
				}
			}

		}
		window.clear(Color(18, 18, 18));

		if (circles.size() < num_balls) {
			for (int i = 0; i < 20; i++)
			{
				kulka temp = kulka(10,80 + i*6, size, hsv(circles.size() / 20, .85, 1));
				//kulka temp = kulka(10, 80 + i * 6, size, colors[circles.size()]);
				temp.applyForce({ 1500000,500000 });
				circles.push_back(temp);
			}
		}
		updatePhysics(dt);

		//window.draw(kulko);
		for (auto& ball : circles)
		{
			window.draw(ball.ball);
		}
		window.draw(buffer);

		window.display();
		frames++;
	}

	return 0;
}