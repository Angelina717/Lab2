#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

/* Константы */
#define INVENTORY_SIZE   10
#define MAX_ITEMS        10
#define MAX_NAME_LEN     32
#define MAX_FARMER_NAME  33    /* 32 символа + '\0' */
#define MAX_LINE_LEN     256
#define MAX_INGREDIENTS  16

/* ID предметов */
#define ITEM_EMPTY   0
#define ITEM_WOOD    1
#define ITEM_STONE   2
#define ITEM_SEEDS   3
#define ITEM_IRON    4
#define ITEM_GOLD    5
#define ITEM_APPLE   6
#define ITEM_POTION  7
#define ITEM_ROPE    8
#define ITEM_TORCH   9

/* Имена файлов */
#define ITEMS_FILE   "items.txt"
#define INPUT_FILE   "input.txt"
#define OUTPUT_FILE  "output.txt"
#define DIARY_FILE   "diary.txt"

/*  Глобальные данные  */
char item_names[MAX_ITEMS][MAX_NAME_LEN];
char farmer_name[MAX_FARMER_NAME] = "Фермер";
int  current_day = 1;
int  current_hour = 8;
int inventory[INVENTORY_SIZE] = {
    ITEM_EMPTY, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS,
    ITEM_IRON,  ITEM_GOLD, ITEM_APPLE, ITEM_POTION,
    ITEM_ROPE,  ITEM_TORCH
};

/*  защита от «дурака»  */

static void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static int read_int(const char* prompt, int* out)
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

static void read_line(const char* prompt, char* buf, size_t size)
{
    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
}

/*  Имя фермера  */

void ask_farmer_name(void)
{
    char buf[MAX_FARMER_NAME];
    read_line("Введите имя фермера (до 32 символов): ", buf, sizeof(buf));
    if (strlen(buf) == 0) {
        printf("Имя не введено, оставляем \"%s\".\n", farmer_name);
        return;
    }
    strncpy(farmer_name, buf, MAX_FARMER_NAME - 1);
    farmer_name[MAX_FARMER_NAME - 1] = '\0';
    printf("Привет, %s!\n", farmer_name);
}

/*  Каталог предметов из items.txt  */

void load_default_items(void)
{
    const char* defs[MAX_ITEMS] = {
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
    load_default_items();  /* сразу заполняем значениями по умолчанию */
    return;
    FILE* f = fopen(ITEMS_FILE, "r");
    if (f == NULL) {
        printf("[!] Файл %s не найден. Используем названия по умолчанию.\n",
            ITEMS_FILE);
        return;
    }

    char line[MAX_LINE_LEN];
    int  idx = 0;

    while (idx < MAX_ITEMS && fgets(line, sizeof(line), f) != NULL) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;

        strncpy(item_names[idx], line, MAX_NAME_LEN - 1);
        item_names[idx][MAX_NAME_LEN - 1] = '\0';
        idx++;
    }
    fclose(f);

    printf("Загружено названий из %s: %d.\n", ITEMS_FILE, idx);
    if (idx < MAX_ITEMS) {
        printf("[!] Строк меньше %d — остальные взяты по умолчанию.\n",
            MAX_ITEMS);
    }
}

const char* get_item_name(int id)
{
    if (id < 0 || id >= MAX_ITEMS) return "?";
    return item_names[id];
}

/*  Вывод инвентаря с названиями */

void print_inventory(void)
{
    printf("\n--- Инвентарь ---\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        printf("Слот %d: [%d] - %s\n",
            i, inventory[i], get_item_name(inventory[i]));
    }
}

/*  Часы  */

void show_clock(void)
{
    printf("Текущее время: День %d, %02d:00\n", current_day, current_hour);
}

void pass_time(void)
{
    int hours;
    if (!read_int("Сколько часов работаем? ", &hours)) return;
    if (hours < 0) {
        printf("Часы не могут быть отрицательными.\n");
        return;
    }
    current_hour += hours;
    while (current_hour >= 24) {
        current_hour -= 24;
        current_day++;
    }
    printf("Прошло %d ч. Теперь: День %d, %02d:00\n",
        hours, current_day, current_hour);
}

/* Работа с инвентарём */
/* Положить предмет в слот*/
void put_item(void)
{
    int slot, id;
    if (!read_int("Индекс слота (0-9): ", &slot)) return;
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Индекс вне диапазона!\n");
        return;
    }
    if (!read_int("ID предмета: ", &id)) return;
    if (id < 0 || id >= MAX_ITEMS) {
        printf("Недопустимый ID!\n");
        return;
    }
    inventory[slot] = id;
    printf("Предмет \"%s\" помещён в слот %d.\n", get_item_name(id), slot);
}
/* Выбросить предмет*/
void drop_item(void)
{
    int slot;
    if (!read_int("Индекс слота для удаления (0-9): ", &slot)) return;
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Индекс вне диапазона!\n");
        return;
    }
    inventory[slot] = ITEM_EMPTY;
    printf("Слот %d очищен.\n", slot);
}

}
/* Очистка мусора */
void clear_trash(void)
{
    int id;
    if (!read_int("Введите ID предмета для удаления: ", &id)) return;

    printf("\nИнвентарь до очистки:\n");
    print_inventory();

    int cleared = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == id && id != ITEM_EMPTY) {
            inventory[i] = ITEM_EMPTY;
            cleared++;
        }
    }

    printf("\nИнвентарь после очистки:\n");
    print_inventory();
    printf("Очищено слотов: %d\n", cleared);
}
int main(void)
{
    setlocale(0, ""); // чтобы буквы нормальные были
    int current_day = 1;
    int current_hour = 8;
    /* Статический массив инвентаря на 10 элементов */
    int inventory[INVENTORY_SIZE] = {
        ITEM_EMPTY,  ITEM_WOOD, ITEM_STONE, ITEM_SEEDS,
        ITEM_IRON,  ITEM_GOLD, ITEM_APPLE, ITEM_POTION,
        ITEM_ROPE, ITEM_TORCH
    };

    int choice = -1;
    int i;          /* счётчик циклов */
    int c;          /* для очистки потока ввода */
    
    while (choice != 0) {
    
        /* --- Меню --- */
        printf("\n=== Меню ===\n");
        printf("[0] Выход\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Промотать время (Поработать)\n");
        printf("[3] Посмотреть инвентарь\n");
        printf("[4] Положить предмет в слот\n");
        printf("[5] Выбросить предмет\n");
        printf("[6] Очистка от мусора\n");
        printf("Выберите пункт: ");
        switch (choice) {

            /* ===== [0] Выход ===== */
        case 0:
            printf("Выход из игры. Пока!\n");
            break;
      
              /* ===== Неверный пункт ===== */
        default:
            printf("Нет такого пункта меню.\n");
            break;
        }
    }

    return 0;
}
