#ifndef __GESTOR_H__
#define __GESTOR_H__

#include "Mensaje.h"

typedef struct
{
    int usuariosMax;
    int conectados;
    int relaciones[100][100];
    int usuariosconectados[100];
    char modo;
    int tiempo;
    char *pipeNom;
    char *nombreArchivo;
    Mensaje tweets[100];
    int nTweets; 
    int tEnviados;
    int tRecibidos;
} Gestor;

// cargar las relaciones de acuerdo al archivo.txt proporcionado
int CargarRelaciones(Gestor *ges);
// mostrar la matriz con las relaciones ya cargadas
void MostrarRelaciones(const Gestor *ges);
int IniciarGestor(Gestor *gestor, int cantidadEntrada, char **Entrada);
// mostrar los datos del gestor
void ActualizarRelaciones(Gestor *ges, int clienteID, int usuarioID, char *operacion);
Mensaje UnfollowGestor(Gestor *gestor, Mensaje *mensajeRecibido);
Mensaje FollowGestor(Gestor *gestor, Mensaje *mensajeRecibido);
Mensaje ConectarCliente(Gestor *gestor, Mensaje *mensajeRecibido);
Mensaje DesconectarCliente(Gestor *gestor, Mensaje *mensajeRecibido);
void EnviarMensaje(Gestor *gestor, Mensaje *mensajeEnvio);

#endif