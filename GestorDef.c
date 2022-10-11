#include <stdio.h>  // Entrada y salida de datos
#include <stdlib.h> // Utilizar funciones de asignación de memoria, control de procesos y otros
#include <string.h> // Trabajar con funciones que manejen cadenas de caracteres

// Librerias utilizadas para el manejo de pipes
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "GestorHea.h"

// Asigna valores al gestor segun la entrada
int IniciarGestor(Gestor *gestor, int cantidadEntrada, char **Entrada)
{
    int Num = 0;
    // $ gestor -n Num -r Relaciones -m modo -t time -p pipeNom
    for (int i = 0; i < cantidadEntrada; i++)
    {
        if ((strcmp(Entrada[i], "-n") == 0) || (strcmp(Entrada[i], "-N") == 0))
        {
            i++;
            Num = atoi(Entrada[i]); // leer el numero

            if (Num > 0 && Num <= 100) // verificar que el numero sea valido
                gestor->usuariosMax = Num;
            else
            {
                printf("ERROR: No pueden haber mas de 100 usuarios.\n\n");
                return 0;
            }
        }
        else if ((strcmp(Entrada[i], "-r") == 0) || (strcmp(Entrada[i], "-R") == 0))
        {
            i++;
            gestor->nombreArchivo = Entrada[i];

            Num = CargarRelaciones(gestor);

            if (Num == 1)
                printf("Relaciones cargadas exitosamente.\n\n");
            else
            {
                printf("ERROR: Las relaciones no han podido ser cargadas con exito.\n\n");
                return 0;
            }
        }
        else if ((strcmp(Entrada[i], "-m") == 0) || (strcmp(Entrada[i], "-M") == 0))
        {
            i++;
            if (strcmp(Entrada[i], "D") == 0)
                gestor->modo = 'D';
            else if (strcmp(Entrada[i], "A") == 0)
                gestor->modo = 'A';
            else
            {
                printf("\nERROR: El modo del gestor no ha podido ser determinado.\n\n");
                return 0;
            }
        }
        else if ((strcmp(Entrada[i], "-t") == 0) || (strcmp(Entrada[i], "-T") == 0))
        {
            i++;
            Num = atoi(Entrada[i]);

            if (Num > -1)
                gestor->tiempo = Num;
            else
            {
                printf("\nERROR: El tiempo es negativo.\n\n");
                return 0;
            }
        }
        else if ((strcmp(Entrada[i], "-p") == 0) || (strcmp(Entrada[i], "-P") == 0))
        {
            i++;
            gestor->pipeNom = Entrada[i];
        }
    }

    gestor->conectados = 0;
    gestor->nTweets = 0;
    gestor->tEnviados = 0;
    gestor->tRecibidos = 0;

    return 1;
};

// Cargar las realaciones en el gestor
int CargarRelaciones(Gestor *ges)
{
    FILE *archivo;
    char linea[200];
    char DELIMITADOR[] = " \t	";
    char *token;
    int fila = 0;

    /* intentar abrir el archivo */
    archivo = fopen(ges->nombreArchivo, "r"); // abrir el archivo en modo lectura
    if (archivo == NULL)
    {
        perror("Error: ");
        return 0;
    }
    else
    {
        printf("\nEl archivo se abrio correctamente.. \n");
        while (!feof(archivo))
        {
            fgets(linea, 200, archivo);
            token = strtok(linea, DELIMITADOR);

            for (int columna = 0; columna < ges->usuariosMax; columna++)
            {
                if(token != NULL)
                    ges->relaciones[fila][columna] = atoi(token);

                token = strtok(NULL, DELIMITADOR);
            }
            fila++;
        }
        fclose(archivo);
    }

    printf("\n\nRELACIONES\n");
    MostrarRelaciones(ges);
    return 1;
}

// Enviar mensaje para verificar
void EnviarMensaje(Gestor *gestor, Mensaje *mensajeEnvio)
{
    //..::Apartado para pipe::.
    int PipeClienteEnvio;
    char numero[2], pipeNomCli[10];

    sprintf(numero, "%d", mensajeEnvio->cliente);   // se guarda el número del cliente
    strcat(strcpy(pipeNomCli, "clienteT"), numero); // pipeNomCli = ClienteTN

    int vez = 0;

    // Conecta con el pipe del gestor.
    do
    {
        PipeClienteEnvio = open(pipeNomCli, O_WRONLY);
        if ((PipeClienteEnvio == -1) && (vez == 1000))
        {
            printf("%d\n", vez);
            perror("ComunicacionPipe");
            printf("\n");
            sleep(1);
        }
        vez+=1;
    } while (PipeClienteEnvio == -1);

    // Manda el mensaje al gestor.
    write(PipeClienteEnvio, mensajeEnvio, sizeof(Mensaje));
    // Cierra el pipe con el gestor
    close(PipeClienteEnvio);
};

