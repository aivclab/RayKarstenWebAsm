

#include "RayLib.h"
#include <cmath>

using namespace glm;

unsigned char image[IMG_WIDTH * IMG_HEIGHT * 4];
unsigned int floorMap[MAP_WIDTH * MAP_HEIGHT];
unsigned int lightMap[MAP_WIDTH * MAP_HEIGHT];
const glm::vec4 LIGHTS[NUM_LIGHTS] = { glm::vec4(3.0f, 1.0f, 3.0f, 32.0f), glm::vec4(31.0f, 0.6f, 10.0f, 8.0f),  glm::vec4(62.5f, 0.5f, 3.5f, 16.0f) };

void createMap()
{
	int mapIndex = 0;

	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			mapIndex = x + y * MAP_WIDTH;

			if (y < 1 || y >= MAP_HEIGHT - 1)
			{
				floorMap[mapIndex] = MAP_WALL;
			}
			else if (x < 1 || x >= MAP_WIDTH - 1)
			{
				floorMap[mapIndex] = MAP_WALL;
			}
		}
	}

	// create wall 1
	for (int y = 0; y < MAP_HEIGHT - 8; y++)
	{
		mapIndex = 13 + y * MAP_WIDTH;
		int mapIndex2 = 14 + y * MAP_WIDTH;
		floorMap[mapIndex] = MAP_WALL;
		floorMap[mapIndex2] = MAP_WALL;
	}


	// create wall 1
	for (int y = 8; y < MAP_HEIGHT; y++)
	{
		mapIndex = MAP_WIDTH - 16 + y * MAP_WIDTH;
		int mapIndex2 = MAP_WIDTH - 15 + y * MAP_WIDTH;
		floorMap[mapIndex] = MAP_WALL;
		floorMap[mapIndex2] = MAP_WALL;
	}

	// create column
	for (int y = MAP_HEIGHT / 2; y < MAP_HEIGHT / 2 + 6; y++)
	{
		for (int x = MAP_WIDTH / 2; x < MAP_WIDTH / 2 + 6; x++)
		{
			mapIndex = x + y * MAP_WIDTH;
			floorMap[mapIndex] = MAP_WALL;
		}
	}

	// create doors
	floorMap[2] = MAP_DOOR;
	floorMap[3] = MAP_DOOR;
	floorMap[(MAP_WIDTH - 4) + (MAP_HEIGHT - 1) * MAP_WIDTH] = MAP_DOOR;
	floorMap[(MAP_WIDTH - 5) + (MAP_HEIGHT - 1) * MAP_WIDTH] = MAP_DOOR;
	

	// update light map
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			mapIndex = x + y * MAP_WIDTH;
			unsigned int lightMapValue = 0;
			unsigned int mask = 1u;

			glm::vec2 pos(static_cast<float>(x), static_cast<float>(y));

			for (int i = 0; i < NUM_LIGHTS; i++)
			{
				glm::vec4 li = LIGHTS[i];
				glm::vec2 lightPos(li.x, li.z);

				if (!shadowRayCast(pos, lightPos))
				{
					lightMapValue += mask;
				}

				mask *= 2u;
			}


			lightMap[mapIndex] = lightMapValue;
		}
	}
}

unsigned int * getFloorMap()
{
	return &floorMap[0];
}

unsigned int * getLightMap()
{
	return &lightMap[0];
}

glm::vec4 getLight(const unsigned int i)
{
	return LIGHTS[i];
}

bool outsideMap(const glm::ivec2 next)
{
	return (next.x < 0 || next.x > MAP_WIDTH || next.y < 0 || next.y > MAP_HEIGHT);
}

int getMapValue(const glm::ivec2 p)
{
	if (p.x + p.y * MAP_WIDTH >= MAP_WIDTH*MAP_HEIGHT) return MAP_EMPTY;

	return floorMap[p.x + p.y * MAP_WIDTH];
}

glm::vec2 intersectCircle(const glm::vec2 p, const glm::vec2 d, const glm::vec2 center, const float radius)
{
	const glm::vec2 pLoc = p - center;
	const float A = 1.0f;
	const float B = 2.0f * glm::dot(pLoc, d);
	const float C = glm::dot(pLoc, pLoc) - radius*radius;
	const float discr = (B*B) - (4.0f * A * C);
	if (discr < 0.0f) return glm::vec2(-1.0f);
	glm::vec2 sol((-B + std::sqrtf(discr)) / 2 * A, (-B - std::sqrtf(discr)) / 2 * A);
	return sol;
}

