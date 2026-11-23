#ifndef COMMON_H
#define COMMON_H

/*  Константы  */
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
#define MAP_MIN_SIZE         5
#define MAP_MAX_SIZE         30
#define OBSTACLE_PERCENT_MIN 15
#define OBSTACLE_PERCENT_MAX 30
#define FOG_RADIUS           4 /* радиус видимости вокруг героя */

/* Символы карты */
#define CH_WALL       '#'
#define CH_FLOOR      '.'
#define CH_TREE       'T'
#define CH_ROCK       'R'
#define CH_HERO       '@'
#define CH_FOG        '?'
#define CH_STAIR_UP   'U'
#define CH_STAIR_DOWN 'D'
#define CH_HEAL       '+'
#define CH_MONSTER    'M'

/*  стартовые характеристики, экономика, статусы*/
#define PLAYER_START_HP        100
#define PLAYER_START_STAMINA   50
#define PLAYER_START_GOLD      50
#define FLOOR_GOLD_BASE        10    /* Награда = FLOOR_GOLD_BASE * номер_этажа */
#define STAIR_STAMINA_COST     5     /* Спуск на этаж тратит выносливость  */
#define EMERGENCY_GOLD_LOSS_DIV 2    /* Эвакуация забирает половину золота */

/* Вариант 3 — статусы и травмы */
#define STATUS_MAX_DURATION      10   /* длительность эффекта в шагах */
#define STATUS_INFLICT_CHANCE    5    /* 5% шанс получить эффект на шаге */
#define STATUS_POISON_HP_DMG     2    /* Отравление: -2 HP/шаг */
#define STATUS_SPRAIN_MULT       2    /* Растяжение: x2 расход выносливости */
#define STATUS_BUFF_FREE_COST    0    /* Бодрость: шаг бесплатный */

/*  НОВОЕ (ЛР6)  */
#define PLAYER_START_ATTACK    5
#define PLAYER_START_DEFENSE   2

#define INTERACT_STAMINA_COST  2    /* активное действие на клетку с сущностью */
#define ENTITY_LEVEL_MULT      1.25 /* множитель роста характеристик с этажом */
#define ENTITY_DENSITY_PERCENT 12   /* % от площади карты под сущности */
#define HEAL_HP_RESTORE        25
#define HEAL_STAMINA_RESTORE   10
#define MONSTER_GOLD_BASE      8
#define MONSTER_DROP_CHANCE    40   /* % шанс выпадения трофея */
#define MIN_DAMAGE             1

/* Вариант 7: Кровопускание и Оглушение */
#define STATUS_BLEED       5    /* новый тип статуса */
#define STATUS_STUN        6
#define BLEED_DURATION     3    /* 3 шага по карте */
#define BLEED_PERCENT      20   /* 20% от базового урона атакующего за шаг */
#define STUN_CHANCE        20   /* % шанс оглушить цель */
#define BLEED_CHANCE       25   /* % шанс наложить кровотечение */
#define MONSTER_BASE_STUN  15
#define MONSTER_BASE_BLEED 20

/*  Перечисления  */
typedef enum MenuAction {
    MENU_EXIT = 0,
    MENU_CREATE_PLAYER,
    MENU_SHOW_CARD,
    MENU_CLOCK,
    MENU_PASS_TIME,
    MENU_REST,
    MENU_INVENTORY,
    MENU_PUT_ITEM,
    MENU_DROP_ITEM,
    MENU_CLEAR_TRASH,
    MENU_FIND_ITEM,
    MENU_DIARY,
    MENU_CRAFT,
    MENU_ADVENTURE
} MenuAction;

typedef enum CellType {
    CELL_WALL = 0,
    CELL_FLOOR,
    CELL_TREE,
    CELL_ROCK,
    CELL_STAIR_UP,
    CELL_STAIR_DOWN,
    CELL_UNKNOWN
} CellType;

typedef enum StatusEffect {
    STATUS_NONE = 0,
    STATUS_HEALTHY,
    STATUS_SPRAIN,
    STATUS_POISON,
    STATUS_ENERGIZED,
    STATUS_BLEEDING,   /* вариант 7 */
    STATUS_STUNNED     /* вариант 7 */
} StatusEffect;

/*  Структуры  */
/* Активный эффект (вариант 3) */
typedef struct Buff {
    StatusEffect type;   /* тип эффекта */
    int          turns;  /* оставшееся число ходов */
    int          power;  /* для кровотечения — сколько HP снимать за шаг */
} Buff;

/* Структура игрока. Добавлены боевые параметры */
typedef struct Player {
    char name[MAX_FARMER_NAME];
    int  hp;
    int  max_hp;
    int  stamina;
    int  max_stamina;
    int  gold;
    int  attack;        /* NEW */
    int  defense;       /* NEW */
    int  x, y;                       /* координаты на карте */
    int  inventory[INVENTORY_SIZE];  /* ID предметов */
    Buff buff;                       /* активный статус */
} Player;

/* Структура карты */
typedef struct Map {
    char** cells;   /* cells[y][x] — символ клетки */
    int    w;       /* ширина (столбцы, x) */
    int    h;       /* высота (строки, y) */
    int    floor_num;
} Map;

#endif /* COMMON_H */
