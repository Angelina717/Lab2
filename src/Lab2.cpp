#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>

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

/* Карта и приключение */
#define MAP_MIN_SIZE       5
#define MAP_MAX_SIZE       30
#define OBSTACLE_PERCENT_MIN 15
#define OBSTACLE_PERCENT_MAX 30
#define FOG_RADIUS         4      /* радиус видимости вокруг героя */

/* Символы карты */
#define CH_WALL      '#'
#define CH_FLOOR     '.'
#define CH_TREE      'T'
#define CH_ROCK      'R'
#define CH_HERO      '@'
#define CH_FOG       '?'
#define CH_STAIR_UP  'U'  

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
/* Карта */

 /* Структура карты: хранит размеры и сам двумерный динамический массив */
typedef struct {
    char** cells;   /* cells[y][x] — символ клетки */
    int    w;       /* ширина  (столбцы) */
    int    h;       /* высота  (строки)  */
} Map;

/*  Выделение динамической карты  */
static char** alloc_map(int h, int w)
{
    char** m = (char**)malloc((size_t)h * sizeof(char*));
    if (m == NULL) {
        printf("Ошибка выделения памяти под строки карты!\n");
        return NULL;
    }
    for (int i = 0; i < h; i++) {
        m[i] = (char*)malloc((size_t)w * sizeof(char));
        if (m[i] == NULL) {
            printf("Ошибка выделения памяти под строку %d!\n", i);
            for (int j = 0; j < i; j++) free(m[j]);
            free(m);
            return NULL;
        }
    }
    return m;
}

/* Освобождение карты (в обратном порядке!)  */
static void free_map(Map* map)
{
    if (map == NULL || map->cells == NULL) return;
    for (int i = 0; i < map->h; i++) {
        free(map->cells[i]);
    }
    free(map->cells);
    map->cells = NULL;
    map->w = 0;
    map->h = 0;
}

/*  Генерация карты с туманом войны  */
static int generate_map(Map* map)
{
    int h = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    int w = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);

    char** cells = alloc_map(h, w);
    if (cells == NULL) return 0;

    map->cells = cells;
    map->h = h;
    map->w = w;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (y == 0 || y == h - 1 || x == 0 || x == w - 1)
                cells[y][x] = CH_WALL;
            else
                cells[y][x] = CH_FLOOR;
        }
    }

    int percent = OBSTACLE_PERCENT_MIN +
        rand() % (OBSTACLE_PERCENT_MAX - OBSTACLE_PERCENT_MIN + 1);

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            if (rand() % 100 < percent) {
                cells[y][x] = (rand() % 2 == 0) ? CH_TREE : CH_ROCK;
            }
        }
    }
    return 1;
}

/*  Поиск свободной клетки для героя  */
static int find_hero_spawn(const Map* map, int* hx, int* hy)
{
    int total = 0;
    for (int y = 1; y < map->h - 1; y++)
        for (int x = 1; x < map->w - 1; x++)
            if (map->cells[y][x] == CH_FLOOR) total++;

    if (total == 0) return 0;

    int pick = rand() % total;
    for (int y = 1; y < map->h - 1; y++) {
        for (int x = 1; x < map->w - 1; x++) {
            if (map->cells[y][x] == CH_FLOOR) {
                if (pick == 0) { *hx = x; *hy = y; return 1; }
                pick--;
            }
        }
    }
    return 0;
}

/*  Размещение лестницы U рядом с героем  */
static void place_stair_up(Map* map, int hx, int hy)
{
    /* Проверяем соседние 8 клеток */
    int dx[] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    int dy[] = { -1, -1, 0, 1, 1, 1, 0, -1 };

    int valid_cells[8][2];
    int count = 0;

    for (int i = 0; i < 8; i++) {
        int nx = hx + dx[i];
        int ny = hy + dy[i];
        if (nx > 0 && nx < map->w - 1 && ny > 0 && ny < map->h - 1) {
            if (map->cells[ny][nx] == CH_FLOOR) {
                valid_cells[count][0] = nx;
                valid_cells[count][1] = ny;
                count++;
            }
        }
    }

    if (count > 0) {
        int pick = rand() % count;
        map->cells[valid_cells[pick][1]][valid_cells[pick][0]] = CH_STAIR_UP;
    }
    else {
        /* Если вокруг всё занято (редкость), ищем любую свободную клетку */
        for (int y = 1; y < map->h - 1; y++) {
            for (int x = 1; x < map->w - 1; x++) {
                if (map->cells[y][x] == CH_FLOOR) {
                    map->cells[y][x] = CH_STAIR_UP;
                    return;
                }
            }
        }
    }
}

