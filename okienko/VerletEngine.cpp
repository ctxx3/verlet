#include <SFML/Graphics.hpp>
#include "BallClass.cpp"
#include <future>
#include <array>
#include <memory>
#include <iostream>
#include "ThreadPool.cpp"

class VerletEngine
{
public:
	std::vector<std::unique_ptr<Ball>> circles;
	std::unique_ptr<ThreadPool> pool;
	Vector2u w_size;
	static constexpr int num_balls = 12000;
	static constexpr int steps = 9;
	static constexpr float size = 2;

	VerletEngine(Vector2u window_size)
	{
		w_size = window_size;

		numThreads = std::thread::hardware_concurrency(); // Number of threads = number of CPU cores
		grid_size = 5;
		updateSize(w_size);
		pool = std::make_unique<ThreadPool>(numThreads);
	}
	void updateSize(Vector2u newSize)
	{
		w_size = newSize;
		numCellsX = std::ceil(w_size.x / grid_size) + 1;
		numCellsY = std::ceil(w_size.y / grid_size) + 1;

		threadCells = numCellsX * numCellsY / numThreads;

		std::vector<std::vector<std::vector<Ball *>>> list(numCellsX, std::vector<std::vector<Ball *>>(numCellsY));
		grid.swap(list);

		int maxDimension = std::max(w_size.x, w_size.y);
		grid_lookup.resize(maxDimension);
		for (int i = 0; i < maxDimension; i++)
		{
			grid_lookup[i] = std::floor(i / grid_size);
		}
	}

	void updatePhysics(float dt)
	{

		for (int k = 0; k < steps; k++)
		{
			// clear grid
			for (int x = 0; x < grid.size(); x++)
			{
				for (int y = 0; y < grid[x].size(); y++)
				{
					grid[x][y].clear();
				}
			}
			// Populate grid
			for (auto &ballPtr : circles)
			{
				Uint16 posX = (Uint16)ballPtr->position_current.x;
				Uint16 posY = (Uint16)ballPtr->position_current.y;
				if (posX < w_size.x && posY < w_size.y)
					grid[grid_lookup[posX]][grid_lookup[posY]].push_back(ballPtr.get());
				else
					grid[0][0].push_back(ballPtr.get());
			}
			// callculate collisions and constraints async
			for (int i = 0; i < numThreads; i++)
			{
				int startCell = i * threadCells;
				int endCell = (i + 1) * threadCells;
				pool->enqueue(&VerletEngine::worker, this, startCell, endCell);
			}
			// update physics
			for (auto &ballPtr : circles)
			{
				ballPtr->update((dt / steps));

				ballPtr->applyForce(Vector2f(0, 1000));
				if (ballPtr->position_current.x - ballPtr->radius < 0)
					ballPtr->position_current.x = ballPtr->radius;
				if (ballPtr->position_current.y - ballPtr->radius < 0)
					ballPtr->position_current.y = ballPtr->radius;
				if (ballPtr->position_current.x + ballPtr->radius > w_size.x)
					ballPtr->position_current.x = w_size.x - ballPtr->radius;
				if (ballPtr->position_current.y + ballPtr->radius > w_size.y)
					ballPtr->position_current.y = w_size.y - ballPtr->radius;
			}
		}
	}

private:
	int grid_size = 1 + size * 2;
	int numThreads;
	int threadCells;

	std::array<Color, num_balls> colors;
	std::vector<std::vector<std::vector<Ball *>>> grid;
	std::vector<Uint16> grid_lookup;
	std::vector<std::future<void>> futures;

	int frames = 0;
	int numCellsX;
	int numCellsY;

	void collide(Ball *a, Ball *b)
	{
		float collision_axis_x = (*a).position_current.x - (*b).position_current.x;
		float collision_axis_y = (*a).position_current.y - (*b).position_current.y;
		float dist = sqrt((collision_axis_x * collision_axis_x) + (collision_axis_y * collision_axis_y));
		float main_dist = (*a).radius + (*b).radius;
		if (dist < main_dist)
		{
			Vector2f n = Vector2f(collision_axis_x / dist, collision_axis_y / dist);
			float delta = main_dist - dist;

			(*a).position_current.x += 0.5f * delta * n.x;
			(*a).position_current.y += 0.5f * delta * n.y;

			(*b).position_current.x -= 0.5f * delta * n.x;
			(*b).position_current.y -= 0.5f * delta * n.y;
		}
	}

	void worker(int startCell, int endCell)
	{
		for (int cell = startCell; cell < endCell; ++cell)
		{
			int i = cell / numCellsY;
			int j = cell % numCellsY;
			for (auto &obj : grid[i][j])
			{
				for (int ii = std::max(i - 1, 0); ii <= std::min(i + 1, numCellsX - 1); ++ii)
				{
					for (int jj = std::max(j - 1, 0); jj <= std::min(j + 1, numCellsY - 1); ++jj)
					{
						for (auto &other : grid[ii][jj])
						{
							if (obj != other)
							{
								collide(obj, other);
							}
						}
					}
				}
			}
		}
	}
};