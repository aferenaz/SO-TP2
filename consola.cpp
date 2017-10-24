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

using namespace std;

#define CMD_LOAD    "load"
#define CMD_ADD     "addAndInc"
#define CMD_MEMBER  "member"
#define CMD_MAXIMUM "maximum"
#define CMD_QUIT    "quit"
#define CMD_SQUIT   "q"

static unsigned int np;


std::ofstream logFile;
uint64_t op_count = 0;
void log(const std::string& txt){

	time_t current_time;
	struct tm * time_info;
	char timeString[9];  // espacio para "HH:MM:SS\0"

	time(&current_time);
	time_info = localtime(&current_time);
	strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);

	logFile << timeString << " : " << txt << std::endl;
}


bool coincide_check_data(const char* buffer, uint64_t check){
    uint64_t* input = (uint64_t*) &buffer[CHECK_DATA];
    return (*input) == check;
}

void set_check_data(char* buffer, uint64_t data){
	uint64_t* check = (uint64_t*) &buffer[CHECK_DATA];
    (*check) = data;
}

//establece el check data y lo devuelve
uint64_t set_new_check_data(char* buffer){

	uint64_t current = op_count++;
    uint64_t* check = (uint64_t*) &buffer[CHECK_DATA];
    (*check) = current;
    return current; 
}

// Crea un ConcurrentHashMap distribuido

static void load(list<string>& params) {

    log("LOAD");
    char buffer[BUFFER_SIZE]; 
    MPI_Status status;
    list<string>::iterator it=params.begin();   
    
    //Enviar a todos los nodos
    uint64_t check = set_new_check_data(buffer);
    log("LOAD_REQ");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, LOAD_REQ, MPI_COMM_WORLD);
    }
    log("LOAD_REQ");

    while(it != params.end()) {

        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == LOAD_ACK && coincide_check_data(buffer, check)){
            log("LOAD_ACK");
            memset(buffer, 0, BUFFER_SIZE);
            strcpy(buffer, (*it).c_str());
            it++;
            MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, status.MPI_SOURCE, LOAD_DATA, MPI_COMM_WORLD);
            log("LOAD_DATA");
        }
        else{
            log("no hace nada");
        }

    };

    //Enviar a todos los nodos
    set_check_data(buffer, check);
    log("LOAD_REL");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, LOAD_REL, MPI_COMM_WORLD);
    }
    log("LOAD_REL");

    log("FIN LOAD");
    //cout << "La listá esta procesada" << endl;
}

// Esta función debe avisar a todos los nodos que deben terminar
static void quit() {

    log("QUIT iniciado");
    char buffer[BUFFER_SIZE];

    log("QUIT");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, QUIT, MPI_COMM_WORLD);
    }
    log("QUIT fin");
    logFile.close();
}


static void member(const string& key) {
    
    log("MEMBER");
    char buffer[BUFFER_SIZE];
    MPI_Status status;

    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, key.c_str());
    uint64_t check = set_new_check_data(buffer);

    bool esta = false;
    log("MEMBER_REQ");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, MEMBER_REQ, MPI_COMM_WORLD);
    }
    log("MEMBER_REQ ultimo");

    unsigned recibidos = 0;

    while(!esta && recibidos < (np -1)){
        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == MEMBER_DATA && coincide_check_data(buffer, check)){
            log("MEMBER_DATA recibe");
            esta = (buffer[0] == 1);
            recibidos++;
        }
        else{
            log("no hace nada");
        }
    }

    log("MEMBERFIN");
    cout << "El string <" << key << (esta ? ">" : "> no") << " está" << endl;
}


// Esta función suma uno a *key* en algún nodo
static void addAndInc(const string& key) {

    log("addAndInc");
    char buffer[BUFFER_SIZE];
    MPI_Status status;

    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, key.c_str());

    uint64_t check = set_new_check_data(buffer);
    log("ADD_REQ");
    for(unsigned i = 1; i < np; i++){
        MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, ADD_REQ, MPI_COMM_WORLD);
    }
    log("ADD_REQ");

    bool agregado = false;
    unsigned rankQueProcesa = CONSOLA;

    while(!agregado){
        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(!coincide_check_data(buffer, check)) continue;
        switch(status.MPI_TAG){
            case ADD_ACK:
                log("ADD_ACK");
                agregado = true;
                rankQueProcesa = status.MPI_SOURCE;
                set_check_data(buffer, check);
                MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, status.MPI_SOURCE, ADD_COMMIT, MPI_COMM_WORLD);
                log("ADD_COMMIT");
                break;
            default:
                log("no hace nada");
        }
    }

    log("ADD_ROLLBACK");
    set_check_data(buffer, check);
    for(unsigned i = 1; i < np; i++)
        if(i != rankQueProcesa)
            MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, i, ADD_ROLLBACK, MPI_COMM_WORLD);
    log("ADD_ROLLBACK");

    log("addAndInc FIN");
  
}


/* static int procesar_comandos()
La función toma comandos por consola e invoca a las funciones correspondientes
Si devuelve true, significa que el proceso consola debe terminar
Si devuelve false, significa que debe seguir recibiendo un nuevo comando
*/





// Esta función calcula el máximo con todos los nodos
static void maximum() {
    pair<string, unsigned int> result;

    // TODO: Implementar
    string str("a");
    result = make_pair(str,10);

    cout << "El máximo es <" << result.first <<"," << result.second << ">" << endl;
}


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

    // Obtenemos el segundo parámetro
    second_param = strtok(NULL, " ");
    if (strncmp(first_param, CMD_MEMBER, sizeof(CMD_MEMBER))==0) {
        if (second_param != NULL) {
            string s(second_param);
            member(s);
        }
        else {
            printf("Falta un parámetro\n");
        }
        return false;
    }

    if (strncmp(first_param, CMD_ADD, sizeof(CMD_ADD))==0) {
        if (second_param != NULL) {
            string s(second_param);
            addAndInc(s);
        }
        else {
            printf("Falta un parámetro\n");
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

    printf("Comando no reconocido");
    return false;
}

void consola(unsigned int np_param) {
    np = np_param;
    printf("Comandos disponibles:\n");
    printf("  "CMD_LOAD" <arch_1> <arch_2> ... <arch_n>\n");
    printf("  "CMD_ADD" <string>\n");
    printf("  "CMD_MEMBER" <string>\n");
    printf("  "CMD_MAXIMUM"\n");
    printf("  "CMD_SQUIT"|"CMD_QUIT"\n");

    bool fin = false;
    while (!fin) {
        printf("> ");
        fflush(stdout);
        fin = procesar_comandos();
    }
}
