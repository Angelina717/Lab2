#define _CRT_SECURE_NO_WARNINGS
#include "player.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*  Создание / удаление */
Player* player_create(void)
{
    Player* p = (Player*)malloc(sizeof(Player));
    if (!p) {
        printf("[!] Не удалось выделить память под игрока!\n");
        return NULL;
    }
    memset(p, 0, sizeof(Player));

    char buf[MAX_FARMER_NAME];
    read_line("Введите имя фермера (до 32 символов): ", buf, sizeof(buf));
    if (strlen(buf) == 0)
        strncpy(p->name, "Фермер", MAX_FARMER_NAME - 1);
    else
        strncpy(p->name, buf, MAX_FARMER_NAME - 1);

    p->max_hp = PLAYER_START_HP;
    p->hp = PLAYER_START_HP;
    p->max_stamina = PLAYER_START_STAMINA;
    p->stamina = PLAYER_START_STAMINA;
    p->gold = PLAYER_START_GOLD;
    p->x = p->y = 0;

    int start_items[INVENTORY_SIZE] = {
        ITEM_EMPTY, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS,
        ITEM_IRON,  ITEM_GOLD, ITEM_APPLE, ITEM_POTION,
        ITEM_ROPE,  ITEM_TORCH
    };
    memcpy(p->inventory, start_items, sizeof(start_items));

    p->buff.type = STATUS_NONE;
    p->buff.turns = 0;

    printf("Привет, %s! Персонаж создан по адресу %p.\n", p->name, (void*)p);
    return p;
}

void player_destroy(Player** p)
{
    if (p == NULL || *p == NULL) return;
    printf("Память структуры по адресу %p освобождена\n", (void*)*p);
    free(*p);
    *p = NULL;
}

/* Статусы (вариант 3) */
const char* status_name(StatusEffect s)
{
    switch (s) {
    case STATUS_HEALTHY:   return "Здоров";
    case STATUS_SPRAIN:    return "Растяжение";
    case STATUS_POISON:    return "Отравление";
    case STATUS_ENERGIZED: return "Бодрость";
    case STATUS_NONE:
    default:               return "Нет";
    }
}

void player_print_buff(const Player* p)
{
    if (!p) return;
    printf("Статус:        %s", status_name(p->buff.type));
    if (p->buff.type != STATUS_NONE && p->buff.turns > 0)
        printf(" (осталось шагов: %d)", p->buff.turns);
    printf("\n");
}

/* Попытка наложить новый эффект (5% шанс на шаге) */
void player_try_inflict_status(Player* p)
{
    if (!p) return;
    if (p->buff.type != STATUS_NONE) return;
    if (rand() % 100 >= STATUS_INFLICT_CHANCE) return;

    int r = rand() % 3;
    switch (r) {
    case 0: p->buff.type = STATUS_SPRAIN;    break;
    case 1: p->buff.type = STATUS_POISON;    break;
    default:p->buff.type = STATUS_ENERGIZED; break;
    }
    p->buff.turns = STATUS_MAX_DURATION;
    printf(">>> Вы наступили на колючку! Получен статус: %s на %d шагов.\n",
        status_name(p->buff.type), p->buff.turns);
}

/* Применить эффект на текущем шаге. Возвращает фактический расход выносливости. */
void player_apply_status_step(Player* p, int* used_stamina_out)
{
    if (!p || !used_stamina_out) return;

    int cost = 1;

    if (p->buff.type == STATUS_SPRAIN)    cost *= STATUS_SPRAIN_MULT;
    if (p->buff.type == STATUS_ENERGIZED) cost = STATUS_BUFF_FREE_COST;

    if (p->buff.type == STATUS_POISON) {
        p->hp -= STATUS_POISON_HP_DMG;
        if (p->hp < 0) p->hp = 0;
        printf("Яд отнимает %d HP (осталось %d).\n", STATUS_POISON_HP_DMG, p->hp);
    }

    if (p->buff.type != STATUS_NONE && p->buff.turns > 0) {
        p->buff.turns--;
        if (p->buff.turns == 0) {
            printf("Статус %s закончился.\n", status_name(p->buff.type));
            p->buff.type = STATUS_NONE;
        }
    }

    *used_stamina_out = cost;
}