/*  В пределах ли видимости (туман) */
static int is_visible(int hx, int hy, int x, int y)
{
    int dx = x - hx;
    int dy = y - hy;
    return (dx * dx + dy * dy) <= (FOG_RADIUS * FOG_RADIUS);
}

/* Отрисовка карты с учётом тумана  */
static void print_map_with_fog(const Map* map, int hx, int hy)
{
    printf("\n");
    for (int y = 0; y < map->h; y++) {
        for (int x = 0; x < map->w; x++) {
            if (x == hx && y == hy) {
                putchar(CH_HERO);
            }
            else if (is_visible(hx, hy, x, y)) {
                putchar(map->cells[y][x]);
            }
            else {
                putchar(CH_FOG);
            }
        }
        putchar('\n');
    }
    printf("\n");
}
/* Попытка перемещения героя  */
static int try_move(const Map* map, int* hx, int* hy, int dx, int dy)
{
    int nx = *hx + dx;
    int ny = *hy + dy;

    if (nx < 0 || nx >= map->w || ny < 0 || ny >= map->h) {
        printf("Туда нельзя: край карты.\n");
        return 0;
    }

    char c = map->cells[ny][nx];
    if (c == CH_WALL) {
        printf("Туда нельзя: стена.\n");
        return 0;
    }
    if (c == CH_TREE) {
        printf("Туда нельзя: дерево.\n");
        return 0;
    }
    if (c == CH_ROCK) {
        printf("Туда нельзя: камень.\n");
        return 0;
    }

    *hx = nx;
    *hy = ny;
    return 1;
}

/* Основной цикл приключения  */
void adventure_mode(void)
{
    Map map = { NULL, 0, 0 };

    if (!generate_map(&map)) {
        printf("Не удалось сгенерировать карту.\n");
        return;
    }

    int hx = 0, hy = 0;
    if (!find_hero_spawn(&map, &hx, &hy)) {
        printf("На карте нет свободного места для героя. Освобождаем память.\n");
        free_map(&map);
        return;
    }
    place_stair_up(&map, hx, hy);

    printf("\n=== Приключение началось! (Туман войны) ===\n");
    printf("Карта %dx%d, радиус видимости %d.\n", map.w, map.h, FOG_RADIUS);
    printf("Управление: W/A/S/D - ходьба, Q - домой (только на лестнице U).\n");
    printf("Лестница U находится рядом с точкой старта.\n");

    int running = 1;
    while (running) {
        print_map_with_fog(&map, hx, hy);
        printf("Герой (%d,%d). Ваш ход: ", hx, hy);

        char cmd[8];
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            clear_input();
            continue;
        }

        size_t len = strlen(cmd);
        while (len > 0 && (cmd[len - 1] == '\n' || cmd[len - 1] == '\r')) {
            cmd[--len] = '\0';
        }
        if (len != 1) {
            printf("Введите одну букву: W/A/S/D/Q.\n");
            continue;
        }

        char c = cmd[0];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');

        switch (c) {
        case 'W': try_move(&map, &hx, &hy, 0, -1); break;
        case 'A': try_move(&map, &hx, &hy, -1, 0); break;
        case 'S': try_move(&map, &hx, &hy, 0, +1); break;
        case 'D': try_move(&map, &hx, &hy, +1, 0); break;
        case 'Q':
            if (map.cells[hy][hx] == CH_STAIR_UP) {
                printf("Вы поднялись по лестнице и благополучно вернулись домой.\n");
                running = 0;
            }
            else {
                printf("Вы можете вернуться домой только стоя на лестнице вверх (U).\n");
            }
            break;
        default:
            printf("Неизвестная команда. Используйте W/A/S/D/Q.\n");
            break;
        }
    }

    /* ОБЯЗАТЕЛЬНО освобождаем память */
    free_map(&map);
    printf("Память карты освобождена. Возврат в меню.\n");
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
