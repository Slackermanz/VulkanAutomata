#!/bin/bash
	clear
	echo "  rm ./bin/SlackAutomata"
			rm ./bin/SlackAutomata	
	echo ""
	echo "  g++ src/*.cpp SlackAutomata.cpp -lvulkan -o ./bin/SlackAutomata"
			g++ src/*.cpp SlackAutomata.cpp -lvulkan -o ./bin/SlackAutomata
	echo ""
	echo "  ./bin/SlackAutomata"
			./bin/SlackAutomata
	echo ""
