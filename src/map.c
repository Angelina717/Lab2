#define _CRT_SECURE_NO_WARNINGS
#include "map.h"
#include "player.h"
#include "utils.h"
#include "entity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ВНУТРЕННИЙ МОДУЛЬ КАРТЫ. Все функции static — снаружи не видны.* /

/* выделение / освобождение памяти */

static char** alloc_cells(int h, int w)
{
    char** c = (char**)malloc((size_t)h * sizeof(char*));
    if (!c) {
        printf("Ошибка выделения памяти под строки карты!\n");
        return NULL;
    }
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

    /* Генерируем ТОЛЬКО рамку-стену по краям, внутри — пол */
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (y == 0 || y == h - 1 || x == 0 || x == w - 1) {
                c[y][x] = CH_WALL;
            }
            else {
                c[y][x] = CH_FLOOR;
            }
        }
    }

    /* Блок с рандомными внутренними стенами удалён.
       препятствиями будут сущности (T, R, M) из entity.c */

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

/*  HUD приключения  */

static void print_hud(const Player* p, int floor_num, int day, int hour)
{
    printf("HP: %d/%d | Выносливость: %d/%d | Атака: %d | Защита: %d | "
        "Золото: %d | Этаж: %d | Время: %02d:00 (День %d)\n",
        p->hp, p->max_hp, p->stamina, p->max_stamina,
        p->attack, p->defense, p->gold,
        floor_num, hour, day);
}

/* Экстренная эвакуация (обморок)  */

static void emergency_evacuate(Player* p)
{
    int lost = p->gold / EMERGENCY_GOLD_LOSS_DIV;
    p->gold -= lost;
    printf("\n>>> ЭКСТРЕННАЯ ЭВАКУАЦИЯ! Спасатели забрали %d золота. <<<\n",
        lost);
    p->hp = p->max_hp;
    p->stamina = p->max_stamina / 2;
    if (p->stamina < 1) p->stamina = 1;
    p->buff.type = STATUS_NONE;
    p->buff.turns = 0;
    p->buff.power = 0;
    p->stunned = 0;
}

/*  Запись отчёта в дневник  */

static void write_adventure_diary(const Player* p, int day, int floor_num,
    const EntityArray* ea, const char* reason)
{
    FILE* f = fopen(DIARY_FILE, "a");
    if (!f) {
        printf("[!] Не удалось открыть %s для записи отчёта.\n", DIARY_FILE);
        return;
    }
    fprintf(f, "\n=== Отчёт о вылазке ===\n");
    fprintf(f, "Фермер: %s\n", p->name);
    fprintf(f, "День %d, этаж %d\n", day, floor_num);
    fprintf(f, "Причина завершения: %s\n", reason ? reason : "выход");
    if (ea) {
        fprintf(f, "Добыто ресурсов: %d\n", ea->trees_destroyed);
        fprintf(f, "Побеждено монстров: %d\n", ea->monsters_killed);
    }
    fprintf(f, "Золото: %d | HP: %d/%d | Выносливость: %d/%d\n",
        p->gold, p->hp, p->max_hp, p->stamina, p->max_stamina);
    fclose(f);
    printf("Отчёт записан в %s.\n", DIARY_FILE);
}

