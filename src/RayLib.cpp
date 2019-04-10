

#include "RayLib.h"
#include <cmath>

using namespace glm;

unsigned char image[IMG_WIDTH * IMG_HEIGHT * 4];
unsigned int floorMap[MAP_WIDTH * MAP_HEIGHT];
const glm::vec3 fogColor(20.0f, 50.0f, 25.0f);
const float ambientLight = 0.15f;
HitRecord hitImage[IMG_WIDTH];
float lightmap[2][MAP_WIDTH * MAP_HEIGHT];
int lightRead = 0;
int lightWrite = 1;


void swapLightMaps()
{
	int tmp = lightRead;
	lightRead = lightWrite;
	lightWrite = tmp;
}

void initLightMap()
{
	for (uint i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
	{
		lightmap[0][i] = 0.0f;
		lightmap[1][i] = 0.0f;
	}
}

float getLight(int x, int y)
{
	const uint mapIndex = x + y * MAP_WIDTH;
	if (mapIndex >= MAP_WIDTH * MAP_HEIGHT) return 0.0f;
	return lightmap[lightRead][mapIndex];
}

void updateLightMap()
{
	for (uint x = 0; x < MAP_WIDTH; x++)
	{
		for (uint y = 0; y < MAP_HEIGHT; y++)
		{
			const uint mapIndex = x + y * MAP_WIDTH;
			const int mapType = floorMap[mapIndex];
			if (mapType == MAP_WALL)
			{
				lightmap[lightWrite][mapIndex] = 0.0f;
			}
			else if (mapType == MAP_LIGHT)
			{
				lightmap[lightWrite][mapIndex] = 12.0f; //Inject some light
			}
			else
			{
				lightmap[lightWrite][mapIndex] = (getLight(x - 1, y) + getLight(x + 1, y) + getLight(x, y-1) + getLight(x, y+1)) * 0.25f;
			}
		}
	}
	swapLightMaps();
}


// bilinear light 
float getLight(glm::vec2 pk)
{
	glm::vec2 p = pk - glm::vec2(0.5f);
	const int ix = static_cast<int>(std::floor(p.x));
	const int iy = static_cast<int>(std::floor(p.y));
	const float wx = p.x - std::floor(p.x);
	const float wy = p.y - std::floor(p.y);

	return
		getLight(ix, iy) * (1.0f-wx) * (1.0f-wy) +
		getLight(ix, iy + 1) * (1.0f-wx) * (wy) +
		getLight(ix + 1, iy) * (wx) * (1.0f-wy) +
		getLight(ix + 1, iy + 1) * ( wx) * (wy);

}

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
		floorMap[mapIndex] = MAP_WALL;
		mapIndex = 14 + y * MAP_WIDTH;
		floorMap[mapIndex] = MAP_WALL;
	}

	// create wall 2
	for (int y = 8; y < MAP_HEIGHT; y++)
	{
		mapIndex = MAP_WIDTH - 16 + y * MAP_WIDTH;
		floorMap[mapIndex] = MAP_WALL;
		mapIndex = MAP_WIDTH - 17 + y * MAP_WIDTH;
		floorMap[mapIndex] = MAP_WALL;
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

	// create lights
	mapIndex = 5 + 8 * MAP_WIDTH;
	floorMap[mapIndex] = MAP_LIGHT;

	mapIndex = 35 + 8 * MAP_WIDTH;
	floorMap[mapIndex] = MAP_LIGHT;

	mapIndex = 45 + 8 * MAP_WIDTH;
	floorMap[mapIndex] = MAP_LIGHT;

	mapIndex = (MAP_WIDTH - 8) + (MAP_HEIGHT - 5) *MAP_WIDTH;
	floorMap[mapIndex] = MAP_LIGHT;

	mapIndex = (MAP_WIDTH - 8) + (MAP_HEIGHT - 15) *MAP_WIDTH;
	floorMap[mapIndex] = MAP_LIGHT;
}

unsigned int * getFloorMapPtr()
{
	return &floorMap[0];
}

