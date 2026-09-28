#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define INVENTORY_SIZE 10

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
    
         /* --- Защита от «дурака» --- */
         if (scanf("%d", &choice) != 1) {
             while ((c = getchar()) != '\n' && c != EOF) {}
             printf("Ошибка ввода! Введите число.\n");
             choice = -1;   /* чтобы не выйти из цикла случайно */
             continue;
         }
         while ((c = getchar()) != '\n' && c != EOF) {} // бесконечный цикл