/* Игровой цикл Input -> Update -> Render*/

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

    /*  Создание карты первого этажа  */
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

    /*  Генерация сущностей этажа */
    int placed = 0;
    EntityArray* ea = entities_generate(m, hx, hy, m->floor_num, &placed);
    if (!ea) { map_destroy(&m); return; }

    printf("\n=== Приключение началось! (Этаж 1 - Без тумана) ===\n");
    printf("Карта %dx%d. Сущностей: %d.\n", m->w, m->h, placed);
    printf("W/A/S/D — шаг или взаимодействие, Q — выход.\n");

    char last_event[MAX_LINE_LEN] = "Вы вошли в подземелье.";
    int  running = 1;
    const char* end_reason = "выход по Q";

    while (running) {
        /*  Render */
        int use_fog = (m->floor_num > 1);
        entities_print_overlay(ea, m, hx, hy, use_fog);
        print_hud(p, m->floor_num, *day, *hour);
        player_print_status_line(p);
        printf("Последнее событие: %s\n", last_event);
        printf("Ваш ход: ");

        /* Input */
        char cmd[8];
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) { clear_input(); continue; }
        size_t len = strlen(cmd);
        while (len > 0 && (cmd[len - 1] == '\n' || cmd[len - 1] == '\r'))
            cmd[--len] = '\0';
        if (len != 1) {
            printf("Введите одну букву: W/A/S/D/Q.\n");
            continue;
        }

        char c = cmd[0];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');

        if (c == 'Q') {
            snprintf(last_event, sizeof(last_event),
                "Вы покинули подземелье на этаже %d.", m->floor_num);
            end_reason = "выход по Q";
            running = 0;
            break;
        }

        int dx = 0, dy = 0;
        switch (c) {
        case 'W': dy = -1; break;
        case 'A': dx = -1; break;
        case 'S': dy = +1; break;
        case 'D': dx = +1; break;
        default:
            printf("Неизвестная команда. Используйте W/A/S/D/Q.\n");
            continue;
        }

        /*  Update */
        int nx = hx + dx, ny = hy + dy;

        /* Край карты */
        if (nx < 0 || nx >= m->w || ny < 0 || ny >= m->h) {
            snprintf(last_event, sizeof(last_event), "Край карты.");
            continue;
        }
        /* Стена */
        if (m->cells[ny][nx] == CH_WALL) {
            snprintf(last_event, sizeof(last_event), "Там стена.");
            continue;
        }

        Entity* e = entity_at(ea, nx, ny);
        if (e) {
            /* На клетке есть сущность — взаимодействие (без шага) */
            int ok = entity_interact(ea, p, m, nx, ny,
                last_event, sizeof(last_event));
            if (!ok) continue;

            if (p->hp <= 0) {
                end_reason = "обморок в бою";
                emergency_evacuate(p);
                running = 0;
                break;
            }
        }
        else {
            /* Обычный шаг */
            hx = nx; hy = ny;
            p->x = hx; p->y = hy;

            /* статусы/выносливость (вариант 3 из ЛР5) */
            int used = 1;
            player_apply_status_step(p, &used);
            p->stamina -= used;
            if (p->stamina < 0) p->stamina = 0;
            player_try_inflict_status(p);

            snprintf(last_event, sizeof(last_event), "Вы сделали шаг.");
        }

        /* Общие проверки после хода */
        if (p->hp <= 0 || p->stamina <= 0) {
            printf("\n>>> Герой падает без сил! <<<\n");
            end_reason = "обморок от истощения";
            emergency_evacuate(p);
            running = 0;
            break;
        }

        /*  Спуск на новый этаж  */
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
                end_reason = "сон на ходу в 02:00";
                emergency_evacuate(p);
                running = 0;
                break;
            }

            /*  Пересборка этажа */
            map_destroy(&m);
            entities_destroy(&ea);

            m = map_create(new_floor);
            if (!m || !map_generate(m)) {
                if (m) map_destroy(&m);
                ea = NULL;
                break;
            }
            if (!map_find_spawn(m, &hx, &hy)) {
                printf("Ошибка спавна героя на новом этаже.\n");
                map_destroy(&m);
                ea = NULL;
                break;
            }
            map_place_stair_up(m, hx, hy);
            map_place_stair_down(m, hx, hy);
            p->x = hx; p->y = hy;

            int new_placed = 0;
            ea = entities_generate(m, hx, hy, new_floor, &new_placed);
            if (!ea) { map_destroy(&m); break; }

            snprintf(last_event, sizeof(last_event),
                "Вы на этаже %d. Сущностей: %d.", new_floor, new_placed);

            if (new_floor == 2)
                printf("\n=== ВНИМАНИЕ: темно (туман войны). Радиус %d. ===\n",
                    FOG_RADIUS);
        }
    }

    /*  Дневник похода */
    int final_floor = m ? m->floor_num : 0;
    write_adventure_diary(p, *day, final_floor, ea, end_reason);

    /* Освобождение памяти */
    if (ea) entities_destroy(&ea);
    if (m)  map_destroy(&m);
    printf("Возврат в меню.\n");
}
