#include "game.h"

/* Definitions */
#define WIDTH          160
#define HEIGHT         90
#define ROOM_WIDTH     256
#define ROOM_HEIGHT    256
#define ROOM_GRID      8
#define ROOM_COLUMNS   (ROOM_WIDTH / ROOM_GRID)
#define ROOM_ROWS      (ROOM_HEIGHT / ROOM_GRID)
#define SCREEN_SIZE    (WIDTH * HEIGHT * 3)
#define MAX_ENTITIES   64
#define FRAMERATE      60
#define DT             (1.0f / FRAMERATE)

/* Math Functions */
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define APPROACH(val, tar, delta) val = (val > tar ? MAX(tar, val - delta) : MIN(tar, val + delta))
#define SIGN(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

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
#define ENTITY_INIT    1
#define ENTITY_UPDATE  2
#define ENTITY_DRAW    3
#define ENTITY_DESTROY 4
#define ENTITY_OVERLAP 5
typedef struct EntityEvent
{
	NB_UINT type;
	Entity* self;
	Entity* other;
} EntityEvent;

/* Entity Types */
void player(EntityEvent*);
void coin(EntityEvent*);
void pop(EntityEvent*);

/* Not used yet, but this would let map data use an index for entity type */
EntityType entity_types[] = {
	player, coin, pop
};

/* Game State */
NB_COL   screen[SCREEN_SIZE];
NB_BOOL  btns_last[BTN_COUNT];
NB_BOOL  btns_next[BTN_COUNT];
Entity   entities[MAX_ENTITIES];
NB_BOOL  solids[ROOM_COLUMNS][ROOM_ROWS];
NB_FLT   cam_x;
NB_FLT   cam_y;
const Entity zeroed_entity;

/* Game API */
Entity*   entity_create    (EntityType type, NB_INT x, NB_INT y);
NB_BOOL   entity_overlaps  (Entity* a, Entity* b);
NB_BOOL   entity_solids    (Entity* a);
void      entity_destroy   (Entity* entity);
void      draw_clear       (NB_COL color);
void      draw_rect        (NB_INT x, NB_INT y, NB_INT w, NB_INT h, NB_COL color);
NB_BOOL   is_down          (NB_UINT btn);
NB_BOOL   is_pressed       (NB_UINT btn);
NB_BOOL   is_released      (NB_UINT btn);