/*  Карточка и инвентарь */
void player_print_card(const Player* p, int day, int hour)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    printf("\n--- Карточка персонажа ---\n");
    printf("Имя:           %s\n", p->name);
    printf("HP:            %d/%d\n", p->hp, p->max_hp);
    printf("Выносливость:  %d/%d\n", p->stamina, p->max_stamina);
    printf("Золото:        %d\n", p->gold);
    printf("Позиция:       (%d, %d)\n", p->x, p->y);
    printf("Время:         День %d, %02d:00\n", day, hour);
    player_print_buff(p);
    printf("Инвентарь:\n");
    for (int i = 0; i < INVENTORY_SIZE; i++)
        printf("  Слот %d: [%d] %s\n", i, p->inventory[i],
            get_item_name(p->inventory[i]));
}

void player_print_inventory(const Player* p)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    printf("\n--- Инвентарь ---\n");
    for (int i = 0; i < INVENTORY_SIZE; i++)
        printf("Слот %d: [%d] - %s\n",
            i, p->inventory[i], get_item_name(p->inventory[i]));
}

/* Положить предмет в слот*/
void player_put_item(Player* p)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    int slot, id;
    if (!read_int("Индекс слота (0-9): ", &slot)) return;
    if (slot < 0 || slot >= INVENTORY_SIZE) { printf("Индекс вне диапазона!\n"); return; }
    if (!read_int("ID предмета: ", &id)) return;
    if (id < 0 || id >= MAX_ITEMS) { printf("Недопустимый ID!\n"); return; }
    p->inventory[slot] = id;
    printf("Предмет \"%s\" помещён в слот %d.\n", get_item_name(id), slot);
}

/* Выбросить  предмет из слота*/
void player_drop_item(Player* p)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    int slot;
    if (!read_int("Индекс слота для удаления (0-9): ", &slot)) return;
    if (slot < 0 || slot >= INVENTORY_SIZE) { printf("Индекс вне диапазона!\n"); return; }
    p->inventory[slot] = ITEM_EMPTY;
    printf("Слот %d очищен.\n", slot);
}

/* Очистка мусора*/
void player_clear_trash(Player* p)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    int id;
    if (!read_int("Введите ID предмета для удаления: ", &id)) return;
    printf("\nИнвентарь до очистки:\n");
    player_print_inventory(p);
    int cleared = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (p->inventory[i] == id && id != ITEM_EMPTY) {
            p->inventory[i] = ITEM_EMPTY;
            cleared++;
        }
    }
    printf("\nИнвентарь после очистки:\n");
    player_print_inventory(p);
    printf("Очищено слотов: %d\n", cleared);
}

/* Найти предмет в рюкзаке*/
void player_find_item(const Player* p)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    char query[MAX_NAME_LEN];
    read_line("Введите название предмета: ", query, sizeof(query));
    if (strlen(query) == 0) { printf("Пустой запрос.\n"); return; }
    int found_id = -1;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(get_item_name(i), query) == 0) { found_id = i; break; }
    }
    if (found_id == -1) { printf("Предмет \"%s\" не найден в каталоге.\n", query); return; }
    printf("ID предмета \"%s\" = %d. Ищем в рюкзаке...\n", query, found_id);
    int any = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (p->inventory[i] == found_id) {
            printf("  Найдено в слоте %d\n", i);
            any = 1;
        }
    }
    if (!any) printf("В рюкзаке такого предмета нет.\n");
}

/* Дневник */
void player_write_diary(const Player* p, int day, int hour)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    FILE* f = fopen(DIARY_FILE, "a");
    if (f == NULL) { printf("Не удалось открыть %s для записи!\n", DIARY_FILE); return; }

    fprintf(f, "Запись дневника\n");
    fprintf(f, "Фермер: %s\n", p->name);
    fprintf(f, "День %d, %02d:00\n", day, hour);
    fprintf(f, "HP %d/%d | Выносливость %d/%d | Золото %d\n",
        p->hp, p->max_hp, p->stamina, p->max_stamina, p->gold);
    fprintf(f, "Статус: %s\n", status_name(p->buff.type));
    fprintf(f, "Инвентарь:\n");
    for (int i = 0; i < INVENTORY_SIZE; i++)
        fprintf(f, "  Слот %d: [%d] - %s\n",
            i, p->inventory[i], get_item_name(p->inventory[i]));
    fprintf(f, "\n");
    fclose(f);

    printf("\n=== Запись дневника ===\n");
    printf("Фермер: %s\n", p->name);
    printf("День %d, %02d:00\n", day, hour);
    printf("Состояние записано в %s.\n", DIARY_FILE);
}

