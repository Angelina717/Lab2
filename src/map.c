#define _CRT_SECURE_NO_WARNINGS
#include "map.h"
#include "player.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ВНУТРЕННИЙ МОДУЛЬ КАРТЫ
* Все функции static — снаружи не видны.* /

/* освобождение памяти */

static char** alloc_cells(int h, int w)
{
    char** c = (char**)malloc((size_t)h * sizeof(char*));
    if (!c) { printf("Ошибка выделения памяти под строки карты!\n"); return NULL; }
    for (int i = 0; i < h; i++) {
        c[i] = (char*)malloc((size_t)w * sizeof(char));
        if (!c[i]) {
            printf("Ошибка выделения памяти под строку %d!\n", i);
            for (int j = 0; j < i; j++) free(c[j]);
            free(c);
            return NULL;
        }
    }
    return c;
}

static Map* map_create(int floor_num)
{
    Map* m = (Map*)malloc(sizeof(Map));
    if (!m) {
        printf("[!] Не удалось выделить память под структуру карты.\n");
        return NULL;
    }
    m->cells = NULL;
    m->w = m->h = 0;
    m->floor_num = floor_num;
    return m;
}

static void map_destroy(Map** m)
{
    if (!m || !*m) return;
    if ((*m)->cells) {
        for (int i = 0; i < (*m)->h; i++) free((*m)->cells[i]);
        free((*m)->cells);
    }
    printf("Память структуры по адресу %p освобождена\n", (void*)(*m));
    free(*m);
    *m = NULL;
}

/* Генерация карты */

static int map_generate(Map* m)
{
    if (!m) return 0;
    if (m->cells) {
        for (int i = 0; i < m->h; i++) free(m->cells[i]);
        free(m->cells);
        m->cells = NULL;
    }

    int h = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    int w = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);

    char** c = alloc_cells(h, w);
    if (!c) return 0;

    m->cells = c;
    m->h = h;
    m->w = w;

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            c[y][x] = (y == 0 || y == h - 1 || x == 0 || x == w - 1) ? CH_WALL : CH_FLOOR;
    /*  случайно выбираем плотность препятствий */
    int percent = OBSTACLE_PERCENT_MIN +
        rand() % (OBSTACLE_PERCENT_MAX - OBSTACLE_PERCENT_MIN + 1);

    for (int y = 1; y < h - 1; y++)
        for (int x = 1; x < w - 1; x++)
            if (rand() % 100 < percent)
                c[y][x] = (rand() % 2 == 0) ? CH_TREE : CH_ROCK;

    return 1;
}

/*  Поиск свободной клетки для героя  */
static int map_find_spawn(const Map* m, int* hx, int* hy)
{
    if (!m) return 0;
    int total = 0;
    for (int y = 1; y < m->h - 1; y++)
        for (int x = 1; x < m->w - 1; x++)
            if (m->cells[y][x] == CH_FLOOR) total++;

    if (!total) return 0;

    int pick = rand() % total;
    for (int y = 1; y < m->h - 1; y++) {
        for (int x = 1; x < m->w - 1; x++) {
            if (m->cells[y][x] == CH_FLOOR) {
                if (pick == 0) { *hx = x; *hy = y; return 1; }
                pick--;
            }
        }
    }
    return 0;
}

/*  Размещение лестницы U рядом с героем */
static void map_place_stair_up(Map* m, int hx, int hy)
{
    if (!m) return;
    int dx[] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    int dy[] = { -1, -1, 0, 1, 1, 1, 0, -1 };
    int vx[8], vy[8], cnt = 0;

    /* вычисляем координаты соседней клетки */
    for (int i = 0; i < 8; i++) {
        int nx = hx + dx[i], ny = hy + dy[i];
        if (nx > 0 && nx < m->w - 1 && ny > 0 && ny < m->h - 1 &&
            m->cells[ny][nx] == CH_FLOOR) {
            vx[cnt] = nx; vy[cnt] = ny; cnt++;
        }
    }
    if (cnt) m->cells[vy[rand() % cnt]][vx[rand() % cnt]] = CH_STAIR_UP;
    else {
        for (int y = 1; y < m->h - 1; y++)
            for (int x = 1; x < m->w - 1; x++)
                if (m->cells[y][x] == CH_FLOOR) {
                    m->cells[y][x] = CH_STAIR_UP;
                    return;
                }
    }
}

