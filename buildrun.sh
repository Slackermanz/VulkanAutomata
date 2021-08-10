#!/bin/bash
	rm ./app/vert_TriQuad.spv
	rm ./app/frag_automata0000.spv
	rm ./app/RunVulkanAutomataGLFW
	clear
	echo ""
	echo "  glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv"
			glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv
	echo ""
	echo "  glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv"
			glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv
	echo ""
	echo "  g++ -g -std=c++17 lib/*.cpp VulkanAutomataGLFW.cpp -fconcepts -lglfw -lvulkan -o ./app/RunVulkanAutomataGLFW"
			g++ -g -std=c++17 lib/*.cpp VulkanAutomataGLFW.cpp -fconcepts -lglfw -lvulkan -o ./app/RunVulkanAutomataGLFW
	echo ""
	echo "  BEGIN ./app/RunVulkanAutomataGLFW"
	echo ""
	./app/RunVulkanAutomataGLFW
	echo ""
	echo "  END   ./app/RunVulkanAutomataGLFW"
	sleep "0.15"
	echo ""
