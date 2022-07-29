all:  dijkstra_large dijkstra_small

dijkstra_large: dijkstra_large.c Makefile
	clang ${PARAMS}  -static dijkstra_large.c  -o dijkstra_large

dijkstra_small: dijkstra_small.c Makefile
	clang ${PARAMS}  -static dijkstra_small.c  -o dijkstra_small 

clean:
	rm -rf dijkstra_large dijkstra_small output*