/*  Размещение лестницы D (вниз) в случайном месте  */
static void map_place_stair_down(Map* m, int hx, int hy)
{
    if (!m) return;
    int total = 0;
    for (int y = 1; y < m->h - 1; y++)
        for (int x = 1; x < m->w - 1; x++)
            /* Ищем пол, но не там, где стоит герой (чтобы D не появилась под ним) */
            if (m->cells[y][x] == CH_FLOOR && !(x == hx && y == hy)) total++;
    if (!total) return;

    int pick = rand() % total;
    for (int y = 1; y < m->h - 1; y++) {
        for (int x = 1; x < m->w - 1; x++) {
            if (m->cells[y][x] == CH_FLOOR && !(x == hx && y == hy)) {
                if (pick == 0) { m->cells[y][x] = CH_STAIR_DOWN; return; }
                pick--;
            }
        }
    }
}

/* Туман и вывод  */
/*  В пределах ли видимости (туман), теорема Пифагора*/
static int map_is_visible(int hx, int hy, int x, int y)
{
    int dx = x - hx, dy = y - hy;
    return (dx * dx + dy * dy) <= (FOG_RADIUS * FOG_RADIUS);
}

/*  Отрисовка карты (с туманом или без) */
static void map_print_with_fog(const Map* m, int hx, int hy, int use_fog)
{
    if (!m) return;
    printf("\n");
    for (int y = 0; y < m->h; y++) {
        for (int x = 0; x < m->w; x++) {
            if (x == hx && y == hy) putchar(CH_HERO);
            else if (!use_fog || map_is_visible(hx, hy, x, y)) putchar(m->cells[y][x]);  /* Если туман отключен (!use_fog) ИЛИ клетка в радиусе видимости */
            else putchar(CH_FOG);
        }
        putchar('\n');
    }
    printf("\n");
}

/* Перемещение героя */

static int map_try_move(const Map* m, int* hx, int* hy, int dx, int dy)
{
    if (!m) return 0;
    int nx = *hx + dx, ny = *hy + dy;
    if (nx < 0 || nx >= m->w || ny < 0 || ny >= m->h) {
        printf("Туда нельзя: край карты.\n");
        return 0;
    }
    char c = m->cells[ny][nx];
    if (c == CH_WALL) { printf("Туда нельзя: стена.\n"); return 0; }
    if (c == CH_TREE) { printf("Туда нельзя: дерево.\n"); return 0; }
    if (c == CH_ROCK) { printf("Туда нельзя: камень.\n"); return 0; }

    *hx = nx;
    *hy = ny;
    return 1;
}

/* HUD приключения */

static void print_hud(const Player* p, int floor_num, int day, int hour)
{
    printf("HP: %d/%d | Выносливость: %d/%d | Золото: %d | Этаж: %d | Время: %02d:00 (День %d)\n",
        p->hp, p->max_hp, p->stamina, p->max_stamina, p->gold,
        floor_num, hour, day);
    if (p->buff.type != STATUS_NONE)
        printf("Статус: %s (%d шагов)\n", status_name(p->buff.type), p->buff.turns);
}

/* Экстренная эвакуация */
static void emergency_evacuate(Player* p)
{
    int lost = p->gold / EMERGENCY_GOLD_LOSS_DIV;
    p->gold -= lost;
    printf("\n>>> ЭКСТРЕННАЯ ЭВАКУАЦИЯ! Спасатели забрали %d золота. <<<\n", lost);
    p->hp = p->max_hp;
    p->stamina = p->max_stamina / 2;
    if (p->stamina < 1) p->stamina = 1;
    p->buff.type = STATUS_NONE;
    p->buff.turns = 0;
}

/* Режим приключения */

