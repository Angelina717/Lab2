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

            /* ===== [1] Посмотреть на часы ===== */
        case 1:
            printf("Текущее время: День %d, %02d:00\n",
                current_day, current_hour);
            break;

            /* ===== [2] Промотать время (Поработать) ===== */
        case 2: {
            int hours;
            printf("Сколько часов работаем? ");

            if (scanf("%d", &hours) != 1) {
                while ((c = getchar()) != '\n' && c != EOF) {}
                printf("Ошибка ввода!\n");
                break;
            }
            while ((c = getchar()) != '\n' && c != EOF) {}

            if (hours < 0) {
                printf("Часы не могут быть отрицательными.\n");
                break;
            }

            current_hour += hours;

            /* Корректный перевод часов в дни */
            while (current_hour >= 24) {
                current_hour -= 24;
                current_day++;
            }

            printf("Прошло %d ч. Теперь: День %d, %02d:00\n",
                hours, current_day, current_hour);
            break;
        }

              /* ===== [3] Посмотреть инвентарь ===== */
        case 3:
            for (i = 0; i < INVENTORY_SIZE; i++) {
                printf("Слот %d: [%d]", i, inventory[i]);

                switch (inventory[i]) {
                case ITEM_EMPTY:   printf(" (Пусто)");  break;
                case ITEM_WOOD:  printf(" (Дерево)");  break;
                case ITEM_STONE:  printf(" (Камень)");  break;
                case ITEM_SEEDS:   printf(" (Семена)");  break;
                case ITEM_IRON:   printf(" (Железо)");  break;
                case ITEM_GOLD:  printf(" (Серебро)");  break;
                case ITEM_APPLE: printf(" (Яблоко)");   break;
                case ITEM_POTION:   printf(" (Зелье)"); break;
                case ITEM_ROPE:  printf(" (Верёвка)");   break;
                case ITEM_TORCH:  printf(" (Факел)");   break;
                default:          printf(" (?)");       break;
                }

                printf("\n");
            }
            break;

            /* ===== [4] Положить предмет в слот ===== */
        case 4: {
            int slot, id;

            printf("Индекс слота (0-9): ");
            if (scanf("%d", &slot) != 1) {
                while ((c = getchar()) != '\n' && c != EOF) {}
                printf("Ошибка ввода!\n");
                break;
            }
            while ((c = getchar()) != '\n' && c != EOF) {}

            if (slot < 0 || slot >= INVENTORY_SIZE) {
                printf("Индекс вне диапазона!\n");
                break;
            }

            printf("ID предмета: ");
            if (scanf("%d", &id) != 1) {
                while ((c = getchar()) != '\n' && c != EOF) {}
                printf("Ошибка ввода!\n");
                break;
            }
            while ((c = getchar()) != '\n' && c != EOF) {}

            inventory[slot] = id;
            printf("Предмет %d помещён в слот %d.\n", id, slot);
            break;
        }

              /* ===== [5] Выбросить предмет ===== */
        case 5: {
            int slot;

            printf("Индекс слота для удаления (0-9): ");
            if (scanf("%d", &slot) != 1) {
                while ((c = getchar()) != '\n' && c != EOF) {}
                printf("Ошибка ввода!\n");
                break;
            }
            while ((c = getchar()) != '\n' && c != EOF) {}

            if (slot < 0 || slot >= INVENTORY_SIZE) {
                printf("Индекс вне диапазона!\n");
                break;
            }

            inventory[slot] = ITEM_EMPTY;
            printf("Слот %d очищен.\n", slot);
            break;
        }

              /* ===== [6] Очистка от мусора ===== */
        case 6: {
            int id;
            int cleared = 0;

            printf("Введите ID предмета для удаления: ");
            if (scanf("%d", &id) != 1) {
                while ((c = getchar()) != '\n' && c != EOF) {}
                printf("Ошибка ввода!\n");
                break;
            }
            while ((c = getchar()) != '\n' && c != EOF) {}

            /* --- Инвентарь до очистки --- */
            printf("\nИнвентарь до очистки:\n");
            for (i = 0; i < INVENTORY_SIZE; i++) {
                printf("Слот %d: [%d]", i, inventory[i]);

                switch (inventory[i]) {
                case ITEM_EMPTY:   printf(" (Пусто)");  break;
                case ITEM_WOOD:  printf(" (Дерево)");  break;
                case ITEM_STONE:  printf(" (Камень)");  break;
                case ITEM_SEEDS:   printf(" (Семена)");  break;
                case ITEM_IRON:   printf(" (Железо)");  break;
                case ITEM_GOLD:  printf(" (Серебро)");  break;
                case ITEM_APPLE: printf(" (Яблоко)");   break;
                case ITEM_POTION:   printf(" (Зелье)"); break;
                case ITEM_ROPE:  printf(" (Верёвка)");   break;
                case ITEM_TORCH:  printf(" (Факел)");   break;
                default:          printf(" (?)");       break;
                }

                printf("\n");
            }

            /* --- Замена всех вхождений id на 0 --- */
            for (i = 0; i < INVENTORY_SIZE; i++) {
                if (inventory[i] == id && id != ITEM_EMPTY) {
                    inventory[i] = ITEM_EMPTY;
                    cleared++;
                }
            }

            /* --- Инвентарь после очистки --- */
            printf("\nИнвентарь после очистки:\n");
            for (i = 0; i < INVENTORY_SIZE; i++) {
                printf("Слот %d: [%d]", i, inventory[i]);

                switch (inventory[i]) {
                case ITEM_EMPTY:   printf(" (Пусто)");  break;
                case ITEM_WOOD:  printf(" (Дерево)");  break;
                case ITEM_STONE:  printf(" (Камень)");  break;
                case ITEM_SEEDS:   printf(" (Семена)");  break;
                case ITEM_IRON:   printf(" (Железо)");  break;
                case ITEM_GOLD:  printf(" (Серебро)");  break;
                case ITEM_APPLE: printf(" (Яблоко)");   break;
                case ITEM_POTION:   printf(" (Зелье)"); break;
                case ITEM_ROPE:  printf(" (Верёвка)");   break;
                case ITEM_TORCH:  printf(" (Факел)");   break;
                default:          printf(" (?)");       break;
                }

                printf("\n");
            }

            printf("Очищено слотов: %d\n", cleared);
            break;
        }

              /* ===== Неверный пункт ===== */
        default:
            printf("Нет такого пункта меню.\n");
            break;
        }
    }

    return 0;
}