// Se realiza la conexion con el cliente
Mensaje ConectarCliente(Gestor *gestor, Mensaje *mensajeRecibido)
{
    Mensaje mensajeEnvio; // se crea el mensaje que se va a encviar

    mensajeEnvio.cliente = mensajeRecibido->cliente;
    mensajeEnvio.opcional = gestor->modo;

    // se verifica que el cliente sea valido
    if ((mensajeRecibido->cliente >= gestor->usuariosMax) || (mensajeRecibido->cliente < 0))
    {
        mensajeEnvio.operador = '0';
        strcpy(mensajeEnvio.data, "El cliente accedido no existe.");
    }
    else
    {
        if (gestor->usuariosconectados[mensajeRecibido->cliente] == 1)
        {
            mensajeEnvio.operador = '0';
            strcpy(mensajeEnvio.data, "El cliente accedido ya esta conectado.");
        }
        else
        {
            gestor->usuariosconectados[mensajeRecibido->cliente] = 1;
            gestor->conectados++;
            mensajeEnvio.operador = '1';
            strcpy(mensajeEnvio.data, "Conexion realizada con exito.");
        }
    }

    return mensajeEnvio;
};

// Se desconecta con el cliente
Mensaje DesconectarCliente(Gestor *gestor, Mensaje *mensajeRecibido)
{
    Mensaje mensajeEnvio;

    mensajeEnvio.cliente = mensajeRecibido->cliente;

    // se verifica que el cliente sea valido
    if ((mensajeRecibido->cliente >= gestor->usuariosMax) || (mensajeRecibido->cliente < 0))
    {
        mensajeEnvio.operador = '0';
        strcpy(mensajeEnvio.data, "El cliente accedido no existe.");
    }
    else
    {
        if (gestor->usuariosconectados[mensajeRecibido->cliente] == 0)
        {
            mensajeEnvio.operador = '0';
            strcpy(mensajeEnvio.data, "El cliente accedido ya se encuentra desconectado.");
        }
        else
        {
            gestor->usuariosconectados[mensajeRecibido->cliente] = 0;
            gestor->conectados--;
            mensajeEnvio.operador = '1';
            strcpy(mensajeEnvio.data, "Desconexion realizada con exito.");
        }
    }

    return mensajeEnvio;
};

// Mostrar las relaciones que tiene cargadas el gestor
void MostrarRelaciones(const Gestor *ges)
{
    printf("\n");
    for (int i = 0; i < ges->usuariosMax; i++)
    {
        for (int j = 0; j < ges->usuariosMax; j++)
        {
            printf("%d ", ges->relaciones[i][j]);
        }
        printf("\n");
    }
    printf("\n");
};

void ActualizarRelaciones(Gestor *ges, int clienteID, int usuarioID, char *operacion){
    // printf("Esta es la operacion; \n", operacion);
};

Mensaje UnfollowGestor(Gestor *gestor, Mensaje *mensajeRecibido)
{
    if (gestor->relaciones[mensajeRecibido->cliente][atoi(mensajeRecibido->data)] == 1)
    {
        gestor->relaciones[mensajeRecibido->cliente][atoi(mensajeRecibido->data)] = 0;
    }
    else
    {
    }
};

Mensaje FollowGestor(Gestor *gestor, Mensaje *mensajeRecibido)
{
    Mensaje mensajeEnvio;
    char numeracion[3];

    if (gestor->relaciones[mensajeRecibido->cliente][atoi(mensajeRecibido->data)] == 0)
    {
        gestor->relaciones[mensajeRecibido->cliente][atoi(mensajeRecibido->data)] = 1;

        sprintf(numeracion, "%d", atoi(mensajeRecibido->data));
        strcat(strcpy(mensajeEnvio.data, "Empezaste a seguir al usuario "), numeracion);
    }
    else
    {
        sprintf(numeracion, "%d", atoi(mensajeRecibido->data));
        strcat(strcpy(mensajeEnvio.data, "Ya sigues al usuario "), numeracion);
    }

    return mensajeEnvio;
};
