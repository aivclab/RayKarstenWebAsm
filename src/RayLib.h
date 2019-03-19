
#include "glm/glm.hpp"

#ifndef RAY_LIB_H
#define RAY_LIB_H
 
const int IMG_WIDTH = 800;
const int IMG_HEIGHT = 600;

#define MAP_WIDTH 64
#define MAP_HEIGHT 24
#define NUM_LIGHTS 3
const float WALL_HEIGHT = 3.5f;

const unsigned int MAP_EMPTY = 0u;
const unsigned int MAP_WALL = 1u;
const unsigned int MAP_DOOR = 2u;
const unsigned int MAP_LIGHT = 3u;


struct HitRecord
{
	float dist{ 0.0f };
	float light{ 0.0f };
	float lightDepth{ 0.0f };
	float dirX{ 0.0f };
	float dirY{ 0.0f };
	int mapValue{ MAP_EMPTY };
	int mapX{ 0 };
	int mapY{ 0 };
};

void rayCast(float x, float y, float dirX, float dirY, float fov);
bool shadowRayCast(const glm::vec2 p, const glm::vec2 lightSrc);
HitRecord rayCastMap(const glm::vec2 p, const glm::vec2 dir);
void createMap();
unsigned int *getFloorMap();
unsigned int *getLightMap();
glm::vec4 getLight(const unsigned int i);

#endif // RAY_LIB_H


