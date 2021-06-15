#!/bin/bash
#	source '/home/pseudo/vulkan-sdk/1.2.170.0/setup-env.sh'
	rm ./app/RunVkAuto
	rm ./app/vert_TriQuad.spv
	rm ./app/frag_automata0000.spv
	clear
	echo ""
	echo "  ./glslc -O ./dat/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv"
	glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv
	echo "  ./glslc -O ./dat/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv"
	glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv
	echo "  ./glslc -O ./dat/frag/ParamUpdate.frag -o ./app/ParamUpdate.spv"
	glslc -O ./res/frag/ParamUpdate.frag -o ./app/ParamUpdate.spv
	echo ""
	echo "  g++ VulkanAutomata.cpp -fconcepts -lX11 -L/home/pseudo/vulkan-sdk/1.2.170.0/x86_64/lib/ -lvulkan -o ./app/RunVkAuto"
	echo ""
	g++ VulkanAutomata.cpp -fconcepts -lX11 -L/home/pseudo/vulkan-sdk/1.2.170.0/x86_64/lib/ -lvulkan -o ./app/RunVkAuto
	echo ""
	echo "  BEGIN ./app/VulkanAutomata"
	echo ""
	./app/RunVkAuto
	echo ""
	echo "  END   ./app/VulkanAutomata"
	sleep "0.15"
	echo ""
