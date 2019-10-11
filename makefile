statnetmake: statnet.c statnet_functions.c
	gcc -O3 -o statnet statnet.c statnet_functions.c
