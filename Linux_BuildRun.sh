#!/bin/bash

	clear

	echo "  rm ./bin/SlackAutomata"
			rm ./bin/SlackAutomata

	echo "  rm ./bin/noop.spv"
			rm ./bin/noop.spv

	echo "  glslc -O -fshader-stage=compute ./res/shaders/noop.comp -o ./bin/noop.spv"
			glslc -O -fshader-stage=compute ./res/shaders/noop.comp -o ./bin/noop.spv
	echo ""

	echo "  g++ src/*.cpp SlackAutomata.cpp -lvulkan -o ./bin/SlackAutomata"
			g++ src/*.cpp SlackAutomata.cpp -lvulkan -o ./bin/SlackAutomata
	echo ""

	echo "  ./bin/SlackAutomata"
			./bin/SlackAutomata
	echo ""
