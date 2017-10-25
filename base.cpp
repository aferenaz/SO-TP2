
#include "base.h"

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