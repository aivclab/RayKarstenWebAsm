#!/bin/bash
cd buildWeb
make
cd ../
mv -v src/RayKarsten.wasm ./
