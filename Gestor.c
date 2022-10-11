#include <stdio.h>  // Entrada y salida de datos
#include <stdlib.h> // Utilizar funciones de asignación de memoria, control de procesos y otros
#include <string.h> // Trabajar con funciones que manejen cadenas de caracteres
#include <time.h>   // uso del tiempo

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

#include "GestorDef.c"

Gestor gestor;
int iniciado = 0; // el pipe

// $ gestor -n Num -r Relaciones -m modo -t time -p pipeNom
//      0    1  2  3      4      5   6   7  8    9   10

void *HiloLectorPipe(void *data);
void *HiloEstadisticas(void *data);
void DefinirMensaje(Mensaje *mensaje);

int main(int cantidadEntrada, char **Entrada)
{
    // Asignar los valores de entrada de argumento para iniciar el gestor.

    // Casos de iniciar el gestor
    // 1 -> Gestor inciado correctamente
    // 2 -> Gestor no pudo ser iniciado por ...
    // 3 -> Gestor no pudo ser iniciado por ...

    iniciado = IniciarGestor(&gestor, cantidadEntrada, Entrada); // Inicia el gestor de acuerdo a los valores de entrada

    if (iniciado == 0)
        return 0;
    else if (iniciado == 1)
    {
        pthread_t hiloLector, hiloEstadisticas;
        int banderaLector, banderaEstadisticas;

        /*creacion del hilo*/
        banderaLector = pthread_create(&hiloLector, NULL, &HiloLectorPipe, NULL);
        banderaEstadisticas = pthread_create(&hiloEstadisticas, NULL, &HiloEstadisticas, NULL);

        if (banderaLector == 0)
            printf("Hilo lector creado con exito.\n\n");
        else
        {
            printf("Hilo lector no creado.\n\n");
            return 0;
        }

        if (banderaEstadisticas == 0)
            printf("Hilo estadisticas creado con exito.\n\n");
        else
        {
            printf("Hilo estadisticas no creado.\n\n");
            return 0;
        }
        /*esperar a que termine*/
        pthread_join(hiloLector, NULL);
        pthread_join(hiloEstadisticas, NULL);
    }
    return 0;
}

// Crea el pipe y se lee le mensaje recibido
void *HiloLectorPipe(void *data)
{
    // Creacion del pipe princiapl
    int creado = 0, pipePrincipal;
    // S_IRUSR - Bit de permiso de LECTURA para el propietario del archivo. En muchos sistemas, este bit es 0400.
    // S_IWUSR - Bit de permiso de ESCRITURA para el propietario del archivo. Por lo general, 0200.

    mode_t modoLectura = S_IRUSR | S_IWUSR; // Asignamos el modo de apertura.
    unlink(gestor.pipeNom);                 // Por si ya existe el pipe.

    if (mkfifo(gestor.pipeNom, modoLectura) == -1)
    {
        perror("mkfifo");
        exit(1);
    }

    do
    {
        printf("Abrio el pipe\n\n");
        pipePrincipal = open(gestor.pipeNom, O_RDONLY);
        if (pipePrincipal == -1)
        {
            perror("Pipe");
            printf("\n");
            printf("\nSe volvera a intentar en 5 segundos.\n");
            sleep(5);
        }
        else
            creado = 1;
    } while (creado == 0);

    // Conexión con el cliente

    int n;

    do
    {
        Mensaje mensajeRecibido;
        n = read(pipePrincipal, &mensajeRecibido, sizeof(Mensaje)); // Se identifica si se envio un mensaje
        if (n > 0)
            DefinirMensaje(&mensajeRecibido); // faltan casos
    } while (iniciado == 1);

    close(pipePrincipal);
}