/* Крафт */
typedef struct {
    char name[MAX_NAME_LEN];
    int  ingredients[MAX_INGREDIENTS];
    int  ing_count;
} Recipe;

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
    while (name_len > 0 && r->name[name_len - 1] == ' ')
        r->name[--name_len] = '\0';

    const char* s = colon + 1;
    while (*s != '\0') {
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '\0') break;
        if (*s < '0' || *s > '9') return 0;
        int val = 0;
        while (*s >= '0' && *s <= '9') { val = val * 10 + (*s - '0'); s++; }
        if (r->ing_count >= MAX_INGREDIENTS) return 0;
        r->ingredients[r->ing_count++] = val;
    }
    return r->ing_count > 0;
}

/* Проверка рецептов крафта */
void player_check_craft(const Player* p, int day, int hour)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    FILE* fin = fopen(INPUT_FILE, "r");
    if (fin == NULL) {
        printf("[!] Файл %s не найден. Проверка рецептов невозможна.\n", INPUT_FILE);
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
    fprintf(fout, "Фермер: %s | День %d, %02d:00\n\n", p->name, day, hour);

    char line[MAX_LINE_LEN];
    int line_no = 0, total = 0, available = 0, not_enough = 0;

    while (fgets(line, sizeof(line), fin) != NULL) {
        line_no++;
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0 || line[0] == '#') continue;

        Recipe r;
        if (!parse_recipe_line(line, &r)) {
            printf("Строка %d: некорректный формат, пропущено.\n", line_no);
            fprintf(fout, "Строка %d: [ОШИБКА ФОРМАТА] %s\n", line_no, line);
            continue;
        }
        total++;

        int need[MAX_ITEMS] = { 0 };
        for (int i = 0; i < r.ing_count; i++) {
            int id = r.ingredients[i];
            if (id >= 0 && id < MAX_ITEMS) need[id]++;
        }

        int missing_ids[MAX_ITEMS], missing_cnt[MAX_ITEMS];
        int missing_total = 0;
        for (int id = 0; id < MAX_ITEMS; id++) {
            if (need[id] == 0) continue;
            int have = count_in_inventory(p->inventory, INVENTORY_SIZE, id);
            if (have < need[id]) {
                missing_ids[missing_total] = id;
                missing_cnt[missing_total] = need[id] - have;
                missing_total++;
            }
        }

        char verdict[MAX_LINE_LEN];
        if (missing_total == 0) {
            available++;
            snprintf(verdict, sizeof(verdict), "[ДОСТУПНО]");
        }
        else {
            not_enough++;
            int pos = snprintf(verdict, sizeof(verdict), "[НЕ ХВАТАЕТ РЕСУРСОВ]");
            for (int i = 0; i < missing_total && pos < (int)sizeof(verdict); i++)
                pos += snprintf(verdict + pos, sizeof(verdict) - pos,
                    " %s x%d", get_item_name(missing_ids[i]), missing_cnt[i]);
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

/* Время и отдых */
void player_show_clock(int day, int hour)
{
    printf("Текущее время: День %d, %02d:00\n", day, hour);
}

void player_rest(Player* p, int* day, int* hour)
{
    *day += 1;
    *hour = 8;
    if (p) {
        p->hp = p->max_hp;
        p->stamina = p->max_stamina;
        p->buff.type = STATUS_NONE;
        p->buff.turns = 0;
        printf("Вы отдохнули. HP и выносливость полностью восстановлены.\n");
    }
    printf("Наступил День %d, %02d:00.\n", *day, *hour);
}

void player_pass_time(Player* p, int* day, int* hour)
{
    if (!p) { printf("[!] Персонаж не создан.\n"); return; }
    int h;
    if (!read_int("Сколько часов ждать? ", &h)) return;
    if (h <= 0) { printf("Нужно хотя бы 1 час.\n"); return; }

    /* Нельзя пройти границу 02:00 ночи */
    int remaining = h;
    while (remaining > 0) {
        int to_02 = (2 - *hour + 24) % 24;
        if (to_02 == 0) to_02 = 24;  /* уже 02:00 -> нельзя */
        if (remaining < to_02) {
            *hour = (*hour + remaining) % 24;
            remaining = 0;
        }
        else {
            printf("Нельзя пройти границу 02:00 — герой идёт спать.\n");
            player_rest(p, day, hour);
            remaining = 0;
        }
    }
    printf("Теперь: День %d, %02d:00\n", *day, *hour);
}
