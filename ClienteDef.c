#include <stdio.h>  // Entrada y salida de datos
#include <stdlib.h> // Utilizar funciones de asignación de memoria, control de procesos y otros

// Librerias utilizadas para el manejo de pipes
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "ClienteHea.h"


int ConectarConGestor(Cliente *cliente)
{
    // 1 - Conexión éxitosa
    // 2 - La ID no es válida
    // 3 - El usuario ya se encuentra conectado

    Mensaje mensajeEnvio;

    //se la da valores inicicales 
    mensajeEnvio.cliente = cliente->ID;
    mensajeEnvio.operador = 'c';
    strcpy(mensajeEnvio.data, "");

    //..::Apartado para pipe::.

    EnviarMensaje(cliente, &mensajeEnvio);

    // Crea un pipe para recibir el mensaje del gestor
    Mensaje mensajeRecibido;
    mensajeRecibido = RecibirMensajeT(cliente);

    int res = (int)mensajeRecibido.operador;
    res = res - 48;

    if (res == 1)
    {
        printf("\n%s\n\n", mensajeRecibido.data);
        cliente->modo = mensajeRecibido.opcional;
    }
    else
    {
        printf("\n%s\n\n", mensajeRecibido.data);
    }

    return res;
};

int DesconectarConGestor(Cliente *cliente)
{
    // 0 - Desconexión fracasada
    // 1 - Desconexión éxitosa

    Mensaje mensajeEnvio;

    mensajeEnvio.cliente = cliente->ID;
    mensajeEnvio.operador = 'd';
    strcpy(mensajeEnvio.data, "");

    /*
    Apartado para pipe
    */

    EnviarMensaje(cliente, &mensajeEnvio);

    Mensaje mensajeRecibido;

    mensajeRecibido = RecibirMensajeT(cliente);

    int res = (int)mensajeRecibido.operador;
    res = res - 48;

    if (res == 1)
    {
        printf("\n%s\n\n", mensajeRecibido.data);
        return 1;
    }
    else
    {
        printf("\n%s\n\n", mensajeRecibido.data);
        return 0;
    }
};

void EnviarMensaje(Cliente *cliente, Mensaje *mensajeEnvio)
{
    int PipeConector;

    // Conecta con el pipe del gesotr.
    do
    {
        PipeConector = open(cliente->pipeNomCli, O_WRONLY);
        if (PipeConector == -1)
        {
            perror("ComunicacionPipe");
            printf("\n\n");
            sleep(1);
        }
    } while (PipeConector == -1);

    // Manda el mensaje al gestor.
    write(PipeConector, mensajeEnvio, sizeof(Mensaje));
    // Cierra el pipe con el gestor
    close(PipeConector);
}

Mensaje RecibirMensajeT(Cliente *cliente)
{
    int PipeReceptor, creado = 0;
    char numero[2], pipeNomCliT[10];

    sprintf(numero, "%d", cliente->ID);
    strcat(strcpy(pipeNomCliT, "clienteT"), numero); // Auxiliar = ClienteTN

    mode_t modoLectura = S_IRUSR | S_IWUSR; // Asignamos el modo de apertura.
    unlink(pipeNomCliT);                    // Por si ya existe el pipe.

    if (mkfifo(pipeNomCliT, modoLectura) == -1)
    {
        perror("mkfifo");
        exit(1);
    }

    do
    {
        PipeReceptor = open(pipeNomCliT, O_RDONLY);
        if (PipeReceptor == -1)
        {
            perror("Pipe");
            sleep(1);
        }
        else
        {
            creado = 1;
        }
    } while (creado == 0);

    Mensaje mensajeRecibido;

    read(PipeReceptor, &mensajeRecibido, sizeof(mensajeRecibido));
    close(PipeReceptor);
    unlink(pipeNomCliT);

    return mensajeRecibido;
}


int ClienteUnfollow(Cliente *cliente, int usuario)
{
    // 1 - Unfollow con exito
    // 0 - Unfollow sin exito

    /*-> cliente.ID,2,u - mensaje hacía el gestor
    <- "0" o "1" - mensaje del gestor
    */

    Mensaje mensajeEnvio;

    mensajeEnvio.cliente = cliente->ID;
    mensajeEnvio.operador = 'u';
    sprintf(mensajeEnvio.data, "%d", usuario); // Castear int a cadena de caracteres

    /*
        Apartado para pipe
    */

    Mensaje mensajeRecibido;

    return atoi(mensajeRecibido.data);
};

int ClienteFollow(Cliente *cliente, int usuario)
{
    // 1 - Follow con exito
    // 0 - Follow sin exito

    Mensaje mensajeEnvio;

    mensajeEnvio.cliente = cliente->ID;
    mensajeEnvio.operador = 'f';
    sprintf(mensajeEnvio.data, "%d", usuario); // Castear int a cadena de caracteres

    /*
        Apartado para pipe
    */

    Mensaje mensajeRecibido;

    return atoi(mensajeRecibido.data);
};

int ClienteTweet(Cliente *cliente, char tweet[])
{
    // 1 - Tweet con exito
    // 0 - Tweet sin exito

    Mensaje mensajeEnvio;

    mensajeEnvio.cliente = cliente->ID;
    mensajeEnvio.operador = 't';
    strcpy(mensajeEnvio.data, tweet);

    /*
        Apartado para pipe
    */

    Mensaje mensajeRecibido;

    return atoi(mensajeRecibido.data);
};

void RecuperarTweetsCliente(Cliente *cliente)
{
    Mensaje mensajeEnvio;

    mensajeEnvio.cliente = cliente->ID;
    mensajeEnvio.operador = 'r';
    strcpy(mensajeEnvio.data, "");

    /*
    Apartado para pipe
    */

    Mensaje mensajeRecibido[1000];

    int cantidad = 0;

    while (&mensajeRecibido[cantidad] != NULL)
    {
        cantidad++;
    }

    for (int i = 0; i < cantidad; i++)
    {
        ImprimitTweet(&mensajeRecibido[i]);
    }
};

void ImprimitTweet(Mensaje *mensaje)
{
    printf("\n  Usuario: %d\n", mensaje->cliente);
    printf("  Tweet: %s\n", mensaje->data);
};


