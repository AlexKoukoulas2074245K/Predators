#!/bin/bash
echo "-- Checking for Brew installation"
which -s brew
if [[ $? != 0 ]] ; then
  echo "-- Brew was not found. Installing via Ruby"
  ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
else
  echo "-- Found Brew"   
fi

if brew ls --versions SDL2 > /dev/null; then
  echo "-- Found SDL2 installation with Brew"
else
  echo "-- SDL2 was not found in the system. Installing via Brew"
  brew install SDL2
fi

if brew ls --versions sdl2_image > /dev/null; then
  echo "-- Found SDL2 Image installation with Brew"
else
  echo "-- SDL2 Image was not found in the system. Installing via Brew"
  brew install sdl2_image
fi

if brew ls --versions freetype > /dev/null; then
  echo "-- Found Freetype installation with Brew"
else
  echo "-- Freetype was not found in the system. Installing via Brew"
  brew install freetype
fi

echo "-- Running CMAKE"
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=./ios_toolchain.cmake -DIOS_PLATFORM=OS
make
make install