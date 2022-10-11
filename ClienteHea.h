#ifndef __CLIENTEHEA_H__
#define __CLIENTEHEA_H__

#include "Mensaje.h"

typedef struct
{
    int ID;
    char *pipeNomCli;
    char modo;
} Cliente;

int ClienteUnfollow(Cliente *cliente, int usuario);
int ClienteFollow(Cliente *cliente, int usuario);
int ClienteTweet(Cliente *cliente, char tweet[]);
int ConectarConGestor(Cliente *cliente);
int DesconectarConGestor(Cliente *cliente);
void RecuperarTweetsCliente(Cliente *cliente);
void ImprimitTweet(Mensaje *mensaje);
Mensaje RecibirMensajeT(Cliente *cliente);
void EnviarMensaje(Cliente *cliente, Mensaje *mensajeEnvio);

#endif