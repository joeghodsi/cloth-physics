# Arash Ghodsi (aghodsi)
# CMPS 161 - Animation & Visualization
# Winter 2011
# Makefile for clothSim

main : main.o skirt.o quaternion.o
	g++ -o clothSim.exe main.o skirt.o quaternion.o -lglut32 -lopengl32 -lglu32

main.o : main.cpp skirt.h
	g++ -c -ansi -Wall main.cpp

skirt.o: skirt.cpp skirt.h quaternion.h
	g++ -c -ansi -Wall skirt.cpp

quaternion.o: quaternion.cpp quaternion.h
	g++ -c -ansi -Wall quaternion.cpp

clean :
	rm -f clothSim.exe main.o skirt.o quaternion.o
