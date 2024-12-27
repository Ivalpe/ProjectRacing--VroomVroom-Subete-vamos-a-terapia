#pragma once

#include "Globals.h"
#include <box2d/b2_math.h>
#include "Entity.h"
#include "ModulePhysics.h"
#include "ModuleRender.h"
#include "Map.h"
#include "ModuleTextures.h"
#include "Module.h"
#include "Globals.h"


class Application;
class PhysBody;

class Player : public Entity
{
public:
	Player(Application* parent);

	virtual ~Player(){};

	update_status Update() override;

	void Render();

	void SetParameters(ModulePhysics* physics, Texture2D txt, float rot, int player = 1);

	float speed;

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void CheckSensor(PhysBody* sensor, bool collisionEnd);
	void CheckFinishLine();
	void PrintPosition(std::vector<Entity*> ranking);


private:
	float currentSpeed = 0.f;

	bool isSpinningRight = false;
	bool isSpinning = false;

	bool forward = false;
	bool stopped = true;

	bool finishedLap;

	KeyboardKey TurnRight, TurnLeft, MoveForward, MoveBack;
};