void adventure_mode(Player* p, int* day, int* hour)
{
    if (!p) { printf("[!] Сначала создайте персонажа.\n"); return; }
    if (p->stamina <= 0) {
        printf("[!] Выносливость равна 0 — нельзя отправиться в приключение.\n");
        return;
    }
    if (*hour == 2) {
        printf("[!] Сейчас 02:00 ночи — фермер должен спать.\n");
        return;
    }

    Map* m = map_create(1);
    if (!m) return;
    if (!map_generate(m)) { map_destroy(&m); return; }

    int hx, hy;
    if (!map_find_spawn(m, &hx, &hy)) {
        printf("На карте нет свободного места для героя.\n");
        map_destroy(&m);
        return;
    }
    map_place_stair_up(m, hx, hy);
    map_place_stair_down(m, hx, hy);
    p->x = hx; p->y = hy;

    printf("\n=== Приключение началось! (Этаж 1 - Без тумана) ===\n");
    printf("Карта %dx%d.\n", m->w, m->h);
    printf("W/A/S/D — движение, D-лестница вниз, Q — выход (на U).\n");

    int running = 1;
    while (running) {
        int use_fog = (m->floor_num > 1);
        map_print_with_fog(m, hx, hy, use_fog);
        print_hud(p, m->floor_num, *day, *hour);
        printf("Ваш ход: ");

        char cmd[8];
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) { clear_input(); continue; }
        size_t len = strlen(cmd);
        while (len > 0 && (cmd[len - 1] == '\n' || cmd[len - 1] == '\r'))
            cmd[--len] = '\0';
        if (len != 1) { printf("Введите одну букву: W/A/S/D/Q.\n"); continue; }

        char c = cmd[0];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');

        if (c == 'Q') {
            if (m->cells[hy][hx] == CH_STAIR_UP) {
                printf("Вы поднялись по лестнице и вернулись домой.\n");
                running = 0;
            }
            else {
                printf("Выход домой доступен только на лестнице вверх (U).\n");
            }
            continue;
        }

        int moved = 0;
        switch (c) {
        case 'W': moved = map_try_move(m, &hx, &hy, 0, -1); break;
        case 'A': moved = map_try_move(m, &hx, &hy, -1, 0); break;
        case 'S': moved = map_try_move(m, &hx, &hy, 0, +1); break;
        case 'D': moved = map_try_move(m, &hx, &hy, +1, 0); break;
        default:  printf("Неизвестная команда. Используйте W/A/S/D/Q.\n"); continue;
        }

        if (!moved) continue;

        /* Вариант 3: применяем статусы */
        int used = 1;
        player_apply_status_step(p, &used);
        p->stamina -= used;
        if (p->stamina < 0) p->stamina = 0;

        player_try_inflict_status(p);

        /* Проверка падения: HP=0 или выносливость=0 */
        if (p->hp <= 0 || p->stamina <= 0) {
            printf("\n>>> Герой падает без сил! <<<\n");
            emergency_evacuate(p);
            running = 0;
            break;
        }

        /* Спуск на новый этаж */
        if (m->cells[hy][hx] == CH_STAIR_DOWN) {
            printf("\n>>> Вы спускаетесь на этаж ниже... <<<\n");

            if (p->stamina < STAIR_STAMINA_COST) {
                printf("Не хватает выносливости на спуск (нужно %d, есть %d).\n",
                    STAIR_STAMINA_COST, p->stamina);
                continue;
            }
            p->stamina -= STAIR_STAMINA_COST;
            *hour += 1;
            if (*hour >= 24) { *hour = 0; *day += 1; }

            int new_floor = m->floor_num + 1;
            int reward = FLOOR_GOLD_BASE * new_floor;
            p->gold += reward;
            printf("Награда за этаж %d: +%d золота.\n", new_floor, reward);

            if (*hour == 2) {
                printf(">>> 02:00 ночи — герой засыпает на ходу! <<<\n");
                emergency_evacuate(p);
                running = 0;
                break;
            }

            map_destroy(&m);
            m = map_create(new_floor);
            if (!m || !map_generate(m)) {
                if (m) map_destroy(&m);
                running = 0;
                break;
            }
            if (!map_find_spawn(m, &hx, &hy)) {
                printf("Ошибка спавна героя на новом этаже.\n");
                map_destroy(&m);
                running = 0;
                break;
            }
            map_place_stair_up(m, hx, hy);
            map_place_stair_down(m, hx, hy);
            p->x = hx; p->y = hy;

            if (new_floor == 2)
                printf("\n=== ВНИМАНИЕ: темно (туман войны). Радиус %d. ===\n", FOG_RADIUS);
        }
    }

    if (m) map_destroy(&m);
    printf("Возврат в меню.\n");
}
