#!/bin/bash
	rm ./app/frag_automata0000.spv
	clear
	echo ""
	echo "  glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv"
			glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv
	echo ""
	echo "  BEGIN ./app/RunVulkanAutomataGLFW"
	echo ""
	./app/RunVulkanAutomataGLFW
	echo ""
	echo "  END   ./app/RunVulkanAutomataGLFW"
	sleep "0.15"
	echo ""
