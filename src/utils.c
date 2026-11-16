#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Каталог предметов — единственный массив данных модуля */
static char item_names[MAX_ITEMS][MAX_NAME_LEN];

/*  Защита от «дурака» */
void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int read_int(const char* prompt, int* out)
{
    printf("%s", prompt);
    if (scanf("%d", out) != 1) {
        clear_input();
        printf("Ошибка ввода! Введите число.\n");
        return 0;
    }
    clear_input();
    return 1;
}

void read_line(const char* prompt, char* buf, size_t size)
{
    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';
}

int count_in_inventory(const int* inv, int size, int id)
{
    int cnt = 0;
    for (int i = 0; i < size; i++)
        if (inv[i] == id) cnt++;
    return cnt;
}

/* Каталог предметов */
static void load_default_items(void)
{
    static const char* defs[MAX_ITEMS] = {
        "Пусто", "Дерево", "Камень", "Семена", "Железо",
        "Золото", "Яблоко", "Зелье", "Верёвка", "Факел"
    };
    for (int i = 0; i < MAX_ITEMS; i++) {
        strncpy(item_names[i], defs[i], MAX_NAME_LEN - 1);
        item_names[i][MAX_NAME_LEN - 1] = '\0';
    }
}

void load_items_from_file(void)
{
    load_default_items();

    FILE* f = fopen(ITEMS_FILE, "r");
    if (f == NULL) {
        printf("[!] Файл %s не найден. Используем названия по умолчанию.\n",
            ITEMS_FILE);
        return;
    }
    char line[MAX_LINE_LEN];
    int idx = 0;
    while (idx < MAX_ITEMS && fgets(line, sizeof(line), f) != NULL) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0) continue;
        strncpy(item_names[idx], line, MAX_NAME_LEN - 1);
        item_names[idx][MAX_NAME_LEN - 1] = '\0';
        idx++;
    }
    fclose(f);
    printf("Загружено названий из %s: %d.\n", ITEMS_FILE, idx);
}

const char* get_item_name(int id)
{
    if (id < 0 || id >= MAX_ITEMS) return "?";
    return item_names[id];
}
