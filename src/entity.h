#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"

/* Тип сущности на карте */
typedef enum EntityType {
    ENT_NONE = 0,
    ENT_TREE,       /* добываемое дерево */
    ENT_ROCK,       /* добываемый камень */
    ENT_HEAL,       /* аптечка/ягода */
    ENT_MONSTER     /* монстр */
} EntityType;

/* Сущность этажа */
typedef struct Entity {
    EntityType type;
    char       symbol;
    int        x, y;
    int        hp, max_hp;
    int        attack;
    int        defense;
    int        drop_item_id;
    int        gold_reward;
    int        stun_chance;    /* % оглушить при ударе */
    int        bleed_chance;   /* % наложить кровотечение */
    int        is_alive;
} Entity;

/* Массив сущностей этажа с метаданными */
typedef struct EntityArray {
    Entity* data;
    int     count;
    int     trees_total;    /* для статистики */
    int     monsters_total;
    int     trees_destroyed;
    int     monsters_killed;
} EntityArray;

/* Жизненный цикл */
EntityArray* entities_generate(const Map* m, int player_x, int player_y,
    int floor_num, int* placed);
void         entities_destroy(EntityArray** arr);
Entity* entity_at(EntityArray* arr, int x, int y);

/* Взаимодействие */
/* Возвращает: 1 — действие выполнено (шаг потрачен),
               0 — действие невозможно (нет сущности и т.п.)
   last_event — строка для HUD (куда писать сообщение) */
int entity_interact(EntityArray* arr, Player* p, const Map* m,
    int target_x, int target_y,
    char* last_event, size_t ev_size);

/* Отрисовка (наложение сущностей поверх карты) */
void entities_print_overlay(const EntityArray* arr, const Map* m,
    int hx, int hy, int use_fog);

/* Пошаговый бой с монстром. Возвращает 1 если герой жив, 0 если эвакуация */
int entity_fight(EntityArray* arr, Entity* mon, Player* p,
    char* last_event, size_t ev_size);

#endif 
