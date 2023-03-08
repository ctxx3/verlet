#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <random>
#include <thread>
#include <iostream>
#include "kulka.cpp"

using namespace sf;

const int steps = 4;

std::vector<kulka> circles;

int frames = 0;
bool running = true;

int size = 3;
int grid_size = 6;

void collide(kulka* a, kulka* b) {
	Vector2f collision_axis = Vector2f(
		(*a).position_current.x - (*b).position_current.x,
		(*a).position_current.y - (*b).position_current.y
	);
	float dist = sqrt((collision_axis.x * collision_axis.x) + (collision_axis.y * collision_axis.y));
	float main_dist = (*a).radius + (*b).radius;
	if (dist < main_dist) {
		Vector2f n = Vector2f(collision_axis.x / dist, collision_axis.y / dist);
		float delta = main_dist - dist;
		(*a).position_current.x += 0.5f * delta * n.x;
		(*a).position_current.y += 0.5f * delta * n.y;
		(*b).position_current.x -= 0.5f * delta * n.x;
		(*b).position_current.y -= 0.5f * delta * n.y;
	}
}



int main()
{
	//srand(time(NULL));
	srand(1);
	ContextSettings settings;
	settings.antialiasingLevel = 4;

	RenderWindow window(VideoMode(640, 640), "Balls", Style::Close, settings);
	window.setFramerateLimit(144);
	Vector2u w_size = window.getSize();
	Vector2i n_size = Vector2i(w_size.x / grid_size,w_size.y / grid_size);
	//std::cout << n_size.x << "  " << n_size.y << "\n";
	Clock clock;
	//std::vector<std::thread> threads;
	//CircleShape kulko = CircleShape(240, 60);
	//kulko.move(0, 0);
	float dt;// = 1 / 144.0f;
	//std::vector<std::vector<int>> grid;
	//grid.resize(n_size.x * n_size.y, std::vector<int>(0, 0));
	//grid.resize(16, std::vector<int>(0, 0));
	//for (int i = 0; i < 16; i++)
	//{
	//    threads.push_back(std::thread(updatePos, (num / 16) * i, num / 16 + ((num / 16) * i)));
	//}


	while (window.isOpen())
	{
		dt = 0.007f;
		int checks = 0;
		//float dt = clock.restart().asMicroseconds() / 1000000.0f;
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				running = false;
				//for (int i = 0; i < threads.size(); i++) {
				//    threads[i].join();
				//}
				window.close();
			}
			else if (event.type == Event::MouseButtonPressed) {
				dt += 0.06;
			}

		}
		window.clear(Color(18, 18, 18));

		if (circles.size() < 2000) {
			for (int i = 0; i < 5; i++)
			{
				kulka wow = kulka(240 + (rand() % 8), 240 + (rand() % 8), size);
				circles.push_back(wow);
			}
		}



		for (int k = 0; k < steps; k++)
		{
			//for (int i = 0; i < grid.size(); i++)
			//{
			//	grid[i].clear();
			//}
			//for (int i = 0; i < circles.size(); i++) {
			//	circles[i].applyForce(Vector2f(0, 1000));
			//	circles[i].floor(w_size.x, w_size.y);
			//	if (!isnan(circles[i].position_current.x)) {
			//		circles[i].grid_idx = circles[i].position_current.x / grid_size;
			//		circles[i].grid_idy = circles[i].position_current.y / grid_size;
			//		//std::cout << circles[i].grid_idx << '\n';
			//		//int id = (yd * n_size.x) + xd;
			//		//std::cout << i << "  " << x << "  " << y << "  " << id << '\n';
			//		//grid[id].push_back(i);
			//	}
			//}

			//for (int i = 0; i < circles.size(); i++)
			//{
			//	for (int j = i + 1; j < circles.size(); j++)
			//	{
			//		int diffx = circles[i].grid_idx - circles[j].grid_idx;
			//		diffx = ((diffx >> 31) | 1) * diffx;
			//		if (diffx == 0 || diffx == 1) {
			//			int diffy = circles[i].grid_idy - circles[j].grid_idy;
			//			diffy = ((diffx >> 31) | 1) * diffx;
			//			//int diffy = abs(circles[i].grid_idy - circles[j].grid_idy);
			//			if (diffy == 0 || diffy == 1) {
			//				collide(&circles[i], &circles[j]);
			//				checks++;

			//			}
			//		}
			//	}
			//}


			//for (int y = 0; y < 4; y++)
			//{
			//	for (int x = 0; x < 4; x++)
			//	{
			//		int current_cell = (y * 4) + x;
			//		for (int dy = -1; dy <= 1; dy++)
			//		{
			//			for (int dx = -1; dx <= 1; dx++)
			//			{
			//				if (y + dy >= 0 && x + dx >= 0 && y + dy < 4 && x + dx < 4) {
			//					int other_cell = ((y + dy) * 4) + (x + dx);
			//					for (int o1 : grid[current_cell]) {
			//						for (int o2 : grid[other_cell])
			//						{
			//							if (o1 != o2) {
			//								//std::cout << "kolizja!\n";
			//								collide(&circles[o1], &circles[o2]);
			//							}
			//						}
			//					}
			//				}

			//			}
			//		}
			//	}
			//}
			// 
			//for (int y = 0; y < n_size.y; y++)
			//{
			//	for (int x = 0; x < n_size.x; x++)
			//	{
			//		int current_cell = (y * n_size.x) + x;
			//		for (int dy = -1; dy <= 1; dy++)
			//		{
			//			for (int dx = -1; dx <= 1; dx++)
			//			{
			//				if (y + dy >= 0 && x + dx >= 0 && y + dy < n_size.y && x + dx < n_size.x) {
			//					int other_cell = ((y + dy) * n_size.x) + (x + dx);
			//					for (int o1 : grid[current_cell]) {
			//						for (int o2 : grid[other_cell])
			//						{
			//							if (o1 != o2) {
			//								//std::cout << "kolizja!\n";
			//								collide(&circles[o1], &circles[o2]);
			//							}
			//						}
			//					}
			//				}

			//			}
			//		}
			//	}
			//}
			for (int i = 0; i < circles.size(); i++)
			{
				circles[i].applyForce(Vector2f(0, 1000));
				circles[i].floor(w_size.x, w_size.y);
			}

			for (int i = 0; i < circles.size(); i++)
			{
				for (int j = i+1; j < circles.size(); j++)
				{
					collide(&circles[i], &circles[j]);
					checks++;
				}
			}

			for (int i = 0; i < circles.size(); i++)
			{
				circles[i].update((dt / steps));
			}
		}
		//window.draw(kulko);
		for (int i = 0; i < circles.size(); i++)
		{
			window.draw(circles[i].ball);
		}
		std::cout << checks << '\n';
		window.display();
		frames++;
	}

	return 0;
}

//void updatePos(int start, int end) {
//    int last_frame = -1;
//    while (running) {
//        if (frames != last_frame) {
//            for (int i = start; i < end; i++)
//            {
//                circles[i].ball.move(x_off[i], y_off[i]);
//                Vector2f pos = circles[i].ball.getPosition();
//
//                if (pos.x <= 0 || pos.x + 10 >= 860) {
//                    x_off[i] *= -1;
//                }
//                if (pos.y <= 0 || pos.y + 10 >= 860) {
//                    y_off[i] *= -1;
//                }
//            }
//            last_frame = frames;
//        }
//        else {
//            auto ms = std::chrono::steady_clock::now() + std::chrono::milliseconds(5);
//            std::this_thread::sleep_until(ms);
//        }
//    }
//
//}