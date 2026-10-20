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

/*  Поиск предмета по названию */

void find_item_by_name(void)
{
    char query[MAX_NAME_LEN];
    read_line("Введите название предмета: ", query, sizeof(query));

    if (strlen(query) == 0) {
        printf("Пустой запрос.\n");
        return;
    }

    int found_id = -1;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(item_names[i], query) == 0) {
            found_id = i;
            break;
        }
    }

    if (found_id == -1) {
        printf("Предмет \"%s\" не найден в каталоге.\n", query);
        return;
    }

    printf("ID предмета \"%s\" = %d. Ищем в рюкзаке...\n", query, found_id);

    int found_any = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == found_id) {
            printf("  Найдено в слоте %d\n", i);
            found_any = 1;
        }
    }

    if (!found_any) {
        printf("В рюкзаке такого предмета нет.\n");
    }
}

/*  Дневник фермера  */

void write_diary(void)
{
    FILE* f = fopen(DIARY_FILE, "a");
    if (f == NULL) {
        printf("Не удалось открыть %s для записи!\n", DIARY_FILE);
        return;
    }
     /* вывод на экран*/
     printf("\n=== Запись дневника ===\n");
     printf("Фермер: %s\n", farmer_name);
     printf("День %d, %02d:00\n", current_day, current_hour);
     printf("Инвентарь:\n");
     for (int i = 0; i < INVENTORY_SIZE; i++) {
         printf("  Слот %d: [%d] - %s\n",
             i, inventory[i], get_item_name(inventory[i]));
     }
     printf("\n");
     /* запись в файл */
    fprintf(f, "=== Запись дневника ===\n");
    fprintf(f, "Фермер: %s\n", farmer_name);
    fprintf(f, "День %d, %02d:00\n", current_day, current_hour);
    fprintf(f, "Инвентарь:\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        fprintf(f, "  Слот %d: [%d] - %s\n",
            i, inventory[i], get_item_name(inventory[i]));
    }
    fprintf(f, "\n");

    fclose(f);
    printf("Состояние записано в %s.\n", DIARY_FILE);
}

/* Проверка рецептов крафта  */

typedef struct {
    char name[MAX_NAME_LEN];
    int  ingredients[MAX_INGREDIENTS];
    int  ing_count;
} Recipe;

/* Сколько раз предмет с данным ID лежит в инвентаре */
static int count_in_inventory(int id)
{
    int cnt = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == id) cnt++;
    }
    return cnt;
}

/*  строки вида "Название: ID1 ID2 ID3".
   Возвращает 1 при успехе, 0 при ошибке формата. */
static int parse_recipe_line(const char* line, Recipe* r)
{
    r->ing_count = 0;
    r->name[0] = '\0';

    const char* colon = strchr(line, ':');
    if (colon == NULL) return 0;

    size_t name_len = (size_t)(colon - line);
    if (name_len == 0 || name_len >= MAX_NAME_LEN) return 0;

    strncpy(r->name, line, name_len);
    r->name[name_len] = '\0';

    /* Обрезаем пробелы справа в имени */
    while (name_len > 0 && r->name[name_len - 1] == ' ') {
        r->name[--name_len] = '\0';
    }

    /* Разбор компонентов */
    const char* p = colon + 1;
    while (*p != '\0') {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        if (*p < '0' || *p > '9') return 0;

        int val = 0;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            p++;
        }

        if (r->ing_count >= MAX_INGREDIENTS) return 0;
        r->ingredients[r->ing_count++] = val;
    }

    if (r->ing_count == 0) return 0;
    return 1;
}