HitRecord rayCastMap(const glm::vec2 p, const glm::vec2 dir)
{
	HitRecord hitc;
	glm::ivec2 iNext;
	glm::ivec2 iStep;

	glm::vec2 deltaInv;
	deltaInv.x = (std::abs(dir.x) < 0.00001f) ? 1000.0f : 1.0f / dir.x;
	deltaInv.y = (std::abs(dir.y) < 0.00001f) ? 1000.0f : 1.0f / dir.y;

	iStep.x = (dir.x < 0.0f) ? -1 : 1;
	iStep.y = (dir.y < 0.0f) ? -1 : 1;

	iNext.x = (dir.x < 0.0f) ? static_cast<int>(std::ceil(p.x)) + iStep.x : static_cast<int>(std::floor(p.x)) + iStep.x;
	iNext.y = (dir.y < 0.0f) ? static_cast<int>(std::ceil(p.y)) + iStep.y : static_cast<int>(std::floor(p.y)) + iStep.y;

	float dist = 0.0f;
	do
	{
		const unsigned int mapValue = getMapValue(iNext);

		if (mapValue == MAP_WALL)
		{
			hitc.dist = dist;
			hitc.mapValue = MAP_WALL;
			hitc.mapX = iNext.x;
			hitc.mapY = iNext.y;
			return hitc;
		}

		if (mapValue == MAP_DOOR)
		{
			hitc.dist = dist;
			hitc.mapValue = MAP_DOOR;
			hitc.mapX = iNext.x;
			hitc.mapY = iNext.y;
			return hitc;
		}

		if (mapValue == MAP_LIGHT)
		{
			glm::vec2 intersection = intersectCircle(p, dir, glm::vec2(static_cast<float>(iNext.x) + 0.5f, static_cast<float>(iNext.y) + 0.5f), 0.5f);

			if (intersection.x > 0.0f)
			{
				hitc.light += std::abs(intersection.x - intersection.y);
				hitc.lightDepth = intersection.x;
			}

		}

		float dx = (static_cast<float>(iNext.x) - p.x) * deltaInv.x;
		float dy = (static_cast<float>(iNext.y) - p.y) * deltaInv.y;

		dist = min(dx, dy);

		if (std::abs(dx - dist) < 0.00001f) iNext.x += iStep.x;
		if (std::abs(dy - dist) < 0.00001f) iNext.y += iStep.y;

	} while (!outsideMap(iNext));

	return hitc;
}

bool shadowRayCast(const glm::vec2 p, const glm::vec2 lightSrc)
{
	glm::ivec2 iNext;
	glm::ivec2 iStep;

	glm::vec2 deltaInv;
	glm::vec2 dir = lightSrc - p;
	const float maxLen = glm::length(dir);

	if (maxLen < 0.0001) return false;

	dir = glm::normalize(dir);
	deltaInv.x = (std::abs(dir.x) < 0.00001f) ? 1000.0f : 1.0f / dir.x;
	deltaInv.y = (std::abs(dir.y) < 0.00001f) ? 1000.0f : 1.0f / dir.y;

	iStep.x = (dir.x < 0.0f) ? -1 : 1;
	iStep.y = (dir.y < 0.0f) ? -1 : 1;

	iNext.x = (dir.x < 0.0f) ? static_cast<int>(std::ceil(p.x)) + iStep.x : static_cast<int>(std::floor(p.x)) + iStep.x;
	iNext.y = (dir.y < 0.0f) ? static_cast<int>(std::ceil(p.y)) + iStep.y : static_cast<int>(std::floor(p.y)) + iStep.y;

	float dist = 0.0f;
	do
	{
		int mapValue = getMapValue(iNext);

		if (mapValue == MAP_WALL)
		{
			return true;
		}

		if (mapValue == MAP_DOOR)
		{
			return true;
		}
		
		float dx = (static_cast<float>(iNext.x) - p.x) * deltaInv.x;
		float dy = (static_cast<float>(iNext.y) - p.y) * deltaInv.y;

		dist = min(dx, dy);

		if (std::abs(dx - dist) < 0.00001f) iNext.x += iStep.x;
		if (std::abs(dy - dist) < 0.00001f) iNext.y += iStep.y;

	} while (!outsideMap(iNext) && (dist <= maxLen));

	return false;
}


void fillImage()
{
	for (int x = 0; x < IMG_WIDTH; x++)
	{
		for (int y = 0; y < IMG_HEIGHT; y++)
		{
			int idx = ((x + y * IMG_WIDTH) * 4);
			// set the red and alpha components
			image[idx] = static_cast<unsigned int>(200);
			image[idx + 1] = static_cast<unsigned int>((x * y) % 255);
			image[idx + 2] = static_cast<unsigned int>(12);
			image[idx + 3] = 255;
		}
	}
}

void rayCastImage(const glm::vec2 cameraPos, const glm::vec2 cameraDir, const float fov)
{
	// compute camera values
	float fp = std::tanf(fov * 0.5f);
	glm::vec2 A = cameraPos + cameraDir - glm::vec2(-cameraDir.y, cameraDir.x) * fp;
	glm::vec2 Ap = cameraPos + cameraDir + glm::vec2(-cameraDir.y, cameraDir.x) * fp;
	glm::vec2 B = Ap - A;
	float step = 1.0f / (static_cast<float>(IMG_WIDTH));
	float s = 0.0f;

	for (int x = 0; x < IMG_WIDTH; x++)
	{
		glm::vec2 d = glm::normalize((A + B * s) - cameraPos);
		float cosdir = glm::dot(d, cameraDir);
		HitRecord hitR = rayCastMap(cameraPos, d);
		for (int y = 0; y < IMG_HEIGHT; y++)
		{
			int idx = ((x + y * IMG_WIDTH) * 4);
			// set the red and alpha components
			image[idx] = 255;
			image[idx + 1] = static_cast<unsigned int>(hitR.dist * cosdir);
			image[idx + 2] = 0;
			image[idx + 3] = 255;
		}
	
		s += step;
	}
}

void rayCast(float x, float y, float dirX, float dirY, float fov)
{
	static bool mapCreated = false;
	if (!mapCreated)
	{
		createMap();
		mapCreated = true;
	}

	glm::vec2 dir(dirX, dirY);
	dir = glm::normalize(dir);
	rayCastImage(glm::vec2(x, y), dir, fov);
}

