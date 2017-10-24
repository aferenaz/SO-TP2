#ifndef _CONSOLA_H
#define _CONSOLA_H

#define BUFFER_SIZE 1024


#ifndef BASE_H
#define BASE_H

//****Mensajes****//

//load
#define LOAD_REQ		1
#define LOAD_ACK		2
#define LOAD_DATA		3
#define LOAD_REL		4

//member
#define MEMBER_REQ		5
#define MEMBER_DATA 	6

//addAndInc
#define ADD_REQ			7
#define ADD_ACK			8
#define ADD_COMMIT		9
#define ADD_ROLLBACK	10


//quit
#define QUIT			11

//****Fin Mensajes****//

//Ranks
#define CONSOLA		0


//otros
#include <stdint.h>
#define BUFFER_SIZE 1024
#define CHECK_DATA	BUFFER_SIZE - sizeof(uint64_t)

//logs
#include <ctime>
#include <fstream>
#include <iostream>
extern std::ofstream logFile;
void log(const std::string&);

//checking
#include <stdint.h>
extern uint64_t np_count;
bool coincide_check_data(const char*, uint64_t);
void set_check_data(char*, uint64_t);
uint64_t set_new_check_data(char*);

#endif





/* Función que maneja la consola.
 * Recibe la cantidad total de nodos.
 */
void consola(unsigned int np);

#endif  /* _CONSOLA_H */
