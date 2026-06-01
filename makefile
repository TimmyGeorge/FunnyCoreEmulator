.PHONY: fscanf

otg.exe: otg.c makefile
	gcc otg.c -o otg.exe -O0 -g -Wall -Wextra -Wpedantic -Werror -pedantic-errors -Wconversion -std=c23
	
fscanf: fscanf.exe
	fscanf.exe

fscanf.exe: fscanf.c
	gcc fscanf.c -o fscanf.exe
	