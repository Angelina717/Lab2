#ifndef UTILS_H
#define UTILS_H

#include "common.h"

/* Защита от «дурака» */
void clear_input(void);
int  read_int(const char* prompt, int* out);
void read_line(const char* prompt, char* buf, size_t size);

/* Каталог предметов */
void        load_items_from_file(void);
const char* get_item_name(int id);

/* Вспомогательное */
int count_in_inventory(const int* inv, int size, int id);

#endif /* UTILS_H */
