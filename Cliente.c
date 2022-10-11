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

// Libreria de hilos
#include <pthread.h>

#include "ClienteDef.c"

Cliente cliente;
int conexion = 0;

void Shell();
char *LeerEntrada();
char **RemoverEspacios(char *entrada);
void *HiloLectorPipe(void *data);
int Acciones(char **argumentos);

// $ cliente -i ID -p pipeNom

// Mensajes
// Follow       - Cliente.ID,f,usuario  - Por hacer
// Unfollow     - Cliente.ID,u,usuario  - Por hacer
// Tweet        - Cliente.ID,t,mensaje  - Por hacer
// Conexión     - Cliente.ID,c          - Hecho
// Desconexion  - Cliente.ID,d          - Hecho
// Recuperar    - Cliente.ID,r          - Por hacer

int main(int cantidadEntrada, char **Entrada)
{
    // Asignar los valores de entrada de argumento para iniciar el cliente.
    for (int i = 0; i < cantidadEntrada; i++)
    {
        if ((strcmp(Entrada[i], "-i") == 0) || (strcmp(Entrada[i], "-I") == 0))
        {
            int numero = 0;
            i++;
            numero = atoi(Entrada[i]);

            if (numero < 0)
            {
                printf("\nEl cliente accedido no existe.\n\n");
                return 0;
            }

            cliente.ID = numero; // ID del cliente casteandolo como int
        }
        else if ((strcmp(Entrada[i], "-p") == 0) || (strcmp(Entrada[i], "-P") == 0))
        {
            i++;
            cliente.pipeNomCli = Entrada[i]; // Nombre del pipe
        }
    }

    conexion = ConectarConGestor(&cliente); // Se realiza la conexion con el gestor

    // 0 - Conexión fracaso
    // 1 - Conexión éxitosa

    if (conexion == 1)
    {
        pthread_t hiloLector;
        int banderaLector;

        /*creating thread*/
        banderaLector = pthread_create(&hiloLector, NULL, &HiloLectorPipe, NULL);

        if (banderaLector != 0)
            return 0;

        Shell();
    }

    return 0;
}

void Shell()
{
    char *entrada;
    char **argumentos;
    int salir = 0;

    do
    {
        // liberar memoria
        printf("$");

        entrada = LeerEntrada();

        if (entrada != NULL)
        {
            argumentos = RemoverEspacios(entrada);
            salir = Acciones(argumentos);
        }

        free(entrada);
        free(argumentos);
    } while (salir != 1);
}

char *LeerEntrada()
{
    size_t linea;
    size_t buffSize = 100; // guardar el tamano del buffer
    char *buffer;          //(arreglo de caracteres) cadena que guarda lo que se digite

    // asignar dinamicamente la memoria utilizando malloc(), este retorna un apuntador
    buffer = malloc(buffSize * sizeof(char *)); // Se puede dejar simplemente buffSize?

    // leer la linea con getline (la direccion del primer caracter, el tamano, secuencia a leer (lo que se digite))
    linea = getline(&buffer, &buffSize, stdin);

    return buffer;
}

char **RemoverEspacios(char *entrada)
{
    size_t buffSize = 100, i = 0;

    // delimitador para remover caracteres de espacio blancos (clase de caraceteres POSIX)
    char *delimitador = " \t\r\n\v\f ";
    char **args; // declarar arreglo de strings a retornar
    args = malloc(buffSize * sizeof(char *));

    char *tokenizer = strtok(entrada, delimitador); // pasar la referencia de la cadena de caracteres
    while (tokenizer != NULL)
    {
        args[i] = tokenizer;
        tokenizer = strtok(NULL, delimitador); // recorrer la cadena y separar segun el delimitador
        i++;
    }

    args[i] = NULL;

    return args;
}

int Acciones(char **argumentos)
{
    int cantidad = 0, posicion = 0;

    if (argumentos[posicion] == NULL)
        return 0;

    while (argumentos[cantidad] != NULL)
        cantidad++;

    if ((strcmp(argumentos[posicion], "a") == 0) || (strcmp(argumentos[posicion], "A") == 0))
    {
        printf("            .::Comandos disponibles::.\n\n");
        printf("    f [usuario] - Follow te permite seguir a un usuario que aun no estes siguiendo.\n");
        printf("    u [usuario] - Unfollow te da la oportunidad de dejar de seguir a un usuario que estes siguiendo.\n");
        printf("    t [tweet] - Permite realizar un tweet de maximo 200 caracteres a tus seguidores.\n");
        if (cliente.modo == 'D')
            printf("    r - Recupera los tweets que ya se han enviado durante el tiempo que el servidor haya estado abierto.\n");
        printf("    a - Esta opción mostrará todos los comandos disponibles.\n");
        printf("    d - Permite desconectarse del servidor. \n\n");
    }
    else if ((strcmp(argumentos[posicion], "f") == 0) || (strcmp(argumentos[posicion], "F") == 0))
    {
    }
    else if ((strcmp(argumentos[posicion], "u") == 0) || (strcmp(argumentos[posicion], "U") == 0))
    {
    }
    else if ((strcmp(argumentos[posicion], "t") == 0) || (strcmp(argumentos[posicion], "T") == 0))
    {
    }
    else if ((strcmp(argumentos[posicion], "r") == 0) || (strcmp(argumentos[posicion], "R") == 0))
    {
        if (cliente.modo == 'D')
        {
        }
        else
            printf("Comando no disponible para Gestor en modo acoplado\n");
    }
    else if ((strcmp(argumentos[posicion], "d") == 0) || (strcmp(argumentos[posicion], "D") == 0))
        return DesconectarConGestor(&cliente);

    return 0;
}

// Crea el pipe del cliente
void *HiloLectorPipe(void *data)
{
    int PipeReceptor, creado = 0;
    char numero[2], pipeNomCliT[10];

    sprintf(numero, "%d", cliente.ID);
    strcat(strcpy(pipeNomCliT, "cliente"), numero); // Auxiliar = ClienteTN

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
            creado = 1;
    } while (creado == 0);

    int n;

    do
    {
        Mensaje mensajeRecibido;
        n = read(PipeReceptor, &mensajeRecibido, sizeof(Mensaje));
        if (n > 0)
            ImprimitTweet(&mensajeRecibido);
    } while (conexion == 0);

    close(PipeReceptor);
    unlink(pipeNomCliT);
}