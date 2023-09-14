#ifndef USER_P_H
#define USER_P_H

//ustawienia konkretnego urządzenia. ustawić według życzenia przed wgraniem kodu na esp32

#define DEBUG true

#define ADDRESS 1


#define NET_SIZE 2


int routing_table[NET_SIZE] = {0,1}; //dla obu urządzeń

//odkomentować odpowiednią linię gdy używane są 3 urządzenia
//int routing_table[NET_SIZE] = {0,1,1}; //dla addr 0
//int routing_table[NET_SIZE] = {0,1,2}; //dla addr 1
//int routing_table[NET_SIZE] = {1,1,2}; //dla addr 2




#endif