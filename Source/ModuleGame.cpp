#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "Player.h"
#include "Enemy.h"
#include "ModuleWindow.h"
#include "Map.h"
#include <random>

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{}

ModuleGame::~ModuleGame()
{}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	stateGame = MAIN_MENU;
	background_layer = LoadTexture("Assets/background.png");
	background_layer2 = LoadTexture("Assets/background2.png");
	background_layer3 = LoadTexture("Assets/background3.png");
	selectedVehicle = LoadTexture("Assets/selectVehicle.png");
	App->map->Load("Assets/Maps/", "racing.tmx");


	vehicles.push_back(LoadTexture("Assets/car1.png"));
	vehicles.push_back(LoadTexture("Assets/car2.png"));
	vehicles.push_back(LoadTexture("Assets/car3.png"));

	car = new Player(App);
	for (auto i = 0; i < 3; i++) {
		enemyCars.push_back(new Enemy(App));
	}

	selectedPos = 0;

	int width = SPRITE_WIDTH * 3;
	int height = SPRITE_HEIGHT * 3;
	float posX = 100, posY = 100;
	for (int i = 0; i < vehicles.size(); i++) {
		posVehicles.push_back(Vector2{ posX,posY });

		if (i > 0 && i % 10 == 0) {
			posY += 100;
			posX = 100;
		}
		else posX += 100;
	}

	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading scene");
	for (auto c : checkpoints) delete c;
	checkpoints.clear();

	LOG("Unloading Intro scene");
	for (auto car : enemyCars) {
		car->CleanUp();
	}
	car->CleanUp();
	App->map->CleanUp();


	return true;
}

// Update: draw background
update_status ModuleGame::Update()
{
	switch (stateGame)
	{
	case MAIN_MENU:
		MainMenu();
		break;
	case SELECT_CHARACTER:
		SelectCharacter();
		break;
	case GAME:
		Game();
		break;
	}


	return UPDATE_CONTINUE;
}

void ModuleGame::MainMenu() {
	Rectangle rect;
	rect.x = 0;
	rect.y = 208;
	rect.width = SCREEN_WIDTH;
	rect.height = SCREEN_HEIGHT;
	//App->renderer->Draw(background_layer, 0, 0, &rect);


	if (IsKeyPressed(KEY_SPACE)) {
		stateGame = SELECT_CHARACTER;
	}
}

void ModuleGame::SelectCharacter() {

	if (IsKeyPressed(KEY_RIGHT) && selectedPos < vehicles.size() - 1) {
		selectedPos++;
	}

	if (IsKeyPressed(KEY_LEFT) && selectedPos > 0) {
		selectedPos--;
	}


	Rectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.width = SPRITE_WIDTH * SCALE;
	rect.height = SPRITE_HEIGHT * SCALE;
	for (int i = 0; i < vehicles.size(); i++) App->renderer->Draw(vehicles[i], posVehicles[i].x, posVehicles[i].y, &rect);

	rect.x = 0;
	rect.y = 0;
	rect.width = SPRITE_WIDTH * SCALE;
	rect.height = SPRITE_HEIGHT * SCALE;
	App->renderer->Draw(selectedVehicle, posVehicles[selectedPos].x, posVehicles[selectedPos].y, &rect);
	
	//Random Car
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(0, vehicles.size() - 1);

	Vector2 pos = { 153 * SCALE, 350 * SCALE };
	if (IsKeyPressed(KEY_SPACE)) {
		car->SetParameters(App->physics, vehicles[selectedPos]);
		for (auto car : enemyCars) {

			car->SetParameters(App->physics, vehicles[dist6(rng)]);
			car->SetPosition(pos);
			pos.x += 100;
		}

		stateGame = GAME;
		checkpoints = App->map->GetSensors();
		for (auto c : checkpoints) {
			if (!c->finishLine) {
				CheckpointSensor* s = new CheckpointSensor;

				s->id = c->id;
				s->active = false;
				s->changeable = true;

				car->sensors.push_back(s);
				for (auto e : enemyCars) e->sensors.push_back(s);
			}
		}
	}

}

void ModuleGame::Game() {

	car->Update();
	for (auto car : enemyCars) {
		car->Update();
	}

	//camera
	int carX, carY;
	/*car->GetPosition(carX, carY);
	car->body->GetPhysicPosition(carX, carY);*/
	//carX = METERS_TO_PIXELS(car->body->body->GetTransform().p.x);
	//carY = METERS_TO_PIXELS(car->body->body->GetTransform().p.y);

	//App->renderer->camera.x = ((carX - SCREEN_WIDTH / 2) * -(SCREEN_SIZE));
	//App->renderer->camera.y = ((carY - SCREEN_HEIGHT / 2) * -(SCREEN_SIZE));

	//TraceLog(LOG_INFO, "Postion %d, %d // Camera %f, %f", carX, carY, App->renderer->camera.x, App->renderer->camera.y);

	//just a test to check the received position was correct
	//DrawRectangle(carX - 25, carY - 40, 50, 80, Color({ 0,0,255,255 }));

	//drew the camera outline and yep, it encloses the map
	DrawRectangleLines(App->renderer->camera.x, App->renderer->camera.y, SCREEN_WIDTH, SCREEN_HEIGHT, Color({ 0,0,255,255 }));


}
