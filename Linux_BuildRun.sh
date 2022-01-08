#!/bin/bash
	clear
	echo "  rm ./bin/SlackAutomata"
			rm ./bin/SlackAutomata

#	echo "  rm ./bin/compute_test.spv"
#			rm ./bin/compute_test.spv
#	echo "  rm ./bin/comp_gather_kernels.spv"
#			rm ./bin/comp_gather_kernels.spv
#	echo "  rm ./bin/comp_compare_kernels.spv"
#			rm ./bin/comp_compare_kernels.spv
#	echo ""
#
#	echo "  glslc -O -fshader-stage=compute res/shaders/compute_test.comp -o bin/compute_test.spv"
#			glslc -O -fshader-stage=compute res/shaders/compute_test.comp -o bin/compute_test.spv
#	echo ""
#
#	echo "  glslc -O -fshader-stage=compute res/shaders/comp_gather_kernels.glsl -o bin/comp_gather_kernels.spv"
#			glslc -O -fshader-stage=compute res/shaders/comp_gather_kernels.glsl -o bin/comp_gather_kernels.spv
#	echo ""
#
#	echo "  glslc -O -fshader-stage=compute res/shaders/comp_compare_kernels.glsl -o bin/comp_compare_kernels.spv"
#			glslc -O -fshader-stage=compute res/shaders/comp_compare_kernels.glsl -o bin/comp_compare_kernels.spv
#	echo ""

###	echo "  g++ src/*.cpp SlackAutomata.cpp -lglfw -lvulkan -o ./bin/SlackAutomata"
###			g++ src/*.cpp SlackAutomata.cpp -lglfw -lvulkan -o ./bin/SlackAutomata
###	echo ""

	echo "  g++ src/*.cpp SlackAutomata.cpp -lvulkan -o ./bin/SlackAutomata"
			g++ src/*.cpp SlackAutomata.cpp -lvulkan -o ./bin/SlackAutomata
	echo ""

	echo "  ./bin/SlackAutomata"
			./bin/SlackAutomata
	echo ""