// Crear el archivo con las estadisticas
void *HiloEstadisticas(void *data)
{
    // Número de usuarios conectados, número total de tweets enviados y recibidos.
    FILE *fp;
    char buffer[80], auxiliar[80], numero[3];

    while (iniciado != 0)
    {
        time_t t = time(NULL);
        struct tm tiempoLocal = *localtime(&t);
        char fechaHora[25];
        char *formato = "%Y-%m-%d %H:%M:%S";
        int bytesEscritos = strftime(fechaHora, sizeof fechaHora, formato, &tiempoLocal);

        printf("..::Estadisticas::..\n");
        printf("%s\n", fechaHora);
        printf("Usuarios conectados: %d\n", gestor.conectados);
        printf("Tweets enviados: %d\n", gestor.tEnviados);
        printf("Tweets recibidos: %d\n\n", gestor.tRecibidos);

        strcat(strcpy(buffer, fechaHora), ": "); // buffer = "AAAA-MM-DD HH:MM:SS:"

        sprintf(numero, "%d", gestor.conectados);         // guardar usuarios conectados
        strcat(strcpy(auxiliar, buffer), "UsuariosON: "); // auxiliar = "AAAA-MM-DD HH:MM:SS: UsuariosON: "
        strcat(strcpy(buffer, auxiliar), numero);         // buffer = "AAAA-MM-DD HH:MM:SS: UsuariosON: n"
        strcat(strcpy(auxiliar, buffer), ", ");           // auxiliar = "AAAA-MM-DD HH:MM:SS: UsuariosON: n, "

        sprintf(numero, "%d", gestor.tEnviados); // guardar usuarios enviados

        strcat(strcpy(buffer, auxiliar), "Tweets enviados: "); // buffer = "AAAA-MM-DD HH:MM:SS: UsuarioON: n, Tweets enviados: "
        strcat(strcpy(auxiliar, buffer), numero);              // auxiliar = "AAAA-MM-DD HH:MM:SS: UsuarioON: n, Tweets enviados: n"
        strcat(strcpy(buffer, auxiliar), ", ");                // buffer = "AAAA-MM-DD HH:MM:SS: UsuarioON: n, Tweets enviados: n, "

        sprintf(numero, "%d", gestor.tRecibidos);               // guardar usuarios recibidos
        strcat(strcpy(auxiliar, buffer), "Tweets recibidos: "); // auxiliar = "AAAA-MM-DD HH:MM:SS: UsuarioON: n, Tweets enviados: n, Tweets recibidos: "
        strcat(strcpy(buffer, auxiliar), numero);               // buffer = "AAAA-MM-DD HH:MM:SS: UsuarioON: n, Tweets enviados: n, Tweets recibidos: n"
        strcat(strcpy(auxiliar, buffer), "\n");                 // auxiliar = "AAAA-MM-DD HH:MM:SS: UsuarioON: n, Tweets enviados: n, Tweets recibidos: n\n"
        strcpy(buffer, auxiliar);

        fp = fopen("Estadisticas.txt", "a");
        fputs(buffer, fp);
        fclose (fp);

        sleep(gestor.tiempo);
    }
}

// Definir el tipo de mensaje que le llega al gestor
void DefinirMensaje(Mensaje *mensaje)
{
    Mensaje mensajeEnvio;

    switch (mensaje->operador)
    {
    // Conexión     - Cliente.ID,c          - Hecho
    case 'c':
        mensajeEnvio = ConectarCliente(&gestor, mensaje);
        EnviarMensaje(&gestor, &mensajeEnvio); // el gestor responde si la comunicion es exitosa con el pipe
        break;
    // Follow       - Cliente.ID,f,usuario  - Por hacer
    case 'f':
        break;
    // Unfollow     - Cliente.ID,u,usuario  - Por hacer
    case 'u':
        break;
    // Tweet        - Cliente.ID,t,mensaje  - Por hacer
    case 't':
        break;
    // Recuperar    - Cliente.ID,r          - Por hacer
    case 'r':
        break;
    // Desconexion  - Cliente.ID,d          - Hecho
    case 'd':
        mensajeEnvio = DesconectarCliente(&gestor, mensaje);
        EnviarMensaje(&gestor, &mensajeEnvio); // el gestor responde si la comunicion es exitosa con el pipe
        break;
    default:
        printf("No se logro definir el mensaje");
        break;
    }
}