#include "nodo.hpp"
#include "HashMap.hpp"
#include <unistd.h>
#include <stdlib.h>

#include "base.h"
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>

using namespace std;

//variables globales del nodo
HashMap hashMap;

void nodo(unsigned int rank) {

	char buffer[BUFFER_SIZE];
	MPI_Status status;
	hashMap = HashMap();

	//convertir rank a string
	string Result;
	stringstream convert;
	convert << "rank";
	convert << rank;
	convert << ".log";

	logFile.open(convert.str().c_str(), std::fstream::out);

	bool continuar = true;
    while (continuar) {
        MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        uint64_t* check = (uint64_t*) &buffer[CHECK_DATA]; 
        switch(status.MPI_TAG){
        	case LOAD_REQ:{	
        		log("recibido LOAD_REQ. Iniciando protocolo load.");
        		load(*check, status);
        		break;
        	}

        	case MEMBER_REQ:{
        		log("recibido MEMBER_REQ. Iniciando protocolo member.");
        		string strKey(buffer);
        		member(*check, status, strKey);
        		break;
        	}

        	case ADD_REQ:{
        		log("recibido ADD_REQ. Iniciando protocolo addAndInc.");
        		string strKey(buffer);
        		addAndInc(*check, status, strKey);
        		break;
        	}

        	case MAXIMUM_REQ:{
        		log("recibido MAXIMUM_REQ. Iniciando protocolo maximum.");
        		maximum(*check, status);
        		break;
        	}
        	case IMPRIMIR_REQ:{
        		log("recibido IMPRIMIR_REQ. Iniciando protocolo imprimir.");
        		imprimir(*check, status);
        		break;
        	}

        	case QUIT:{
        		log("recibido QUIT, saliendo");
        		continuar = false;
        		break;
        	}
        }
        if(continuar) log("esperando nuevo protocolo");
    }

    //terminar
    logFile.close();
}


void load(uint64_t check, MPI_Status status){

	char buffer[BUFFER_SIZE];

	//si llegas hasta aca es porque recibiste LOAD_REQ
	while(true){
		set_check_data(buffer, check);
		trabajarArduamente();
	    MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, LOAD_ACK, MPI_COMM_WORLD);
		log("enviado LOAD_ACK");
		MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		switch(status.MPI_TAG){
			case LOAD_DATA:{
				std::string nombreArchivo(buffer);
				log(std::string("recibido LOAD_DATA, cargando archivo ") + nombreArchivo);
				hashMap.load(nombreArchivo);
				break;
			}

			case LOAD_REL:{
				log("recibido LOAD_REL, terminando protocolo de load");
				return; //se terminaron de cargar los archivos
			}

			default:
				//no hacer nada
				log("recibido mensaje inválido (protocolo actual: load)");
		}
	}
}

void member(uint64_t check, MPI_Status status, const string& key){

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	buffer[0] = hashMap.member(key);
	set_check_data(buffer, check);

	trabajarArduamente();
	MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, MEMBER_DATA, MPI_COMM_WORLD);
	log("enviado MEMBER_DATA");
	log("protocolo de member terminado");
}

void addAndInc(uint64_t check, MPI_Status status, const string& key){

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	set_check_data(buffer, check);
	trabajarArduamente();
	MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, ADD_ACK, MPI_COMM_WORLD);
	log("enviado ADD_ACK");
	do{
		MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}while(!coincide_check_data(buffer, check));

	switch(status.MPI_TAG){

		case ADD_COMMIT:{
			log(string("recibido ADD_COMMIT, agregando clave ") + key);
			hashMap.addAndInc(key);
			break;
		}
		case ADD_ROLLBACK:{
			log("recibido ADD_ROLLBACK");
			break;
		}
		default:
			stringstream cvt;
			cvt << status.MPI_TAG;
			log(string("recibido mensaje inválido (protocolo actual: addAndInc) msg: ") + cvt.str());
			break;
	}

	log("protocolo addAndInc terminado");
}

void maximum(uint64_t check, MPI_Status status){

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	HashMap::iterator it = hashMap.begin();

	trabajarArduamente();
	while(it != hashMap.end()){
		strcpy(buffer, (*it).c_str());
		set_check_data(buffer, check);
		MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, MAXIMUM_DATA, MPI_COMM_WORLD);
		log("enviado MAXIMUM_DATA");
		it++;
	}
	set_check_data(buffer, check);
	MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, MAXIMUM_END, MPI_COMM_WORLD);
	log("enviado MAXIMUM_END");
	log("protocolo maximum terminado");
}

void imprimir(uint64_t check, MPI_Status status){

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	HashMap::iterator it = hashMap.begin();

	trabajarArduamente();
	while(it != hashMap.end()){
		strcpy(buffer, (*it).c_str());
		set_check_data(buffer, check);
		MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, IMPRIMIR_DATA, MPI_COMM_WORLD);
		log("enviado IMPRIMIR_DATA");
		it++;
	}
	set_check_data(buffer, check);
	MPI_Send(buffer, BUFFER_SIZE, MPI_CHAR, CONSOLA, IMPRIMIR_END, MPI_COMM_WORLD);
	log("enviado IMPRIMIR_END");
	log("protocolo imprimir terminado");
}

void trabajarArduamente() {
    int r = rand() % 2500000 + 500000;
    usleep(r);
}
