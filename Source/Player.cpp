
#include "Globals.h"
#include "Entity.h"
#include "Module.h"
#include "Player.h"
#include "box2d/b2_math.h"
#include <algorithm>


Player::Player(Application* parent) : Entity(parent)
{
	speed = 0.f;
}

void Player::SetParameters(ModulePhysics* physics, Texture2D txt) {
	texture = txt;
	body = physics->CreateRectangle(0, 0, SPRITE_WIDTH * SCALE, SPRITE_HEIGHT * SCALE, b2_dynamicBody);
	x = SCREEN_WIDTH / 2;
	y = SCREEN_HEIGHT / 2;

	body->body->SetTransform({ PIXEL_TO_METERS(x), PIXEL_TO_METERS(y) }, body->body->GetTransform().q.GetAngle());
	body->listenerptr = this;
}

update_status Player::Update() {

	update_status ret = UPDATE_CONTINUE;

	currentSpeed = body->ScalarLinearVelocity();
	TraceLog(LOG_INFO, "speed = %.10f", currentSpeed);
	TraceLog(LOG_INFO, "angle: %f", GetBodyAngle());

	static b2Vec2 velocity = b2Vec2(0, 0);
	if (abs(currentSpeed) >= MinSpeed) {
		isSpinning = false;
		if (IsKeyDown(KEY_RIGHT) && GetBodyAngle() < MaxAngle) {
			if (isSpinningRight) TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
			isSpinningRight = true;
			isSpinning = true;
			TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
		}

		if (IsKeyDown(KEY_LEFT) && GetBodyAngle() > -MaxAngle) {
			if (!isSpinningRight) TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
			isSpinningRight = false;
			isSpinning = true;
			TurnBody(forward, isSpinningRight, torqueSpeed, currentSpeed);
		}

		if (!isSpinning) body->ResetAngularVelocity();
	}
	else body->ResetAngularVelocity();

	speed = 0;
	if (IsKeyDown(KEY_UP)) {
		Stopped = false;
		forward = true;
		if ((int)currentSpeed * 10000 == 0) {
			TraceLog(LOG_INFO, "FASTER");
			if (currentSpeed <= MaxSpeed) speed -= forceIncrement * 2;
		}
		else if (currentSpeed <= MaxSpeed) speed -= forceIncrement;
	}
	else if (IsKeyDown(KEY_DOWN)) {
		Stopped = false;
		forward = false;
		if ((int)currentSpeed * 10000 == 0) {
			TraceLog(LOG_INFO, "FASTER");
			if (currentSpeed <= MaxSpeed) speed += forceIncrement * 2;
		}
		else if (currentSpeed <= MaxSpeed) speed += forceIncrement;
	}


	if (IsKeyUp(KEY_UP) && IsKeyUp(KEY_DOWN)) {
		if (!Stopped) {
			if (currentSpeed <= MinSpeed) {
				Stopped = true;
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
	Rectangle source = { 0.0f , 0.0f, (float)texture.width, (float)texture.height };
	Rectangle dest = { x , y , (float)texture.width * SCALE , (float)texture.height * SCALE };
	Vector2 origin = { ((float)texture.width / (2.0f)) * SCALE, ((float)texture.height / (2.0f)) * SCALE };
	float rotation = body->GetRotation() * RAD2DEG;
	DrawTexturePro(texture, source, dest, origin, rotation, WHITE);


	return ret;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::WALL:
		TraceLog(LOG_INFO, "COLLISION");
		Stopped = true;
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::WALL:
		break;
	default:
		break;
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

float Entity::GetBodyAngle() const{
	return body->GetAngleRotation();
}

void Entity::TurnBody(bool isGoingForward, bool isGoingRight, float torque, float speed) const {
	float FinalTorque = 0.f;
	if (isGoingRight) {
		if (isGoingForward) FinalTorque = torque / abs(speed);
		else FinalTorque = -torque / abs(speed);
	} else {
		if (isGoingForward) FinalTorque = -torque / abs(speed);
		else FinalTorque = torque / abs(speed);
	}

	body->TurnWithTorque(FinalTorque);
}