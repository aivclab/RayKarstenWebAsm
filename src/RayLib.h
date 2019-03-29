
#include "glm/glm.hpp"

#ifndef RAY_LIB_H
#define RAY_LIB_H
 
const int IMG_WIDTH = 512;
const int IMG_HEIGHT = 512;

#define MAP_WIDTH 72
#define MAP_HEIGHT 32
#define NUM_LIGHTS 3

const float degToRad = 3.14159265359f / 180.0f;
const float radToDeg = 180.0f / 3.14159265359f;

const float WALL_HEIGHT = 4.0f;
const unsigned int MAP_EMPTY = 0u;
const unsigned int MAP_WALL = 1u;
const unsigned int MAP_DOOR = 2u;
const unsigned int MAP_LIGHT = 3u;

const unsigned int WALL_NORM_POS_X = 0;
const unsigned int WALL_NORM_NEG_X = 1;
const unsigned int WALL_NORM_POS_Y = 2;
const unsigned int WALL_NORM_NEG_Y = 3;

struct HitRecord
{
	float dist{ 0.0f };
	float dirX{ 0.0f };
	float dirY{ 0.0f };
	float cosDir{ 1.0f };
	unsigned int mapValue{ MAP_EMPTY };
	unsigned int wallNormal{ WALL_NORM_POS_X };
	int mapX{ 0 };
	int mapY{ 0 };
};

void rayCastImage(float x, float y, float dirX, float dirY, float fov);
int getMapType(float x, float y);
unsigned char *getImagePtr();
unsigned int getImageSize();

bool shadowRayCast(const glm::vec2 p, const glm::vec2 lightSrc);
HitRecord rayCastMap(const glm::vec2 p, const glm::vec2 dir);
void createMap();
unsigned int *getFloorMapPtr();
HitRecord *getHitRecords();


#endif // RAY_LIB_H


