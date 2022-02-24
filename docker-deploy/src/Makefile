TARGETS=main

all: $(TARGETS)

main: main.cpp 
	g++ -g -o main main.cpp proxy.cpp Service.cpp request.cpp response.cpp Cache.cpp -lpthread

clean:
	rm -f $(TARGETS) *~
