#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten.h>

using namespace emscripten;

val getImage()
{
unsigned int uiSize = IMAGE_WIDTH *IMAGE_HEIGHT * 4;
return val(memory_view<uint8_t>(uiSize, &image[0]));
}

EMSCRIPTEN_BINDINGS(ray_module) {
function("rayCast", &rayCast);
function("getImage", &getImage);
}
