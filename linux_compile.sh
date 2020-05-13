g++ -c first_sfml.cpp point.cpp 
g++ first_sfml.o point.o -o space -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
./space

