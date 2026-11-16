#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"

/* Жизненный цикл игрока */
Player* player_create(void);
void    player_destroy(Player** p);

/* Статусы */
const char* status_name(StatusEffect s);
void        player_print_buff(const Player* p);
void        player_try_inflict_status(Player* p); /* Попытка наложить новый эффект (5% шанс на шаге) */
void        player_apply_status_step(Player* p, int* used_stamina_out); /* Применить эффект на текущем шаге. Возвращает фактический расход выносливости. */

/* Карточка и инвентарь */
void player_print_card(const Player* p, int day, int hour);
void player_print_inventory(const Player* p);
void player_put_item(Player* p);
void player_drop_item(Player* p);
void player_clear_trash(Player* p);
void player_find_item(const Player* p);

/* Дневник */
void player_write_diary(const Player* p, int day, int hour);

/* Крафт */
void player_check_craft(const Player* p, int day, int hour);

/* Время и отдых */
void player_show_clock(int day, int hour);
void player_rest(Player* p, int* day, int* hour);
void player_pass_time(Player* p, int* day, int* hour);

#endif /* PLAYER_H */
