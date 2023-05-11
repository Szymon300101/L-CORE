#ifndef USER_P_H
#define USER_P_H


#define DEBUG true

#define ADDRESS 2
#define NET_SIZE 3

//3 urządzenia
//int routing_table[NET_SIZE] = {0,1,1}; //dla addr 0
//int routing_table[NET_SIZE] = {0,1,2}; //dla addr 1
//int routing_table[NET_SIZE] = {1,1,2}; //dla addr 2

//2 urządzenia (NET_SIZE = 2, ale addr 1 jest nie używany)
int routing_table[NET_SIZE] = {0,1,2}; //dla obu urządzeń



#endif