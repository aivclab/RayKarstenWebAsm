#include<windows.h>
#include<iostream>
#include <cmath>
#include <stdio.h>

#include "RayLib.h"

using namespace std;

// Rendering
const int QUAD_SIZE = 10;
const int yOffset = 50;

// Image setup
const float ASPECT = (static_cast<float>(IMG_HEIGHT) / static_cast<float>(IMG_WIDTH));
const float FOV = (60.0f) * degToRad;
const float FOVY = FOV * ASPECT;
const float FOVY_STEP = FOVY / static_cast<float>(IMG_HEIGHT);
const float IMG_FOCAL_LENGTH = static_cast<float>(IMG_HEIGHT) / (static_cast<float>((IMG_WIDTH / 2)) / tanf(FOV * 0.5f));

// World setup
glm::vec2 cameraPos(16.0f, 5.0f);
glm::vec2 cameraDir(0.9f, -0.05f);

void gotoxy(int column, int line)
{
	COORD coord;
	coord.X = column;
	coord.Y = line;
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE),
		coord
	);
}

void SetPos(int XPos, int YPos)
{
	printf("\033[%d;%dH", YPos + 1, XPos + 1);
}

COLORREF getColor(unsigned int mapType,unsigned int lightMap)
{

	unsigned int l0 = lightMap & 1;
	unsigned int l1 = (lightMap >> 1) & 1;
	unsigned int l2 = (lightMap >> 2) & 1;
	
	switch (mapType)
	{
	case MAP_DOOR:
		return RGB(0, 200, 0);
		break;
	case MAP_EMPTY:
		return RGB(10, 10, (l0+l1+l2) * 50);
		break;
	case MAP_LIGHT:
		return RGB(200, 200, 0);
		break;
	}

	return RGB(100, 100, 100);
}

void renderQuad(HDC mydc, COLORREF color, int x, int y)
{
	for (int iy = 0; iy < QUAD_SIZE; iy++)
	{
		for (int ix = 0; ix < QUAD_SIZE; ix++)
		{
			SetPixel(mydc,x + ix, y +iy, color);
		}
	}
}

void renderPixel(HDC mydc, COLORREF color, int x, int y)
{
	SetPixel(mydc, x, y, color);
}


void renderLine(HDC hdc, COLORREF color, const glm::vec2 start, const glm::vec2 end)
{
	glm::vec2 dir = glm::normalize(end - start);
	float len = glm::length(end - start);
	float dist = 1.0f;

	while (dist < len)
	{
		glm::vec2 p = start + dir * dist;
		int x = static_cast<int>(std::roundf(p.x));
		int y = static_cast<int>(std::roundf(p.y));
		renderQuad(hdc, color, x * QUAD_SIZE, y * QUAD_SIZE + yOffset);
		dist += 1.5f;
	}

}

void renderPoint(HDC hdc, COLORREF color, const glm::vec2 p)
{
	int x = static_cast<int>(std::roundf(p.x));
	int y = static_cast<int>(std::roundf(p.y));
	renderQuad(hdc, color, x * QUAD_SIZE, y * QUAD_SIZE + yOffset);
}

void renderMap(HDC mydc)
{
	unsigned int *floorM = getFloorMapPtr();	
	unsigned int *lightM = getLightMapPtr();
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			const unsigned int mt = floorM[x + y * MAP_WIDTH];
			const unsigned int lt = lightM[x + y * MAP_WIDTH];
			renderQuad(mydc, getColor(mt, lt), x * QUAD_SIZE, y * QUAD_SIZE + yOffset);
		}
	}
}

void renderFinalImage(HDC hdc)
{
	unsigned char* img = getImagePtr();
	for (int y = 0; y < IMG_HEIGHT; y++)
	{
		for (int x = 0; x < IMG_WIDTH; x++)
		{
			int idx = ((x + y * IMG_WIDTH) * 4);
			COLORREF color = RGB(img[idx], img[idx+1], img[idx+2]);
			renderPixel(hdc, color, x, y);
		}
	}
}