void game_entry()
{
	NB_INT i, j, nx, ny;
	EntityEvent ev;

	platform_init("Game", FRAMERATE, WIDTH, HEIGHT, screen, btns_next);

	/* Create some entities */
	entity_create(player, 32, 32);
	for (i = 0; i < 6; i ++)
		entity_create(coin, 64 + i * 32, 32);

	/* Add solids along room edges */
	for (i = 0; i < ROOM_COLUMNS; i ++)
	for (j = 0; j < ROOM_ROWS; j ++)
		solids[i][j] = (i == 0 || j == 0 || i == ROOM_COLUMNS - 1 || j == ROOM_ROWS - 1);

	while (NB_TRUE)
	{
		/* update state */
		for (i = 0; i < BTN_COUNT; i ++)
			btns_last[i] = btns_next[i];
		if (!platform_poll())
			break;

		/* run entities */
		ev.type = ENTITY_UPDATE;
		for (i = 0; i < MAX_ENTITIES; i ++)
		{
			if (entities[i].type == NB_NULL)
				continue;

			entities[i].timer += 1.0f / 60.0f;

			/* get move amount */
			nx = entities[i].sx + entities[i].rx;
			ny = entities[i].sy + entities[i].ry;

			/* try to move */
			if (entities[i].solids)
			{
				j = nx;
				while (j != 0)
				{
					entities[i].x += SIGN(j);
					if (entity_solids(entities + i))
					{
						entities[i].sx = entities[i].rx = nx = 0;
						entities[i].x -= SIGN(j);
						break;
					}
					j -= SIGN(j);
				}

				j = ny;
				while (j != 0)
				{
					entities[i].y += SIGN(j);
					if (entity_solids(entities + i))
					{
						entities[i].sy = entities[i].ry = ny = 0;
						entities[i].y -= SIGN(j);
						break;
					}
					j -= SIGN(j);
				}
			}
			else
			{
				entities[i].x += nx;
				entities[i].y += ny;
			}

			/* update move remainder */
			entities[i].rx = entities[i].sx + entities[i].rx - nx;
			entities[i].ry = entities[i].sy + entities[i].ry - ny;

			/* call their update event */
			ev.self = entities + i;
			entities[i].type(&ev);
		}

		/* do overlap tests */
		ev.type = ENTITY_OVERLAP;
		for (i = 0; i < MAX_ENTITIES; i ++)
		{
			if (entities[i].type == NB_NULL || entities[i].bw < 0 || entities[i].bh <= 0 || !entities[i].overlaps)
				continue;
			ev.self = entities + i;

			for (j = 0; j < MAX_ENTITIES; j ++)
			{
				if (i == j || entities[j].type == NB_NULL || entities[j].bw < 0 || entities[j].bh <= 0 || !entities[i].overlaps)
					continue;
				ev.other = entities + j;
				if (entity_overlaps(entities + i, entities + j))
					entities[i].type(&ev);
			}
		}

		/* make sure camera is inside the room bounds */
		cam_x = MAX(0, MIN(ROOM_WIDTH - WIDTH, cam_x)); 
		cam_y = MAX(0, MIN(ROOM_HEIGHT - HEIGHT, cam_y)); 

		/* draw bg */
		draw_clear(0x222034);
		for (i = cam_x / 32; i <= (cam_x + WIDTH) / 32; i ++)
			draw_rect(i * 32, cam_y, 1, HEIGHT, 0x444444);
		for (i = cam_y / 32; i <= (cam_y + HEIGHT) / 32; i ++)
			draw_rect(cam_x, i * 32, WIDTH, 1, 0x444444);

		/* draw solids */
		for (i = 0; i < ROOM_COLUMNS; i ++)
		for (j = 0; j < ROOM_ROWS; j ++)
			if (solids[i][j])
				draw_rect(i * ROOM_GRID, j * ROOM_GRID, ROOM_GRID, ROOM_GRID, 0xffffff);

		/* draw entities */
		ev.type = ENTITY_DRAW;
		for (i = 0; i < MAX_ENTITIES; i ++)
		{
			ev.self = entities + i;
			if (entities[i].type != NB_NULL)
				entities[i].type(&ev);
		}

		/* present screen */
		platform_present();
	}
}

/* Entities */
void player(EntityEvent* ev)
{
	const NB_FLT max_speed = 3;
	const NB_FLT accel = 20;
	const NB_FLT friction = 15;

	Entity* self = ev->self;
	NB_FLT p = (1 - MIN(self->timer * 8, 1)) * 3;

	if (ev->type == ENTITY_INIT)
	{
		self->bx = -4;
		self->by = -8;
		self->bw = 8;
		self->bh = 8;
		self->solids = NB_TRUE;
		self->overlaps = NB_TRUE;
	}
	else if (ev->type == ENTITY_UPDATE)
	{
		if (is_down(BTN_LEFT))
			APPROACH(self->sx, -max_speed, accel * DT);
		else if (is_down(BTN_RIGHT))
			APPROACH(self->sx, max_speed, accel * DT);
		else
			APPROACH(self->sx, 0, friction * DT);

		if (is_down(BTN_UP))
			APPROACH(self->sy, -max_speed, accel * DT);
		else if (is_down(BTN_DOWN))
			APPROACH(self->sy, max_speed, accel * DT);
		else
			APPROACH(self->sy, 0, friction * DT);

		/* make camera approach player */
		cam_x += ((self->x + self->bx + self->bw / 2 - WIDTH / 2) - cam_x) / 10;
		cam_y += ((self->y + self->by + self->bh / 2 - HEIGHT / 2) - cam_y) / 10;
	}
	else if (ev->type == ENTITY_OVERLAP)
	{
		if (ev->other->type == coin)
		{
			self->timer = 0;
			entity_destroy(ev->other);
		}
	}
	else if (ev->type == ENTITY_DRAW)
	{
		draw_rect(self->x + self->bx - p, self->y + self->by - p, self->bw + p * 2, self->bh + p * 2, 0x6abe30);
	}
}

