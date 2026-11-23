#define _CRT_SECURE_NO_WARNINGS
#include "entity.h"
#include "utils.h"
#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Вспомогательные  */

static int clamp_int(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int scaled(int base, int floor_num)
{
    /* base * ENTITY_LEVEL_MULT^(floor-1), округление вверх */
    double v = (double)base;
    for (int i = 1; i < floor_num; i++) v *= ENTITY_LEVEL_MULT;
    return (int)(v + 0.5);
}

static int cell_is_free_for_entity(const Map* m, int x, int y)
{
    return (x > 0 && x < m->w - 1 && y > 0 && y < m->h - 1 &&
        m->cells[y][x] == CH_FLOOR);
}

/* Генерация  */

EntityArray* entities_generate(const Map* m, int player_x, int player_y,
    int floor_num, int* placed)
{
    if (!m || !placed) return NULL;
    *placed = 0;

    EntityArray* arr = (EntityArray*)calloc(1, sizeof(EntityArray));
    if (!arr) { printf("[!] Нет памяти под EntityArray.\n"); return NULL; }

    int area = (m->w - 2) * (m->h - 2);
    if (area < 1) area = 1;
    int wanted = area * ENTITY_DENSITY_PERCENT / 100;
    if (wanted < 3) wanted = 3;

    arr->data = (Entity*)calloc((size_t)wanted, sizeof(Entity));
    if (!arr->data) {
        printf("[!] Нет памяти под массив сущностей.\n");
        free(arr);
        return NULL;
    }
    arr->count = 0;

    int attempt = 0, max_attempts = wanted * 40;
    while (arr->count < wanted && attempt < max_attempts) {
        attempt++;
        int x = 1 + rand() % (m->w - 2);
        int y = 1 + rand() % (m->h - 2);
        if (!cell_is_free_for_entity(m, x, y)) continue;
        if (x == player_x && y == player_y) continue;

        /* не ставим сущность поверх другой */
        int busy = 0;
        for (int i = 0; i < arr->count; i++)
            if (arr->data[i].is_alive &&
                arr->data[i].x == x && arr->data[i].y == y) {
                busy = 1; break;
            }
        if (busy) continue;

        Entity e;
        memset(&e, 0, sizeof(e));
        e.x = x; e.y = y; e.is_alive = 1;

        int roll = rand() % 100;
        if (roll < 35) {
            e.type = ENT_TREE;
            e.symbol = CH_TREE;
            e.max_hp = scaled(20, floor_num);
            e.hp = e.max_hp;
            e.attack = 0; e.defense = 0;
            e.drop_item_id = ITEM_WOOD;
            e.gold_reward = 0;
            arr->trees_total++;
        }
        else if (roll < 60) {
            e.type = ENT_ROCK;
            e.symbol = CH_ROCK;
            e.max_hp = scaled(30, floor_num);
            e.hp = e.max_hp;
            e.attack = 0; e.defense = 3;
            e.drop_item_id = ITEM_STONE;
            e.gold_reward = 0;
            arr->trees_total++;
        }
        else if (roll < 75) {
            e.type = ENT_HEAL;
            e.symbol = CH_HEAL;
            e.max_hp = 1; e.hp = 1;
            e.attack = 0; e.defense = 0;
            e.drop_item_id = ITEM_EMPTY;
        }
        else {
            e.type = ENT_MONSTER;
            e.symbol = CH_MONSTER;
            e.max_hp = scaled(30, floor_num);
            e.hp = e.max_hp;
            e.attack = scaled(6, floor_num);
            e.defense = scaled(2, floor_num);
            e.drop_item_id = (rand() % 2) ? ITEM_POTION : ITEM_APPLE;
            e.gold_reward = scaled(MONSTER_GOLD_BASE, floor_num) +
                rand() % (MONSTER_GOLD_BASE + 1);
            /* Вариант 7: шансы на эффекты растут с этажом */
            e.stun_chance = clamp_int(MONSTER_BASE_STUN + floor_num, 0, 60);
            e.bleed_chance = clamp_int(MONSTER_BASE_BLEED + floor_num, 0, 60);
            arr->monsters_total++;
        }

        arr->data[arr->count++] = e;
        (*placed)++;
    }

    printf("Сгенерировано сущностей: %d (деревьев/камней: %d, монстров: %d).\n",
        arr->count, arr->trees_total, arr->monsters_total);
    return arr;
}

void entities_destroy(EntityArray** arr)
{
    if (!arr || !*arr) return;
    if ((*arr)->data) {
        printf("Память массива сущностей по адресу %p освобождена\n",
            (void*)(*arr)->data);
        free((*arr)->data);
    }
    free(*arr);
    *arr = NULL;
}

Entity* entity_at(EntityArray* arr, int x, int y)
{
    if (!arr) return NULL;
    for (int i = 0; i < arr->count; i++)
        if (arr->data[i].is_alive &&
            arr->data[i].x == x && arr->data[i].y == y)
            return &arr->data[i];
    return NULL;
}

/*  Отрисовка  */

void entities_print_overlay(const EntityArray* arr, const Map* m,
    int hx, int hy, int use_fog)
{
    if (!m) return;
    (void)use_fog; /* учтено картой; здесь рисуем только живые сущности */
    printf("\n");
    for (int y = 0; y < m->h; y++) {
        for (int x = 0; x < m->w; x++) {
            char ch = m->cells[y][x];
            int vis = !use_fog || ((x - hx) * (x - hx) + (y - hy) * (y - hy)
                <= FOG_RADIUS * FOG_RADIUS);
            if (!vis) { putchar(CH_FOG); continue; }
            if (x == hx && y == hy) { putchar(CH_HERO); continue; }

            Entity* e = NULL;
            if (arr)
                for (int i = 0; i < arr->count; i++)
                    if (arr->data[i].is_alive &&
                        arr->data[i].x == x && arr->data[i].y == y)
                    {
                        e = &arr->data[i]; break;
                    }

            if (e) putchar(e->symbol);
            else   putchar(ch);
        }
        putchar('\n');
    }
    printf("\n");
}

/*  Боевые формулы */

static int calc_damage(int atk, int def)
{
    int dmg = atk - def / 2;
    return dmg < MIN_DAMAGE ? MIN_DAMAGE : dmg;
}

/* Попытка наложить оглушение/кровотечение на цель.
   atk — атакующий (нужен для силы кровотечения),
   stun_ch — шанс оглушения, bleed_ch — шанс кровотечения. */
static void try_apply_effects(Player* p, Entity* mon,
    int stun_ch, int bleed_ch,
    int attacker_atk, int target_is_player,
    char* msg, size_t msg_size)
{
    if (stun_ch > 0 && rand() % 100 < stun_ch) {
        if (target_is_player) {
            p->stunned = 1;
            strncat(msg, " Вы оглушены!", msg_size - strlen(msg) - 1);
        }
        else if (mon) {
            /* на монстре оглушение обрабатывается в бою — просто помечаем */
            strncat(msg, " Монстр оглушён!", msg_size - strlen(msg) - 1);
        }
    }
    if (bleed_ch > 0 && rand() % 100 < bleed_ch) {
        if (target_is_player) {
            p->buff.type = STATUS_BLEEDING;
            p->buff.turns = BLEED_DURATION;
            p->buff.power = attacker_atk * BLEED_PERCENT / 100;
            if (p->buff.power < 1) p->buff.power = 1;
            strncat(msg, " Кровотечение!", msg_size - strlen(msg) - 1);
        }
        else if (mon) {
            /* на монстра кровотечение — просто доп. урон в бою */
            strncat(msg, " Монстр истекает кровью!", msg_size - strlen(msg) - 1);
        }
    }
}

/*  Добыча ресурсов и аптечки */

static int interact_resource(EntityArray* arr, Entity* e, Player* p,
    char* last_event, size_t ev_size)
{
    int crit = (rand() % 100 < 15) ? 1 : 0;
    int dmg = calc_damage(p->attack, e->defense);
    if (crit) dmg *= 2;
    e->hp -= dmg;

    char buf[MAX_LINE_LEN];
    snprintf(buf, sizeof(buf),
        "Вы %s%s объект на %d ед. (осталось прочности: %d).",
        crit ? "КРИТИЧЕСКИ " : "",
        (e->type == ENT_TREE) ? "рубите" : "долбите",
        dmg, e->hp > 0 ? e->hp : 0);
    strncpy(last_event, buf, ev_size - 1);
    last_event[ev_size - 1] = '\0';

    if (e->hp > 0) return 1;

    e->is_alive = 0;
    if (e->type == ENT_TREE || e->type == ENT_ROCK)
        arr->trees_destroyed++;

    /* пытаемся положить дроп в инвентарь */
    int slot = -1;
    for (int i = 0; i < INVENTORY_SIZE; i++)
        if (p->inventory[i] == ITEM_EMPTY) { slot = i; break; }

    if (slot == -1) {
        strncat(last_event, " Рюкзак полон! Ресурс потерян.",
            ev_size - strlen(last_event) - 1);
    }
    else {
        p->inventory[slot] = e->drop_item_id;
        char add[MAX_NAME_LEN + 48];
        snprintf(add, sizeof(add), " Получено: %s!",
            get_item_name(e->drop_item_id));
        strncat(last_event, add, ev_size - strlen(last_event) - 1);
    }
    return 1;
}

static int interact_heal(EntityArray* arr, Entity* e, Player* p,
    char* last_event, size_t ev_size)
{
    (void)arr;
    int hp_before = p->hp, st_before = p->stamina;
    p->hp = clamp_int(p->hp + HEAL_HP_RESTORE, 0, p->max_hp);
    p->stamina = clamp_int(p->stamina + HEAL_STAMINA_RESTORE, 0, p->max_stamina);
    e->is_alive = 0;

    snprintf(last_event, ev_size,
        "Вы подобрали дар природы: +%d HP, +%d выносливости.",
        p->hp - hp_before, p->stamina - st_before);
    return 1;
}

/*  Пошаговый бой  */

int entity_fight(EntityArray* arr, Entity* mon, Player* p,
    char* last_event, size_t ev_size)
{
    if (!mon || !mon->is_alive) return 1;

    int round = 0;
    while (mon->hp > 0 && p->hp > 0 && round < 20) {
        round++;
        p->stunned = 0;

        /* -Ход героя */
        int hero_crit = (rand() % 100 < 15) ? 1 : 0;
        int hero_dmg = calc_damage(p->attack, mon->defense);
        if (hero_crit) hero_dmg *= 2;
        mon->hp -= hero_dmg;
        if (mon->hp < 0) mon->hp = 0; /* Защита от отрицательного HP монстра */

        char round_msg[MAX_LINE_LEN] = { 0 };
        snprintf(round_msg, sizeof(round_msg),
            "Ход %d. Вы бьёте: %d%s урона.",
            round, hero_dmg, hero_crit ? " (КРИТ!)" : "");

        if (mon->hp <= 0) {
            /* ... (код победы над монстром остаётся без изменений) ... */
            mon->is_alive = 0;
            arr->monsters_killed++;
            p->gold += mon->gold_reward;

            int slot = -1;
            for (int i = 0; i < INVENTORY_SIZE; i++)
                if (p->inventory[i] == ITEM_EMPTY) { slot = i; break; }

            char tail[MAX_LINE_LEN];
            int drop_ok = (rand() % 100 < MONSTER_DROP_CHANCE) && slot >= 0;
            if (drop_ok) {
                p->inventory[slot] = mon->drop_item_id;
                snprintf(tail, sizeof(tail),
                    " Монстр повержен! +%d золота, трофей: %s.",
                    mon->gold_reward, get_item_name(mon->drop_item_id));
            }
            else {
                snprintf(tail, sizeof(tail),
                    " Монстр повержен! +%d золота.", mon->gold_reward);
            }
            strncat(round_msg, tail, sizeof(round_msg) - strlen(round_msg) - 1);
            strncpy(last_event, round_msg, ev_size - 1);
            last_event[ev_size - 1] = '\0';
            return 1;
        }

        /* Наложение эффектов героем на монстра  */
        try_apply_effects(p, mon, 0, BLEED_CHANCE, p->attack, 0,
            round_msg, sizeof(round_msg));

        /* кровотечение монстра — доп. урон */
        if (strstr(round_msg, "истекает кровью")) {
            int bleed = p->attack * BLEED_PERCENT / 100;
            if (bleed < 1) bleed = 1;
            mon->hp -= bleed;
            if (mon->hp < 0) mon->hp = 0; /* Защита от отрицательного HP */

            if (mon->hp <= 0) {
                mon->is_alive = 0;
                arr->monsters_killed++;
                p->gold += mon->gold_reward;
                char tail[128];
                snprintf(tail, sizeof(tail),
                    " Монстр истёк кровью и погиб! +%d золота.",
                    mon->gold_reward);
                strncat(round_msg, tail,
                    sizeof(round_msg) - strlen(round_msg) - 1);
                strncpy(last_event, round_msg, ev_size - 1);
                last_event[ev_size - 1] = '\0';
                return 1;
            }
        }

        /* Ответный ход монстра */
        if (mon->hp > 0) {
            int mon_dmg = calc_damage(mon->attack, p->defense);
            p->hp -= mon_dmg;
            if (p->hp < 0) p->hp = 0; /*  ЗАЩИТА ОТ МИНУСА */

            char tail[128];
            snprintf(tail, sizeof(tail), " Монстр бьёт: %d урона.", mon_dmg);
            strncat(round_msg, tail,
                sizeof(round_msg) - strlen(round_msg) - 1);

            /* монстр накладывает эффекты на игрока */
            try_apply_effects(p, mon, mon->stun_chance, mon->bleed_chance,
                mon->attack, 1,
                round_msg, sizeof(round_msg));
        }

        strncpy(last_event, round_msg, ev_size - 1);
        last_event[ev_size - 1] = '\0';

        if (p->hp <= 0) return 0;
    }

    return (p->hp > 0) ? 1 : 0;
}

/*  Основное взаимодействие  */

int entity_interact(EntityArray* arr, Player* p, const Map* m,
    int target_x, int target_y,
    char* last_event, size_t ev_size)
{
    (void)m;
    if (!arr || !p || !last_event || ev_size == 0) return 0;

    Entity* e = entity_at(arr, target_x, target_y);
    if (!e) return 0;

    if (p->stamina < INTERACT_STAMINA_COST) {
        snprintf(last_event, ev_size,
            "Не хватает выносливости на действие (нужно %d).",
            INTERACT_STAMINA_COST);
        return 0;
    }
    p->stamina -= INTERACT_STAMINA_COST;
    if (p->stamina < 0) p->stamina = 0; /* ДЛЯ СТРАХОВКИ */

    switch (e->type) {
    case ENT_TREE:
    case ENT_ROCK:
        return interact_resource(arr, e, p, last_event, ev_size);

    case ENT_HEAL:
        return interact_heal(arr, e, p, last_event, ev_size);

    case ENT_MONSTER:
        return entity_fight(arr, e, p, last_event, ev_size);

    default:
        return 0;
    }
}
