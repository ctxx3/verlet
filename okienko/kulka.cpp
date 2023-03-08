#include <SFML/Graphics.hpp>
#include <iostream>
using namespace sf;

class kulka {
public:
	CircleShape ball;
	Vector2f acceleration;
	Vector2f velocity;

	Vector2f position_current;
	Vector2f position_old;

	float radius = 5;
	int grid_idx = 0;
	int grid_idy = 0;

	kulka(float x, float y, float r) {
		radius = r;
		ball = CircleShape(r, 8);
		ball.setFillColor(Color(rand() % 255, rand() % 255, rand() % 255));
		ball.move(x, y);
		position_current = Vector2f(x, y);
		position_old = Vector2f(x, y);
		velocity = Vector2f(0, 0);
		acceleration = Vector2f(0, 0);
	}

	void applyForce(Vector2f force) {
		acceleration.x += force.x;
		acceleration.y += force.y;
	}

	void update(float dt) {
		const Vector2f velocity = Vector2f(position_current.x - position_old.x, position_current.y - position_old.y);
		position_old.x = position_current.x;
		position_old.y = position_current.y;

		position_current.x = position_current.x + velocity.x + acceleration.x * dt * dt;
		position_current.y = position_current.y + velocity.y + acceleration.y * dt * dt;

		acceleration.x = 0;
		acceleration.y = 0;

		ball.setPosition(Vector2f(position_current.x - radius, position_current.y - radius));
	}
	void constraint() {
		const Vector2f position = Vector2f(240, 240);
		const float rradius = 240.0f;
		const Vector2f to_obj = Vector2f(position_current.x - position.x, position_current.y - position.y);
		const float dist = sqrt((to_obj.x * to_obj.x) + (to_obj.y * to_obj.y));

		if (dist > rradius - radius) {
			position_current.x = position.x + ((to_obj.x / dist) * (rradius - radius));
			position_current.y = position.y + ((to_obj.y / dist) * (rradius - radius));
		}
	}
	void floor(int width, int height) {
		if (position_current.x < radius) {
			position_current.x = radius;
		}
		else if (position_current.x > width - radius) {
			position_current.x = width - radius;
		}
		if (position_current.y < radius) {
			position_current.y = radius;
		}
		else if (position_current.y > height - radius) {
			position_current.y = height - radius;
		}
	}
	void attractor(int x, int y, float force) {
		Vector2f diff = Vector2f(x - position_current.x, y - position_current.y);
		float mag = sqrt(diff.x * diff.x + diff.y * diff.y);
		applyForce({ (diff.x / mag) * force, (diff.y / mag) * force });
	}
	void repeller(int x, int y, float force) {
		Vector2f diff = Vector2f(position_current.x - x, position_current.y - y);
		float mag = sqrt(diff.x * diff.x + diff.y * diff.y);
		applyForce({ (diff.x / mag) * force, (diff.y / mag) * force });
	}
};

struct cell {
	std::vector<int> ids;
};


class zbior {
public:
	std::vector<cell> cells;
	int width;
	int height;
	int rozmiar;
	zbior(int width, int height) {
		//rozmiar = to_one_dim(width,height);
		rozmiar = width * height + 1;
		for (int i = 0; i < rozmiar; i++)
		{
			cells.push_back(cell());
		}
	}
	void clear() {
		for (int i = 0; i < rozmiar; i++)
		{
			cells[i].ids.clear();
		}
	}
	void add(int id, int x, int y) {
		cells[to_one_dim(x, y)].ids.push_back(id);
	}
	cell* get(int x, int y) {
		return &cells[to_one_dim(x, y)];
	}

	int to_one_dim(int x, int y) {
		return y * width + x;
	}
};