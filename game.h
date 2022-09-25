#ifndef NB_GAME_H
#define NB_GAME_H

/* Types */
#define NB_FLT   float
#define NB_INT   int
#define NB_UINT  unsigned int
#define NB_COL   unsigned int
#define NB_BOOL  int
#define NB_TRUE  1
#define NB_FALSE 0
#define NB_STR   const char*
#define NB_NULL  0

/* Constants */
#define NB_TITLE       "Game"
#define NB_WIDTH       160
#define NB_HEIGHT      90
#define NB_FRAMERATE   60
#define NB_ENTITIES    64
#define NB_ROOM_WIDTH  256
#define NB_ROOM_HEIGHT 256
#define NB_GRID_SIZE   8
#define NB_ROOM_COLS   (NB_ROOM_WIDTH / NB_GRID_SIZE)
#define NB_ROOM_ROWS   (NB_ROOM_HEIGHT / NB_GRID_SIZE)
#define NB_DELTA       (1.0f / NB_FRAMERATE)

/* Controls */
typedef enum Buttons
{
	NB_LEFT,
	NB_RIGHT,
	NB_UP,
	NB_DOWN,
	NB_A,
	NB_B,
	NB_BUTTON_COUNT
} Buttons;

/* Entity */
struct Entity;
struct EntityEvent;
typedef void (*EntityType)(struct EntityEvent*); /* entity type is just a pointer to a behavior function */

typedef struct Entity
{
	EntityType type;        /* entity type; a pointer to a behavior function */
	NB_INT x, y;            /* position */
	NB_FLT sx, sy;          /* speed */
	NB_FLT rx, ry;          /* movement remainder */
	NB_INT bx, by, bw, bh;  /* bounding box */
	NB_BOOL solids;         /* if we should collide with solids */
	NB_BOOL overlaps;       /* if we should overlap with other entities */
	NB_FLT timer;           /* general timer, starts at 0 on create */
} Entity;

/* Entity Event */
typedef enum EntityEventType
{
	NB_ENTITY_INIT,
	NB_ENTITY_UPDATE,
	NB_ENTITY_DRAW,
	NB_ENTITY_DESTROY,
	NB_ENTITY_OVERLAP
} EntityEventType;

typedef struct EntityEvent
{
	EntityEventType type;
	Entity* self;
	Entity* other;
} EntityEvent;

typedef struct Game
{
	NB_FLT    cam_x, cam_y;
	NB_BOOL   btn[NB_BUTTON_COUNT];
	NB_BOOL   btn_prev[NB_BUTTON_COUNT];
	NB_BOOL   tiles[NB_ROOM_COLS][NB_ROOM_ROWS];
	NB_COL    screen[NB_WIDTH * NB_HEIGHT];
	Entity    entities[NB_ENTITIES];
} Game;

/* Globals */
extern Game nb_game;
extern const Entity nb_zero_entity;
extern const EntityType nb_entity_types[];

/* Game API */
void     nb_run          ();
Entity*  nb_en_create    (EntityType type, NB_INT x, NB_INT y);
NB_BOOL  nb_en_overlaps  (Entity* a, Entity* b);
NB_BOOL  nb_en_solids    (Entity* entity);
void     nb_en_destroy   (Entity* entity);
void     nb_clear        (NB_COL color);
void     nb_rect         (NB_INT x, NB_INT y, NB_INT w, NB_INT h, NB_COL color);
NB_BOOL  nb_down         (NB_UINT btn);
NB_BOOL  nb_pressed      (NB_UINT btn);
NB_BOOL  nb_released     (NB_UINT btn);

/* Math Functions */
#define  nb_min(x, y) ((x) < (y) ? (x) : (y))
#define  nb_max(x, y) ((x) > (y) ? (x) : (y))
#define  nb_appr(val, tar, delta) val = (val > (tar) ? nb_max(tar, val - delta) : nb_min(tar, val + delta))
#define  nb_sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/* Entity Types */
void     nb_player(EntityEvent*);
void     nb_coin(EntityEvent*);
void     nb_pop(EntityEvent*);

/* Platform API */
void     nb_platform_init();
NB_BOOL  nb_platform_poll();
void     nb_platform_present();

#endif