void coin(EntityEvent* ev)
{
	Entity* self = ev->self;
	Entity* other;

	if (ev->type == ENTITY_INIT)
	{
		self->bx = -2;
		self->by = -2;
		self->bw = 4;
		self->bh = 4;
		self->overlaps = NB_TRUE;
	}
	else if (ev->type == ENTITY_DRAW)
	{
		draw_rect(self->x + self->bx, self->y + self->by, self->bw, self->bh, 0xfbf236);
	}
	else if (ev->type == ENTITY_DESTROY)
	{
		other = entity_create(pop, self->x, self->y);
		other->sx = 4;
		other->sy = 4;
		other = entity_create(pop, self->x, self->y);
		other->sx = -4;
		other->sy = 4;
		other = entity_create(pop, self->x, self->y);
		other->sx = 4;
		other->sy = -4;
		other = entity_create(pop, self->x, self->y);
		other->sx = -4;
		other->sy = -4;
	}
}

void pop(EntityEvent* ev)
{
	Entity* self = ev->self;

	if (ev->type == ENTITY_INIT)
	{
		self->bw = 0;
		self->bh = 0;
	}
	else if (ev->type == ENTITY_UPDATE)
	{
		APPROACH(self->sx, 0, 32 * DT);
		APPROACH(self->sy, 0, 32 * DT);
		if (self->timer > 0.20f)
			entity_destroy(self);
	}
	else if (ev->type == ENTITY_DRAW)
	{
		draw_rect(self->x - 1, self->y - 1, 2, 2, 0xfbf236);
	}
}

Entity* entity_create(EntityType type, NB_INT x, NB_INT y)
{
	NB_UINT id = 0;
	EntityEvent ev;

	for (id = 0; id < MAX_ENTITIES; id ++)
		if (entities[id].type == NB_NULL)
		{
			ev.type = ENTITY_INIT;
			ev.self = entities + id;
			entities[id] = zeroed_entity;
			entities[id].type = type;
			entities[id].x = x;
			entities[id].y = y;
			entities[id].timer = 0.0f;
			entities[id].type(&ev);
			break;
		}

	return (id >= 0 && id < MAX_ENTITIES ? entities + id : NB_NULL);
}

NB_BOOL entity_overlaps(Entity* a, Entity* b)
{
	if (a->x + a->bx < b->x + b->bx + b->bw &&
		a->y + a->by < b->y + b->by + b->bh &&
		a->x + a->bx + a->bw > b->x + b->bx &&
		a->y + a->by + a->bh > b->y + b->by)
		return NB_TRUE;
	return NB_FALSE;
}

NB_BOOL entity_solids(Entity* a)
{
	NB_INT l, r, t, b, x, y;
	l = MAX(0, (a->x + a->bx) / ROOM_GRID);
	r = MIN(ROOM_COLUMNS - 1, (a->x + a->bx + a->bw - 1) / ROOM_GRID);
	t = MAX(0, (a->y + a->by) / ROOM_GRID);
	b = MIN(ROOM_ROWS - 1, (a->y + a->by + a->bh - 1) / ROOM_GRID);
	
	for (x = l; x <= r; x ++)
	for (y = t; y <= b; y ++)
		if (solids[x][y]) return NB_TRUE;
	return NB_FALSE;
}

void entity_destroy(Entity* entity)
{
	EntityEvent ev;
	NB_INT id = entity - entities;
	if (id >= 0 && id < MAX_ENTITIES && entities[id].type != NB_NULL)
	{
		ev.type = ENTITY_DESTROY;
		ev.self = entities + id;
		entities[id].type(&ev);
		entities[id].type = NB_NULL;
	}
}

void draw_clear(NB_COL color)
{
	NB_COL* end = screen + WIDTH * HEIGHT;
	NB_COL* at = screen;

	while (at < end)
	{
		*at = color;
		at++;
	}
}

void draw_rect(NB_INT x, NB_INT y, NB_INT w, NB_INT h, NB_COL color)
{
	NB_INT px, py, l, t, r, b;

	x -= cam_x; y -= cam_y;
	l = MAX(0, x);
	t = MAX(0, y);
	r = MIN(WIDTH, x + w);
	b = MIN(HEIGHT, y + h);

	for (py = t; py < b; py ++)
	for (px = l; px < r; px ++)
		screen[px + py * WIDTH] = color;
}

NB_BOOL is_down (NB_UINT btn) { return btns_next[btn]; }
NB_BOOL is_pressed (NB_UINT btn) { return !btns_last[btn] && btns_next[btn]; }
NB_BOOL is_released (NB_UINT btn) { return btns_last[btn] && !btns_next[btn]; }