unsigned char * getImagePtr()
{
	return &image[0];
}

HitRecord * getHitRecords()
{
	return &hitImage[0];
}

unsigned int getImageSize()
{
	return IMG_WIDTH *IMG_HEIGHT * 4;
}

bool outsideMap(const glm::ivec2 next)
{
	return (next.x < 0 || next.x > MAP_WIDTH || next.y < 0 || next.y > MAP_HEIGHT);
}

int getMapValue(const glm::ivec2 p)
{
	const int index = p.x + p.y * MAP_WIDTH;
	if (index < 0 || index >= MAP_WIDTH*MAP_HEIGHT) return MAP_EMPTY;
	return floorMap[index];
}

glm::vec2 intersectCircle(const glm::vec2 p, const glm::vec2 d, const glm::vec2 center, const float radius)
{
	const glm::vec2 pLoc = p - center;
	const float A = 1.0f;
	const float B = 2.0f * glm::dot(pLoc, d);
	const float C = glm::dot(pLoc, pLoc) - radius*radius;
	const float discr = (B*B) - (4.0f * A * C);
	if (discr < 0.0f) return glm::vec2(-1.0f);
	glm::vec2 sol((-B + std::sqrt(discr)) / 2 * A, (-B - std::sqrt(discr)) / 2 * A);
	return sol;
}

