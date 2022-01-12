#!/bin/bash

	clear

	echo "  rm -r ./build"
			rm -r ./build

	echo "  mkdir build"
			mkdir build

	echo "  cd build"
			cd build

	echo "  cmake .."
			cmake ..

	echo "  make"
			make

	echo "  ./vulkan_automata"
			./vulkan_automata
