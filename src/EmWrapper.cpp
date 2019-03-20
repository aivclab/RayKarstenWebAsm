#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten.h>

#include "RayLib.h"

using namespace emscripten;

val getImage()
{
	unsigned int uiSize = IMG_WIDTH *IMG_HEIGHT * 4;
	unsigned char *imagePtr = getImagePtr();
	return val(memory_view<uint8_t>(uiSize,imagePtr));
}

EMSCRIPTEN_BINDINGS(ray_module) {
	function("rayCastImage", &rayCastImage);
	function("getMapType", &getMapType);
	function("getImage", &getImage);
}
