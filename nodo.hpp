#ifndef _NODO_H
#define _NODO_H

#define BUFFER_SIZE 1024


#include "mpi.h"
#include "base.h"
#include <iostream>
#include <string>
#include <stdint.h>

/* Función que maneja un nodo.
 * Recibe el rank del nodo.
 */
void nodo(unsigned int rank);

/* Simula un tiempo de procesamiento no determinístico.
 */
void trabajarArduamente();

/* Protocolos */
void load(uint64_t, MPI_Status);
void member(uint64_t, MPI_Status, const std::string&);
void addAndInc(uint64_t, MPI_Status, const std::string&);
void maximum(uint64_t, MPI_Status);
void imprimir(uint64_t, MPI_Status);

#endif  /* _NODO_H */
