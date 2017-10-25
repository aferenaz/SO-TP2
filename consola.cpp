#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include "consola.hpp"
#include "HashMap.hpp"
#include "mpi.h"

#include "base.h"

using namespace std;

#define CMD_LOAD    "load"
#define CMD_ADD     "addAndInc"
#define CMD_MEMBER  "member"
#define CMD_MAXIMUM "maximum"
#define CMD_IMPRIMIR "imprimir"
#define CMD_QUIT    "quit"
#define CMD_SQUIT   "q"

static unsigned int np;
extern std::ofstream logFile; 

void ignorar(MPI_Status status){

    //esta función no hace nada. El programa se construyó así
    //por si se necesita, en el futuro,  "acomodar" algo cuando se ignora un mensaje
}

// Crea un ConcurrentHashMap distribuido
static void load(list<string>& params) {

    log("protocolo load iniciado");
    char buffer[BUFFER_SIZE]; 
    MPI_Status status;
    list<string>::iterator it=params.begin();   
    
    //Enviar a todos los nodos
    uint64_t check = set_new_check_data(buffer);
    log("enviando LOAD_REQ a todos los nodos");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, LOAD_REQ, MPI_COMM_WORLD);
    }
    log("enviado LOAD_REQ a todos los nodos");

    while(it != params.end()) {

        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == LOAD_ACK && coincide_check_data(buffer, check)){
            log("recibido LOAD_ACK");
            memset(buffer, 0, BUFFER_SIZE);
            strcpy(buffer, (*it).c_str());
            it++;
            MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, status.MPI_SOURCE, LOAD_DATA, MPI_COMM_WORLD);
            log("enviado LOAD_DATA");
        }
        else{
            ignorar(status);
        }

    };

    //Enviar a todos los nodos
    set_check_data(buffer, check);
    log("enviando LOAD_REL a todos los nodos");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, LOAD_REL, MPI_COMM_WORLD);
    }
    log("enviado LOAD_REL a todos los nodos");

    log("protocolo load terminado");
    //cout << "La listá esta procesada" << endl;
}

// Esta función debe avisar a todos los nodos que deben terminar
static void quit() {

    log("protocolo quit iniciado");
    char buffer[BUFFER_SIZE];

    log("enviando QUIT a todos los nodos");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, QUIT, MPI_COMM_WORLD);
    }
    log("enviado QUIT a todos los nodos. Saliendo...");
    logFile.close();
}

// Esta función calcula el máximo con todos los nodos
static void maximum() {

    log("protocolo maximum iniciado");
    pair<string, unsigned int> result;

    string str("a");
    result = make_pair(str,10);

    char buffer[BUFFER_SIZE];
    MPI_Status status;

    memset(buffer, 0, BUFFER_SIZE);
    HashMap hp;

    uint64_t check = set_new_check_data(buffer);
    log("enviando MAXIMUM_REQ a todos los nodos");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, MAXIMUM_REQ, MPI_COMM_WORLD);
    }
    log("enviado MAXIMUM_REQ a todos los nodos");


    unsigned nodosQueFinalizaron = 0;
    while(nodosQueFinalizaron != (np -1)){
        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(!coincide_check_data(buffer, check)){
            ignorar(status);
            continue;
        }
        switch(status.MPI_TAG){
            case MAXIMUM_DATA:{
                log("recibido MAXIMUM_DATA");
                string key(buffer);
                hp.addAndInc(key);
                break;
            }

            case MAXIMUM_END:{
                log("recibido MAXIMUM_END");
                nodosQueFinalizaron++;
                break;
            }

            default:
                ignorar(status);
        }

    }
    log("recibido MAXIMUM_END de todos los nodos");


    result = hp.maximum();
    log("protocolo maximum terminado");
    cout  << result.first <<" " << result.second << endl;
}
static void imprimir(){

    log("protocolo imprimir iniciado");
    pair<string, unsigned int> result;

    string str("a");
    result = make_pair(str,10);

    char buffer[BUFFER_SIZE];
    MPI_Status status;


    memset(buffer, 0, BUFFER_SIZE);
    HashMap hp;


    uint64_t check = set_new_check_data(buffer);
    log("enviando IMPRIMIR_REQ a todos los nodos");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, IMPRIMIR_REQ, MPI_COMM_WORLD);
    }
    log("enviado IMPRIMIR_REQ a todos los nodos");

    unsigned nodosQueFinalizaron = 0;
    while(nodosQueFinalizaron != (np -1)){


        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(!coincide_check_data(buffer, check)){
            ignorar(status);
            continue;
        }
        switch(status.MPI_TAG){
            case IMPRIMIR_DATA:{
                log("recibido IMPRIMIR_DATA");
                string key(buffer);
                hp.addAndInc(key);
                break;
            }

            case IMPRIMIR_END:{
                log("recibido IMPRIMIR_END");
                nodosQueFinalizaron++;
                break;
            }

            default:
                ignorar(status);
        }

    }


    log("recibido IMPRIMIR_END de todos los nodos");

    hp.printAll();
    log("protocolo imprimir terminado");

}