void renderImage(HDC hdc)
{
	for (int i = 0; i < IMG_WIDTH; i++)
	{
		HitRecord &hitR = getHitRecords()[i];
		const float d = hitR.dist + 1.0f;
		const float l = hitR.lightDepth + 1.0f;
		int pixelH = static_cast<int>(static_cast<float>(IMG_HEIGHT) *  (WALL_HEIGHT /d));
		int lightHeightH = static_cast<int>(static_cast<float>(IMG_HEIGHT) *  (1.0f / (d*0.175f)));
		int floorH = (IMG_HEIGHT - pixelH) / 2;
		int CeilH =  IMG_HEIGHT- ( (IMG_HEIGHT - pixelH) / 2);
		const int m = hitR.mapValue;
		const int light = static_cast<int>(hitR.light*hitR.light) * 60;
		const int di = static_cast<int>( 200.0f - (d*2.0f));

		for (int y = 0; y < IMG_HEIGHT; y++)
		{
			COLORREF col = RGB(0,0,0);
			float offset = std::abs(static_cast<float>(y - (IMG_HEIGHT / 2)));
			float ceilDepth = 1.0f/ (WALL_HEIGHT /(offset+1.0f));
		
			if (m == MAP_WALL)
			{
				col = RGB(di+light, di+light, di);
			}
			else if (m == MAP_DOOR)
			{
				col = RGB(0, 200+light, 50+light);
			}

			if (y < floorH)
			{
				col = RGB(ceilDepth*6, ceilDepth*6, ceilDepth*6);
			}

			if (y >= CeilH)
			{
				col = RGB(10, ceilDepth * 6, ceilDepth * 6);
			}

			renderPixel(hdc, col, i, 400 - y);
		}	
	}
}

float InScatter(glm::vec3 start, glm::vec3 dir, glm::vec3 lightPos, float d)
{
	// light to ray origin
	glm::vec3 q = start - lightPos;

	// coefficients
	float b = glm::dot(dir, q);
	float c = glm::dot(q, q);

	// evaluate integral
	float tmp = sqrt(c - b*b);
	float s = 1.0f / tmp;
	float l = s * (atan((d + b) * s) - atan(b*s));
	return l;
}

void renderDepthImage(HDC hdc)
{
	unsigned int *lightMap = getLightMapPtr();

	for (int x = 0; x < IMG_WIDTH; x++)
	{
		const HitRecord &hitR = getHitRecords()[x];
		const float pixelDepth = hitR.dist;
		int projWallPixelHeight = 0;

		if (pixelDepth > 0.0f)
		{
			projWallPixelHeight = static_cast<int>( (WALL_HEIGHT / (pixelDepth * IMG_FOCAL_LENGTH))  * static_cast<float>(IMG_HEIGHT));
			projWallPixelHeight = (projWallPixelHeight < IMG_HEIGHT / 2) ? projWallPixelHeight : IMG_HEIGHT / 2;
		}
		
		int tmp = (IMG_HEIGHT / 2) - projWallPixelHeight;
		int floorPixelHeight = tmp;
		int ceilPixelHeight  =  IMG_HEIGHT - tmp;
		const float depthScale = 2.0f;

	for (int y = 0; y < IMG_HEIGHT; y++)
		{
			
			float theta = 0.0f;
		
			if (y >= IMG_HEIGHT / 2)
			{
				theta = static_cast<float>(y - (IMG_HEIGHT/2)) * FOVY_STEP;
			}
			else
			{
				theta = -static_cast<float>((IMG_HEIGHT / 2) - y) * FOVY_STEP;
			}

			glm::vec3 rayDir(hitR.dirX, sin(theta), hitR.dirY);
			rayDir = glm::normalize(rayDir);

			if(y < floorPixelHeight) // render floor
			{	
				/*glm::vec2 ray(1.0f, std::sin(theta));
				ray = glm::normalize(ray);
				float t = (abs(ray.y) > 0.001f) ? (-WALL_HEIGHT) / ray.y : 1000.0f;
				t -= IMG_FOCAL_LENGTH;
				t *= depthScale;

				float inScattering = InScatter(glm::vec3(cameraPos.x, 0.0f, cameraPos.y), rayDir, lightPos, t);
				inScattering += InScatter(glm::vec3(cameraPos.x, 0.0f, cameraPos.y), rayDir, lightPos2, t);
				float light = inScattering * LightPower;
				light = (light > 255.0f) ? 255.0f : light;
				COLORREF col = RGB(light * 0.5f, light, light * 0.5f);

			//	COLORREF col = RGB(t + 40, t, t);
				renderPixel(hdc, col, x, IMG_HEIGHT - y);*/
			}
			else if (y > ceilPixelHeight) // render Ceil
			{
			/*	glm::vec2 ray(1.0f, std::sin(theta));
				ray = glm::normalize(ray);
				float t = (abs(ray.y) > 0.001f)?  (WALL_HEIGHT) / ray.y : 1000.0f;
				t -= IMG_FOCAL_LENGTH;
				t *= depthScale;

				float inScattering = InScatter(glm::vec3(cameraPos.x, 0.0f, cameraPos.y), rayDir, lightPos, t);
				inScattering += InScatter(glm::vec3(cameraPos.x, 0.0f, cameraPos.y), rayDir, lightPos2, t);
				float light = inScattering * LightPower;
				light = (light > 255.0f) ? 255.0f : light;
				COLORREF col = RGB(light, light, light * 0.5f);
			//	COLORREF col = RGB(t, t, t + 40);
				renderPixel(hdc, col, x, IMG_HEIGHT - y);*/
			}
			else
			{

				float inScatteredLight = 0.0f;
				unsigned int lightValue = (lightMap[hitR.mapX + hitR.mapY * MAP_WIDTH]);
			
				int mask = 1;
				for (int i = 0; i < NUM_LIGHTS; i++)
				{
					glm::vec4 lI = getLight(i);
					float shadowValue = static_cast<float>((lightValue & mask) >> i) + 0.5f;
					inScatteredLight += InScatter(glm::vec3(cameraPos.x, 0.0f, cameraPos.y), rayDir, glm::vec3(lI.x, lI.y, lI.z), pixelDepth) * lI.w * 10.0f * shadowValue;
					mask *= 2;
				}

			//	inScatteredLight *= 50.0f;
		
			/*	float inScattering = InScatter(glm::vec3(cameraPos.x, 0.0f, cameraPos.y), rayDir, lightPos, pixelDepth);
				inScattering += InScatter(glm::vec3(cameraPos.x, 0.0f, cameraPos.y), rayDir, lightPos2, pixelDepth);
				float light = inScattering * LightPower;*/
				inScatteredLight = (inScatteredLight > 255.0f) ? 255.0f : inScatteredLight;

			//	int pixelDepthColor = static_cast<int>(pixelDepth * depthScale * 0.5f +  inScattering);
				COLORREF col = RGB(inScatteredLight, inScatteredLight, inScatteredLight);
				//COLORREF col = RGB(pixelDepthColor, pixelDepthColor, pixelDepthColor);
				renderPixel(hdc, col, x, IMG_HEIGHT - y);
			}

		}
	}
}


