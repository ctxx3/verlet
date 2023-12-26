#include <SFML/Graphics.hpp>
#include <mutex>
#include <iostream>
using namespace sf;

class Ball
{
public:
	CircleShape ball;
	Vector2f acceleration;
	Vector2f velocity;

	Vector2f position_current;
	Vector2f position_old;
	float maxSpeed = 4.0f;

	float radius = 5;

	Ball(float x, float y, float r, Color col)
	{
		radius = r;
		ball = CircleShape(r, 6);
		ball.setFillColor(col);
		ball.move(x, y);
		position_current = Vector2f(x, y);
		position_old = Vector2f(x, y);
		velocity = Vector2f(0, 0);
		acceleration = Vector2f(0, 0);
	}

	void applyForce(Vector2f force)
	{
		acceleration.x += force.x;
		acceleration.y += force.y;
	}

	void update(float dt)
	{
		velocity.x = position_current.x - position_old.x;
		velocity.y = position_current.y - position_old.y;
		position_old.x = position_current.x;
		position_old.y = position_current.y;

		limitVelocity();
		position_current.x = position_current.x + velocity.x + acceleration.x * dt * dt;
		position_current.y = position_current.y + velocity.y + acceleration.y * dt * dt;

		acceleration.x = 0;
		acceleration.y = 0;

		ball.setPosition(Vector2f(position_current.x - radius, position_current.y - radius));
	}

	void constraint()
	{
		const Vector2f position = Vector2f(240, 240);
		const float rradius = 240.0f;
		const Vector2f to_obj = Vector2f(position_current.x - position.x, position_current.y - position.y);
		const float dist = sqrt((to_obj.x * to_obj.x) + (to_obj.y * to_obj.y));

		if (dist > rradius - radius)
		{
			position_current.x = position.x + ((to_obj.x / dist) * (rradius - radius));
			position_current.y = position.y + ((to_obj.y / dist) * (rradius - radius));
		}
	}

	void wallConstraint(int width, int height)
	{
		if (position_current.x - radius < 0)
			position_current.x = radius;
		if (position_current.y - radius < 0)
			position_current.y = radius;
		if (position_current.x + radius > width)
			position_current.x = width - radius;
		if (position_current.y + radius > height)
			position_current.y = height - radius;
	}
	void limitVelocity()
	{
		if (velocity.x > maxSpeed)
		{
			velocity.x = maxSpeed;
		}
		else if (velocity.x < -maxSpeed)
		{
			velocity.x = -maxSpeed;
		}

		if (velocity.y > maxSpeed)
		{
			velocity.y = maxSpeed;
		}
		else if (velocity.y < -maxSpeed)
		{
			velocity.y = -maxSpeed;
		}
	}

	void attractor(int x, int y, float force)
	{
		Vector2f diff = Vector2f(x - position_current.x, y - position_current.y);
		float mag = sqrt(diff.x * diff.x + diff.y * diff.y);
		applyForce({(diff.x / mag) * force, (diff.y / mag) * force});
	}

	void repeller(int x, int y, float force)
	{
		Vector2f diff = Vector2f(position_current.x - x, position_current.y - y);
		float mag = sqrt(diff.x * diff.x + diff.y * diff.y);
		applyForce({(diff.x / mag) * force, (diff.y / mag) * force});
	}
};