
#include "Globals.h"
#include "Entity.h"
#include "Module.h"
#include "Player.h"
#include "box2d/b2_math.h"
#include <algorithm>
#include <random>
#include "Application.h"
#include "ModuleAudio.h"


Player::Player(Application* parent) : Entity(parent)
{
	speed = 0.f;
}


void Player::SetParameters(ModulePhysics* physics, Texture2D txt, float rot, std::vector<Texture2D> it, int player) {

	item = 0;
	items = it;
	texture = txt;
	body = physics->CreateRectangle(0, 0, SPRITE_WIDTH * SCALE, SPRITE_HEIGHT * SCALE, b2_dynamicBody);
	carType = PLAYER;


	//SFX Load
	turboSFX = App->audio->LoadFx("Assets/Audio/SFX/Mushroom Boost.wav");

	if (player == 1) {
		TurnLeft = KEY_LEFT;
		TurnRight = KEY_RIGHT;
		MoveBack = KEY_DOWN;
		MoveForward = KEY_UP;
		Power = KEY_RIGHT_SHIFT;
		posItem = { 10, (float)SCREEN_HEIGHT - (items[0].height * 2) - 10 };

		body->body->SetTransform({ PIXEL_TO_METERS(x), PIXEL_TO_METERS(y) }, rot);
	}
	else {
		TurnLeft = KEY_A;
		TurnRight = KEY_D;
		MoveBack = KEY_S;
		MoveForward = KEY_W;
		Power = KEY_LEFT_SHIFT;
		posItem = { (float)SCREEN_WIDTH - (items[0].width * 2) - 10, (float)SCREEN_HEIGHT - (items[0].height * 2) - 10 };

		body->body->SetTransform({ PIXEL_TO_METERS(x), PIXEL_TO_METERS(y) }, rot);
	}

	body->ctype = ColliderType::CAR;
	body->listenerptr = this;

}

update_status Player::Update() {

	update_status ret = UPDATE_CONTINUE;

	currentSpeed = body->ScalarLinearVelocity();
	//Turn
	static b2Vec2 velocity = b2Vec2(0, 0);
	if (abs(currentSpeed) >= MinSpeed) {
		isSpinning = false;
		if (IsKeyDown(TurnRight) && GetBodyAngle() < MaxAngle) {
			if (isSpinningRight) TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
			isSpinningRight = true;
			isSpinning = true;
			TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
		}

		if (IsKeyDown(TurnLeft) && GetBodyAngle() > -MaxAngle) {
			if (!isSpinningRight) TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
			isSpinningRight = false;
			isSpinning = true;
			TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
		}

		if (!isSpinning) body->ResetAngularVelocity();
	}
	else body->ResetAngularVelocity();

	//Forward Back
	speed = 0;
	if (IsKeyDown(MoveForward)) {
		stopped = false;
		forward = true;
		if ((int)currentSpeed * 10000 == 0) {
			if (currentSpeed <= MaxSpeed) speed -= forceIncrement * 2;
		}
		else if (currentSpeed <= MaxSpeed) speed -= forceIncrement;
	}
	else if (IsKeyDown(MoveBack)) {
		stopped = false;
		forward = false;
		if ((int)currentSpeed * 10000 == 0) {
			if (currentSpeed <= MaxSpeed) speed += forceIncrement * 2;
		}
		else if (currentSpeed <= MaxSpeed) speed += forceIncrement;
	}

	//Power
	if (hasPower) {
		App->renderer->Draw(items[item], posItem.x, posItem.y);
	}

	if (hasPower && IsKeyPressed(Power) && !powerActive) {
		App->audio->PlayFx(turboSFX);
		powerActive = true;
		hasPower = false;
	}

	if (powerActive) {
		speed -= forceIncrement * 2;
		timerTurbo--;
	}

	if (timerTurbo == 0) {
		powerActive = false;
		timerTurbo = 60;
	}

	//Stop
	if (IsKeyUp(MoveForward) && IsKeyUp(MoveBack) && !powerActive) {
		if (!stopped) {
			if (currentSpeed <= MinSpeed) {
				stopped = true;
				body->ResetLinearVelocity();
			}
			else {
				if (forward) speed += forceIncrement / 2;
				else speed -= forceIncrement / 2;
			}
		}
		else body->ResetLinearVelocity();
	}

	body->ApplyMovingForce(speed);

	GetPosition(x, y);
	return ret;
}

void Player::Render() {
	GetPosition(x, y);
	Rectangle source = { 0.0f , 0.0f, (float)texture.width, (float)texture.height };
	Rectangle dest = { x + App->renderer->camera.x, y + App->renderer->camera.y, (float)texture.width * SCALE , (float)texture.height * SCALE };
	Vector2 origin = { ((float)texture.width / (2.0f)) * SCALE, ((float)texture.height / (2.0f)) * SCALE };
	float rotation = body->GetRotation() * RAD2DEG;
	DrawTexturePro(texture, source, dest, origin, rotation, WHITE);
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(0, items.size() - 1);
	switch (physB->ctype) {
	case ColliderType::WALL:
		stopped = true;
		break;
	case ColliderType::SENSOR:
		if (!FinishedLaps) CheckSensor(physB);
		break;
	case ColliderType::FINISH_LINE:
		if (!FinishedLaps) CheckSensor(physB);
		if(!FinishedLaps) CheckFinishLine();
		break;
	case ColliderType::ITEM:
		if (!hasPower) {
			hasPower = true;
			item = dist6(rng);
		}
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::WALL:
		break;
	case ColliderType::SENSOR:
		break;
	case ColliderType::FINISH_LINE:
		break;
	default:
		break;
	}
}

void Player::CheckSensor(PhysBody* sensor) {
	int i = 0;
	while (i < sensors.size()) {
		if (sensors[i].id == sensor->id) sensors[i].active = true;
		++i;
	}
}

void Player::CheckFinishLine() {
	finishedLap = true;
	int i = 0;
	while (i < sensors.size()) {
		if (!sensors[i].active) finishedLap = false;
		++i;
	}

	if (finishedLap) {
		Lap++;
		for (auto s : sensors) s.active = false;
		if (Lap >= MaxLaps) FinishedLaps = true;
	}

}

// Entity -----------------------------------------

void Entity::GetPosition(int& x, int& y) const {
	body->GetPhysicPosition(x, y);
}

void Entity::SetPosition(Vector2 pos) {

	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.x), PIXEL_TO_METERS(pos.y));
	body->body->SetTransform(bodyPos, body->GetRotation());

}

float Entity::GetBodyAngle() const {
	return body->GetAngleRotation();

}

void Entity::TurnBody(bool isGoingForward, bool isGoingRight, float torque, float speed) const {
	float FinalTorque = 0.f;
	if (isGoingRight) {
		if (isGoingForward) FinalTorque = torque / abs(speed);
		else FinalTorque = -torque / abs(speed);
	}
	else {
		if (isGoingForward) FinalTorque = -torque / abs(speed);
		else FinalTorque = torque / abs(speed);
	}

	body->TurnWithTorque(FinalTorque);
}