int main()
{

	HWND myconsole = GetConsoleWindow();
	HDC hdc = GetDC(myconsole);

	createMap();
	renderMap(hdc);

	//return 0;

	// test ray marching
	cameraDir = glm::normalize(cameraDir);

	renderLine(hdc,RGB(200, 200, 200),  cameraPos, cameraPos + cameraDir * 20.0f);
	renderPoint(hdc, RGB(200, 0, 0), cameraPos);

	float fp = std::tanf(FOV * 0.5f);
	glm::vec2 A = cameraPos + cameraDir -glm::vec2(-cameraDir.y, cameraDir.x) * fp;
	glm::vec2 Ap = cameraPos + cameraDir + glm::vec2(-cameraDir.y, cameraDir.x) * fp;
	glm::vec2 B = Ap - A;

	renderPoint(hdc, RGB(0, 0, 200), cameraPos + glm::normalize(A-cameraPos) * 3.0f);
	renderPoint(hdc, RGB(0, 200, 200), cameraPos + glm::normalize(Ap - cameraPos) * 3.0f);
	
	/*float step = 1.0f / (static_cast<float>(IMG_WIDTH));
	float s = 0.0f;

	for (int i = 0; i < IMG_WIDTH; i++)
	{
		glm::vec2 d = glm::normalize((A + B * s) - cameraPos);
		float cosdir = glm::dot(d, cameraDir);

		HitRecord &hitRecord = getHitRecords()[i];
		hitRecord = rayCastMap(cameraPos, d);
		hitRecord.dist *= cosdir; // get the projected distance (Fish Bowl effect)
		hitRecord.dirX = d.x;
		hitRecord.dirY = d.y;
		s += step;
	}*/

	rayCastImage(cameraPos.x,cameraPos.y, cameraDir.x, cameraDir.y, degToRad*60.0f);
	renderFinalImage(hdc);

	//renderImage(mydc);
//	renderDepthImage(hdc);
	ReleaseDC(myconsole, hdc);
	cin.ignore();
	return 0;
}