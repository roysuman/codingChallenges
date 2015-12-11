all: server client
server: server.cpp connection.hpp function.hpp structure.hpp 
	g++ server.cpp -o server -std=c++11
client: client.cpp connection.hpp function.hpp structure.hpp
	g++ client.cpp -o client -std=c++11 -lpthread

clean: 
	rm server client
