#!/bin/bash
	rm ./app/vert_TriQuad.spv
	rm ./app/frag_automata0000.spv
	rm ./app/RunVulkanAutomataSDL
	clear
	echo ""
	echo "  glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv"
			glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv
	echo ""
	echo "  glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv"
			glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv
	echo ""
	echo "  g++ VulkanAutomataSDL.cpp -fconcepts -lSDL2 -lvulkan -o ./app/RunVulkanAutomataSDL"
			g++ VulkanAutomataSDL.cpp -fconcepts -lSDL2 -lvulkan -o ./app/RunVulkanAutomataSDL
	echo ""
	echo "  BEGIN ./app/RunVulkanAutomataSDL"
	echo ""
	./app/RunVulkanAutomataSDL
	echo ""
	echo "  END   ./app/RunVulkanAutomataSDL"
	sleep "0.15"
	echo ""