void check_craft_recipes(void)
{
    FILE* fin = fopen(INPUT_FILE, "r");
    if (fin == NULL) {
        printf("[!] Файл %s не найден. Проверка рецептов невозможна.\n",
            INPUT_FILE);
        return;
    }

    FILE* fout = fopen(OUTPUT_FILE, "w");
    if (fout == NULL) {
        printf("[!] Не удалось открыть %s для записи.\n", OUTPUT_FILE);
        fclose(fin);
        return;
    }

    printf("\n Проверка рецептов крафта \n");
    fprintf(fout, " Проверка рецептов крафта \n");
    fprintf(fout, "Фермер: %s | День %d, %02d:00\n\n",
        farmer_name, current_day, current_hour);

    char line[MAX_LINE_LEN];
    int  line_no = 0;
    int  total = 0, available = 0, not_enough = 0;

    while (fgets(line, sizeof(line), fin) != NULL) {
        line_no++;

        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;         /* пустые строки пропускаем */
        if (line[0] == '#') continue;   /* комментарии пропускаем */

        Recipe r;
        if (!parse_recipe_line(line, &r)) {
            printf("Строка %d: некорректный формат, пропущено.\n", line_no);
            fprintf(fout, "Строка %d: [ОШИБКА ФОРМАТА] %s\n", line_no, line);
            continue;
        }

        total++;

        /* Сколько каждого ID требуется */
        int need[MAX_ITEMS] = { 0 };
        for (int i = 0; i < r.ing_count; i++) {
            int id = r.ingredients[i];
            if (id < 0 || id >= MAX_ITEMS) continue;
            need[id]++;
        }

        /* Что и в каком количестве не хватает */
        int missing_ids[MAX_ITEMS];
        int missing_cnt[MAX_ITEMS];
        int missing_total = 0;

        for (int id = 0; id < MAX_ITEMS; id++) {
            if (need[id] == 0) continue;
            int have = count_in_inventory(id);
            if (have < need[id]) {
                missing_ids[missing_total] = id;
                missing_cnt[missing_total] = need[id] - have;
                missing_total++;
            }
        }

        /* Формируем вердикт */
        char verdict[MAX_LINE_LEN];
        if (missing_total == 0) {
            available++;
            snprintf(verdict, sizeof(verdict), "[ДОСТУПНО]");
        }
        else {
            not_enough++;
            int pos = snprintf(verdict, sizeof(verdict),
                "[НЕ ХВАТАЕТ РЕСУРСОВ]");
            for (int i = 0; i < missing_total && pos < (int)sizeof(verdict); i++) {
                pos += snprintf(verdict + pos, sizeof(verdict) - pos,
                    " %s x%d",
                    get_item_name(missing_ids[i]),
                    missing_cnt[i]);
            }
        }

        printf("%s: %s\n", r.name, verdict);
        fprintf(fout, "%s: %s\n", r.name, verdict);
    }

    fprintf(fout, "\nИтого рецептов: %d | Доступно: %d | Не хватает: %d\n",
        total, available, not_enough);
    printf("\nИтого рецептов: %d | Доступно: %d | Не хватает: %d\n",
        total, available, not_enough);
    printf("Результат сохранён в %s.\n", OUTPUT_FILE);

    fclose(fin);
    fclose(fout);
}
/*  Меню  */

void print_menu(void)
{
    printf("\n Меню (Фермер: %s | День %d, %02d:00) \n",
        farmer_name, current_day, current_hour);
    printf("[0] Выход\n");
    printf("[1] Посмотреть на часы\n");
    printf("[2] Промотать время (Поработать)\n");
    printf("[3] Посмотреть инвентарь\n");
    printf("[4] Положить предмет в слот\n");
    printf("[5] Выбросить предмет\n");
    printf("[6] Очистка от мусора\n");
    printf("[7] Поиск предмета в рюкзаке по названию\n");
    printf("[8] Записать состояние в дневник фермера\n");
    printf("[9] Проверить рецепты крафта (вариант 6)\n");
    printf("Выберите пункт: ");
}

int main(void)
{
    setlocale(LC_CTYPE, "rus"); // чтобы буквы нормальные были 
    system("chcp 1251 > nul"); // чтобы буквы нормальные были 
    printf(" Весёлый фермер: лабораторная №3 \n");
    load_items_from_file();
    ask_farmer_name();

    int choice = -1;
    while (choice != 0) {
        print_menu();

        if (scanf("%d", &choice) != 1) {
            clear_input();
            printf("Ошибка ввода! Введите число.\n");
            choice = -1;
            continue;
        }
        clear_input();

        switch (choice) {
        case 0: printf("Выход из игры. Пока, %s!\n", farmer_name); break;
        case 1: show_clock();          break;
        case 2: pass_time();           break;
        case 3: print_inventory();     break;
        case 4: put_item();            break;
        case 5: drop_item();           break;
        case 6: clear_trash();         break;
        case 7: find_item_by_name();   break;
        case 8: write_diary();         break;
        case 9: check_craft_recipes(); break;
        default: printf("Нет такого пункта меню.\n"); break;
        }
    }

    return 0;
}