// Esta función busca la existencia de *key* en algún nodo
static void member(const string& key) {
    
    log("protocolo member iniciado");
    char buffer[BUFFER_SIZE];
    MPI_Status status;

    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, key.c_str());
    uint64_t check = set_new_check_data(buffer);

    bool esta = false;
    log("enviando MEMBER_REQ a todos los nodos");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, MEMBER_REQ, MPI_COMM_WORLD);
    }
    log("enviado MEMBER_REQ a todos los nodos");

    unsigned recibidos = 0;

    while(!esta && recibidos < (np -1)){
        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == MEMBER_DATA && coincide_check_data(buffer, check)){
            log("recibido MEMBER_DATA");
            esta = (buffer[0] == 1);
            recibidos++;
        }
        else{
            ignorar(status);
        }
    }

    log("protocolo member terminado");
    cout << "El string <" << key << (esta ? ">" : "> no") << " está" << endl;
}


// Esta función suma uno a *key* en algún nodo
static void addAndInc(const string& key) {

    log("protocolo addAndInc iniciado");
    char buffer[BUFFER_SIZE];
    MPI_Status status;

    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, key.c_str());

    uint64_t check = set_new_check_data(buffer);
    log("enviando ADD_REQ a todos los nodos");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, ADD_REQ, MPI_COMM_WORLD);
    }
    log("enviado ADD_REQ a todos los nodos");

    bool agregado = false;
    unsigned rankQueProcesa = CONSOLA;

    while(!agregado){
        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(!coincide_check_data(buffer, check)) continue;
        switch(status.MPI_TAG){
            case ADD_ACK:
                log("recibido ADD_ACK");
                agregado = true;
                rankQueProcesa = status.MPI_SOURCE;
                set_check_data(buffer, check);
                MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, status.MPI_SOURCE, ADD_COMMIT, MPI_COMM_WORLD);
                log("enviado ADD_COMMIT");
                break;
            default:
                ignorar(status);
        }
    }

    log("enviando ADD_ROLLBACK a los nodos restantes");
    set_check_data(buffer, check);
    for(unsigned i = 1; i < np; i++)
        if(i != rankQueProcesa)
            MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, ADD_ROLLBACK, MPI_COMM_WORLD);
    log("enviado ADD_ROLLBACK a todos los nodos restantes");

    log("protocolo addAndInc terminado");
    //cout << "Agregado: " << key << endl;
}


/* static int procesar_comandos()
La función toma comandos por consola e invoca a las funciones correspondientes
Si devuelve true, significa que el proceso consola debe terminar
Si devuelve false, significa que debe seguir recibiendo un nuevo comando
*/

static bool procesar_comandos() {

    char buffer[BUFFER_SIZE];
    size_t buffer_length;
    char *res, *first_param, *second_param;

    // Mi mamá no me deja usar gets :(
    res = fgets(buffer, sizeof(buffer), stdin);

    // Permitimos salir con EOF
    if (res==NULL)
        return true;

    buffer_length = strlen(buffer);
    // Si es un ENTER, continuamos
    if (buffer_length<=1)
        return false;

    // Sacamos último carácter
    buffer[buffer_length-1] = '\0';

    // Obtenemos el primer parámetro
    first_param = strtok(buffer, " ");

    if (strncmp(first_param, CMD_QUIT, sizeof(CMD_QUIT))==0 ||
        strncmp(first_param, CMD_SQUIT, sizeof(CMD_SQUIT))==0) {

        quit();
        return true;
    }

    if (strncmp(first_param, CMD_MAXIMUM, sizeof(CMD_MAXIMUM))==0) {
        maximum();
        return false;
    }
    if (strncmp(first_param, CMD_IMPRIMIR, sizeof(CMD_IMPRIMIR))==0) {
        imprimir();
        return false;
    }

    // Obtenemos el segundo parámetro
    second_param = strtok(NULL, " ");
    if (strncmp(first_param, CMD_MEMBER, sizeof(CMD_MEMBER))==0) {
        if (second_param != NULL) {
            string s(second_param);
            member(s);
        }
        else {
            //printf("Falta un parámetro\n");
        }
        return false;
    }

    if (strncmp(first_param, CMD_ADD, sizeof(CMD_ADD))==0) {
        if (second_param != NULL) {
            string s(second_param);
            addAndInc(s);
        }
        else {
            //printf("Falta un parámetro\n");
        }
        return false;
    }

    if (strncmp(first_param, CMD_LOAD, sizeof(CMD_LOAD))==0) {
        list<string> params;
        while (second_param != NULL)
        {
            string s(second_param);
            params.push_back(s);
            second_param = strtok(NULL, " ");
        }

        load(params);
        return false;
    }

    //printf("Comando no reconocido");
    return false;
}

void consola(unsigned int np_param) {
    
    //inicializar log
    logFile.open("consola.log", std::fstream::out);

    np = np_param;
    //printf("Comandos disponibles:\n");
    //printf("  "CMD_LOAD" <arch_1> <arch_2> ... <arch_n>\n");
    //printf("  "CMD_ADD" <string>\n");
    //printf("  "CMD_MEMBER" <string>\n");
    //printf("  "CMD_MAXIMUM"\n");
    //printf("  "CMD_SQUIT"|"CMD_QUIT"\n");

    bool fin = false;
    while (!fin) {
        //printf("> ");
        fflush(stdout);
        fin = procesar_comandos();
    }
}
