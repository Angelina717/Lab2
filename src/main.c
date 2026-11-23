#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#include "common.h"
#include "player.h"
#include "map.h"          
#include "utils.h"

/*  Меню  */
static void print_menu(const Player* p, int day, int hour)
{
    printf("\n===== Весёлый фермер (ЛР5, вариант 3) =====\n");
    if (p)
        printf("Фермер: %s | HP %d/%d | Атака %d | Защита %d | Золото %d\n",
            p->name, p->hp, p->max_hp, p->attack, p->defense, p->gold);
    else
        printf("Фермер: не создан\n");
    printf("Время: День %d, %02d:00\n", day, hour);
    printf("--------------------------\n");
    printf("[%d]  Создать / пересоздать персонажа\n", MENU_CREATE_PLAYER);
    printf("[%d]  Карточка персонажа\n", MENU_SHOW_CARD);
    printf("[%d]  Посмотреть на часы\n", MENU_CLOCK);
    printf("[%d]  Пропустить время (Ждать)\n", MENU_PASS_TIME);
    printf("[%d]  Отдых (Лечь спать)\n", MENU_REST);
    printf("[%d]  Посмотреть инвентарь\n", MENU_INVENTORY);
    printf("[%d]  Положить предмет\n", MENU_PUT_ITEM);
    printf("[%d]  Выбросить предмет\n", MENU_DROP_ITEM);
    printf("[%d]  Очистка от мусора\n", MENU_CLEAR_TRASH);
    printf("[%d] Поиск предмета по названию\n", MENU_FIND_ITEM);
    printf("[%d] Записать дневник\n", MENU_DIARY);
    printf("[%d] Проверить рецепты крафта\n", MENU_CRAFT);
    printf("[%d] Отправиться в приключение\n", MENU_ADVENTURE);
    printf("[%d]  Выход\n", MENU_EXIT);
    printf("Выберите пункт: ");
}

static int require_player(const Player* p)
{
    if (!p) { printf("[!] Сначала создайте персонажа.\n"); return 0; }
    return 1;
}

/* Точка входа */
int main(void)
{
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand((unsigned)time(NULL));

    load_items_from_file();

    Player* player = NULL;
    int day = 1, hour = 8;
    int choice = -1;

    while (choice != MENU_EXIT) {
        print_menu(player, day, hour);

        if (scanf("%d", &choice) != 1) {
            clear_input();
            printf("Ошибка ввода! Введите число.\n");
            choice = -1;
            continue;
        }
        clear_input();

        switch (choice) {
        case MENU_EXIT:
            printf("Выход из игры.\n");
            break;

        case MENU_CREATE_PLAYER:
            if (player) {
                char ans[8];
                read_line("Персонаж уже существует. Удалить? (yes/no): ", ans, sizeof(ans));
                if (ans[0] != 'y' && ans[0] != 'Y') { printf("Отмена.\n"); break; }
                player_destroy(&player);
            }
            player = player_create();
            day = 1; hour = 8;
            break;

        case MENU_SHOW_CARD:
            if (!require_player(player)) break;
            player_print_card(player, day, hour);
            break;

        case MENU_CLOCK:
            player_show_clock(day, hour);
            break;

        case MENU_PASS_TIME:
            if (!require_player(player)) break;
            player_pass_time(player, &day, &hour);
            break;

        case MENU_REST:
            if (!require_player(player)) break;
            player_rest(player, &day, &hour);
            break;

        case MENU_INVENTORY:
            if (!require_player(player)) break;
            player_print_inventory(player);
            break;

        case MENU_PUT_ITEM:
            if (!require_player(player)) break;
            player_put_item(player);
            break;

        case MENU_DROP_ITEM:
            if (!require_player(player)) break;
            player_drop_item(player);
            break;

        case MENU_CLEAR_TRASH:
            if (!require_player(player)) break;
            player_clear_trash(player);
            break;

        case MENU_FIND_ITEM:
            if (!require_player(player)) break;
            player_find_item(player);
            break;

        case MENU_DIARY:
            if (!require_player(player)) break;
            player_write_diary(player, day, hour);
            break;

        case MENU_CRAFT:
            if (!require_player(player)) break;
            player_check_craft(player, day, hour);
            break;

        case MENU_ADVENTURE:
            if (!require_player(player)) break;
            adventure_mode(player, &day, &hour);
            break;

        default:
            printf("Нет такого пункта меню.\n");
            break;
        }
    }

    if (player) player_destroy(&player);
    return 0;
}
