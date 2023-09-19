#ifndef USER_P_H
#define USER_P_H

//ustawienia konkretnego urządzenia. ustawić według życzenia przed wgraniem kodu na esp32

#define DEBUG true

// adres urządzenia.
// 0 - urządzenie wysyłające
// 1 - urządzenie przekazujące (opcjonalne)
// 2 - urządzenie odbierające (Modbus)
#define ADDRESS 2


//wielkość sieci (nie zmieniać!)
#define NET_SIZE 4


//zostawić odkomentowane gdy używane są 2 urządzenia (NET_SIZE = 3, ale addr '1' jest nie używany)
int routing_table[NET_SIZE] = {0,1,2,3}; //dla obu urządzeń

//odkomentować odpowiednią linię gdy używane są 3 urządzenia
//int routing_table[NET_SIZE] = {0,1,1}; //dla addr 0
//int routing_table[NET_SIZE] = {0,1,2}; //dla addr 1
//int routing_table[NET_SIZE] = {1,1,2}; //dla addr 2


#endif