HitRecord rayCastMap(const glm::vec2 p, const glm::vec2 dir)
{
	HitRecord hitc;
	glm::ivec2 iNext;
	glm::ivec2 iStep;
	glm::vec2 deltaInv;

	deltaInv.x = (std::abs(dir.x) < 0.0000001f) ? 100000.0f : 1.0f / dir.x;
	deltaInv.y = (std::abs(dir.y) < 0.0000001f) ? 100000.0f : 1.0f / dir.y;

	iStep.x = (dir.x < 0.0f) ? -1 : 1;
	iStep.y = (dir.y < 0.0f) ? -1 : 1;

	iNext.x = (dir.x < 0.0f) ? static_cast<int>(std::ceil(p.x)) + iStep.x : static_cast<int>(std::floor(p.x)) + iStep.x;
	iNext.y = (dir.y < 0.0f) ? static_cast<int>(std::ceil(p.y)) + iStep.y : static_cast<int>(std::floor(p.y)) + iStep.y;
	bool firstStep = true;

	int wallNormal = WALL_NORM_NEG_X;
	float dist = 0.0f;
	do
	{
		const float px = p.x + dir.x * dist *1.0001f;
		const float py = p.y + dir.y * dist *1.0001f;
		const unsigned int mapValue = (firstStep) ? MAP_EMPTY : getMapValue(glm::ivec2(px, py));
		firstStep = false;

		if (mapValue == MAP_WALL || mapValue == MAP_DOOR)
		{
			hitc.dist = dist;
			hitc.mapValue = mapValue;
			hitc.wallNormal = wallNormal;
			hitc.mapX = iNext.x;
			hitc.mapY = iNext.y;
			return hitc;
		}

		float dx = (static_cast<float>(iNext.x) - p.x) * deltaInv.x;
		float dy = (static_cast<float>(iNext.y) - p.y) * deltaInv.y;
		dist = glm::min(dx, dy);

		if (std::abs(dx - dist) < 0.00001f)
		{
			iNext.x += iStep.x;
			wallNormal = (iStep.x == -1) ? WALL_NORM_NEG_X : WALL_NORM_POS_X;
		}
		if (std::abs(dy - dist) < 0.00001f)
		{
			iNext.y += iStep.y;
			wallNormal = (iStep.y == -1) ? WALL_NORM_NEG_Y : WALL_NORM_POS_Y;
		}

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

float getFogValue(const float depth, const float fogStart, const float fogEnd)
{
	const float tmp = (depth > fogEnd)? 0.0001f : fogEnd - depth;
	return glm::min(1.0f, tmp / (fogEnd - fogStart));
}

void setPixel(const int x, const int y, const glm::ivec3 color)
{
	int idx = ((x + y * IMG_WIDTH) * 4);
	image[idx] = color.x;
	image[idx + 1] = color.y;
	image[idx + 2] = color.z;
	image[idx + 3] = 255;
}

glm::vec3 getWallNormal(const unsigned int wn)
{
	glm::vec3 norm(0.0f);
	switch (wn)
	{
	case WALL_NORM_POS_X:
		norm = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case WALL_NORM_NEG_X:
		norm = glm::vec3(-1.0f, 0.0f, 0.0f);
		break;
	case WALL_NORM_POS_Y:
		norm = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	case WALL_NORM_NEG_Y:
		norm = glm::vec3(0.0f, 0.0f, -1.0f);
		break;
	}
	return norm;
}


// https://www.shadertoy.com/view/4sfGWX
glm::vec3 wolfensteinGreyBrick(glm::vec2 uv_)
{
    glm::vec2 uv = floor(mod(uv_ + 64.0f, vec2(64.0f)));
	vec2 buv = vec2(mod(uv.x + 1.0f + (floor((uv.y + 1.0f) / 16.0f) * 16.0f), 32.0f), mod(uv.y + 1.0f, 16.0f));
	float bbr = mix(190.0f, 91.0f, (buv.y) / 14.0f);
	if (buv.x < 2.0f || buv.y < 2.0f)
	{
		bbr = 52.0f;
	}
	glm::vec3 col = vec3(bbr*0.95f);
	return col;
}

glm::vec3 getBrickTexture(const float u, const float v)
{
	const int brickRows = 4;
	const float mortar = 0.1f;
	int brickCollums = 3;

	float row = v * static_cast<float>(brickRows);
	float rowOffset = row - std::floor(row);

	if (static_cast<int>(row) % 2 == 0) brickCollums -= 1;

	float col = u * static_cast<float>(brickCollums);
	float colOffset = col - std::floor(col);
	bool insideBrick = (rowOffset > mortar && colOffset > mortar);

	if (insideBrick)
	{
		return glm::vec3(150.0f);
	}
	return glm::vec3(50.0f);
}


void renderImage(const glm::vec3 cameraPos, const float imgFocalLength, const float fovYStep)
{
	for (int x = 0; x < IMG_WIDTH; x++)
	{	
		const HitRecord &hitR = hitImage[x];
		const unsigned int hitMapType = hitImage[x].mapValue;
	
		glm::vec3 rayDir2D(hitR.dirX, 0.0f, hitR.dirY);
		const glm::vec3 planePosition = cameraPos + rayDir2D * hitR.dist;
		const glm::vec3 planeNormal = getWallNormal(hitR.wallNormal);
		
		for (int y = 0; y < IMG_HEIGHT; y++)
		{
			float theta = 0.0f;
			float t = 0.0f;
			glm::vec3 color(0.0f);

			if (y >= IMG_HEIGHT / 2)
			{
				theta = -static_cast<float>(y - (IMG_HEIGHT / 2)) * fovYStep;
			}
			else
			{
				theta = static_cast<float>((IMG_HEIGHT / 2) - y) * fovYStep;
			}

			const glm::vec3 rayDir =  glm::normalize(glm::vec3(hitR.dirX, sin(theta), hitR.dirY));
			
			// ray wall intersection test 
			const float ON = glm::dot(cameraPos - planePosition, planeNormal);
			const float DN = glm::max(0.00001f, glm::dot(planeNormal, rayDir));
			t = std::fabs(ON / DN);
			const glm::vec3 wallPos = cameraPos + rayDir * t;
			
			if (std::abs(wallPos.y*hitR.cosDir) <= WALL_HEIGHT)
			{	   
				// calculate texture coordinates
				float valY = wallPos.y*hitR.cosDir + WALL_HEIGHT; // make it positive
				float valX = (abs(planeNormal.z) > 0.0001f) ?  wallPos.x : wallPos.z;

				valX *= 0.249f;
				valY *= 0.25f;
				float u = valX - std::floor(valX);
				float v = valY - std::floor(valY);
				const float light = glm::min(1.0f, glm::max(ambientLight, getLight(glm::vec2(wallPos.x, wallPos.z))));
				//color = getBrickTexture(u, v);
				color = wolfensteinGreyBrick(glm::vec2(u,-v)*64.01f);
				color *= light;
			}
			else if(wallPos.y > WALL_HEIGHT) // ceiling
			{	
				// ray ceil intersection
				t = (abs(rayDir.y) > 0.001f) ? (WALL_HEIGHT) / rayDir.y : 100.0f;
				const glm::vec3 ceilPos = cameraPos + rayDir * t;
				float flight =  glm::max(ambientLight, getLight(glm::vec2(ceilPos.x, ceilPos.z))) * 0.15f;
				flight = flight * flight * flight * flight * flight;
				color = glm::vec3(70.0f,90.0f,70.0f) * flight;
			}
			else
			{
				// ray floor intersection
				t = (abs(rayDir.y) > 0.001f) ? (-WALL_HEIGHT) / rayDir.y : 100.0f;
				const glm::vec3 floorPos = cameraPos + rayDir * t;
				const float flight = glm::min(1.0f, glm::max(ambientLight, getLight(glm::vec2(floorPos.x, floorPos.z))));
				color = glm::vec3(50.0f, 50.0f, 50.0f) * flight;
			}

			const float fog = getFogValue(t, 2.0f, 40.0f);
			color = color * fog + fogColor * (1.0f - fog);
			color = glm::vec3(glm::min(255.0f, color.x), glm::min(255.0f, color.y), glm::min(255.0f, color.z));
			setPixel(x, y, glm::ivec3(color.x, color.y, color.z));
		}
	}
}

void rayCastImage(float x, float y, float dirX, float dirY, float fovDeg)
{
	static bool mapCreated = false;
	if (!mapCreated)
	{
		createMap();
		initLightMap();
		for (uint i = 0; i < 300; i++)
		{
			updateLightMap();
		}
		mapCreated = true;
	}

	const float fov = fovDeg * degToRad;
	const float aspectRatio = (static_cast<float>(IMG_HEIGHT) / static_cast<float>(IMG_WIDTH));
	const float fovy = fov * aspectRatio;
	const float fovyStep = fovy / static_cast<float>(IMG_HEIGHT);
	const float imgFocalLength = static_cast<float>(IMG_HEIGHT) / (static_cast<float>((IMG_WIDTH / 2)) / tan(fov * 0.5f));

	// camera setup
	glm::vec2 cameraPos(x, y);
	glm::vec2 cameraDir = glm::normalize(glm::vec2(dirX, dirY));
	
	// camera plane 
	float fp = std::tan(fov * 0.5f);
	glm::vec2 A = cameraPos + cameraDir - glm::vec2(-cameraDir.y, cameraDir.x) * fp;
	glm::vec2 Ap = cameraPos + cameraDir + glm::vec2(-cameraDir.y, cameraDir.x) * fp;
	glm::vec2 B = Ap - A;

	const float step = 1.0f / (static_cast<float>(IMG_WIDTH));
	float s = 0.0f;

	for (int i = 0; i < IMG_WIDTH; i++)
	{
		glm::vec2 d = glm::normalize((A + B * s) - cameraPos);
		float cosdir = glm::dot(d, cameraDir);
		HitRecord hit = rayCastMap(cameraPos, d);
		hitImage[i].dist = hit.dist;
		hitImage[i].mapValue = hit.mapValue;
		hitImage[i].wallNormal = hit.wallNormal;
		hitImage[i].dirX = d.x;
		hitImage[i].dirY = d.y;
		hitImage[i].cosDir = cosdir;
		s += step;
	}

	renderImage(glm::vec3(cameraPos.x,0.0f, cameraPos.y),imgFocalLength,fovyStep);
}

int getMapType(float x, float y)
{
	return getMapValue(glm::ivec2( x, y));
}


