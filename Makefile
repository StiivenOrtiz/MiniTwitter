#REGLA PRINCIPAL - make
all: cliente gestor

#COPILACION DEL PROGRAMA PRINCIPAL
cliente: Cliente.c ClienteHea.h ClienteDef.c
	gcc -o cliente Cliente.c -lpthread

gestor: Gestor.c GestorHea.h GestorDef.c
	gcc -o gestor Gestor.c -lpthread
