#!/bin/bash
	source $1'setup-env.sh'
	rm ./app/RunVkAuto
	rm ./app/vert_TriQuad.spv
	rm ./app/frag_automata0000.spv
	clear
	echo ""
	echo "  $1'x86_64/bin/glslc' -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv"
			$1'x86_64/bin/glslc' -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv
	echo ""
	echo "  $1'x86_64/bin/glslc' -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv"
			$1'x86_64/bin/glslc' -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv
	echo ""
	echo "  g++ VulkanAutomata.cpp -fconcepts -L$1'x86_64/lib/' -lvulkan -o ./app/RunVkAuto"
			g++ VulkanAutomata.cpp -fconcepts -L$1'x86_64/lib/' -lvulkan -o ./app/RunVkAuto
	echo ""
	echo "  BEGIN ./app/VulkanAutomata"
	echo ""
	./app/RunVkAuto
	echo ""
	echo "  END   ./app/VulkanAutomata"
	sleep "0.15"
	echo ""
