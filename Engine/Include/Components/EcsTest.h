#pragma once

#include <flecs.h>
#include "TransformComponent.h"
#include <iostream>

void Test()
{
	flecs::world world;

	world.component<Transform>("Transform");
	auto entity = world.entity("Player")
		.add<Transform>();

	entity.set<Transform>({ 10.0f, 10.0f, 10.0f });

	world.progress();

	auto transform = entity.get<Transform>();
	std::cout << "Player position: " << transform.x << ", " << transform.y << ", " << transform.z << std::endl;

}