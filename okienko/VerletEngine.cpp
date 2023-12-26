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
	static constexpr int steps = 8;
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

		grid = std::vector<std::vector<std::vector<Ball*>>>(numCellsX, std::vector<std::vector<Ball*>>(numCellsY));

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


			// calculate collisions and constraints async for even cells
			for (int i = 0; i < numThreads; i++)
			{
				int startCell = i * threadCells;
				int endCell = std::min((i + 1) * threadCells, numCellsX * numCellsY);
				if (startCell % 2 == 0) // if startCell is even
				{
					pool->enqueue(&VerletEngine::worker, this, startCell, endCell);
				}
			}
			// wait for all tasks to finish
			pool->wait();

			// calculate collisions and constraints async for odd cells
			for (int i = 0; i < numThreads; i++)
			{
				int startCell = i * threadCells;
				int endCell = std::min((i + 1) * threadCells, numCellsX * numCellsY);
				if (startCell % 2 != 0) // if startCell is odd
				{
					pool->enqueue(&VerletEngine::worker, this, startCell, endCell);
				}
			}
			// wait for all tasks to finish
			pool->wait();

			for (auto &ballPtr : circles)
			{
				// Physics update
				ballPtr->update((dt / steps));

				// Gravity
				ballPtr->applyForce(Vector2f(0, 1000));

				// Wall collisions
				ballPtr->wallConstraint(w_size.x, w_size.y);
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

	int numCellsX;
	int numCellsY;

	void collide(Ball* a, Ball* b)
	{
		float collision_axis_x = (*a).position_current.x - (*b).position_current.x;
		float collision_axis_y = (*a).position_current.y - (*b).position_current.y;
		float dist_sq = (collision_axis_x * collision_axis_x) + (collision_axis_y * collision_axis_y);
		float main_dist = (*a).radius + (*b).radius;
		if (dist_sq < main_dist * main_dist)
		{
			float dist = sqrt(dist_sq);
			Vector2f n = Vector2f(collision_axis_x / dist, collision_axis_y / dist);
			float delta = main_dist - dist;

			// Move the balls apart
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