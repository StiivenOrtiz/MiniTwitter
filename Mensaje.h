#ifndef __MENSAJE_H__
#define __MENSAJE_H__

typedef struct
{
    int cliente;
    char operador;
    char data[200];
    char opcional;
} Mensaje;

#endif