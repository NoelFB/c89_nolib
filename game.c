#include "game.h"
#include "spritesheet.h"

/* Platform Implementations */
#include "platform_win32.c"
#include "platform_x11.c"

/* Global Variables */
Game nb_game;
const Entity nb_zero_entity;
const EntityType nb_entity_types[] = {
	nb_player, nb_coin, nb_pop, NB_NULL
};

/* Begin Game API */

void nb_init()
{
	NB_INT i, j;

	/* Zero out entities */
	for (i = 0; i < NB_ENTITIES; i ++)
		nb_game.entities[i] = nb_zero_entity;

	/* Zero out tiles with solids along room edges */
	for (i = 0; i < NB_ROOM_COLS; i ++)
	for (j = 0; j < NB_ROOM_ROWS; j ++)
		nb_game.tiles[i][j] = (i == 0 || j == 0 || i == NB_ROOM_COLS - 1 || j == NB_ROOM_ROWS - 1);

	/* Default Palette */
	nb_game.palette[NB_COL_0] = 0x30436a;
	nb_game.palette[NB_COL_1] = 0xff8d78;
	nb_game.palette[NB_COL_2] = 0xe8c562;
	nb_game.palette[NB_COL_3] = 0xd4cdff;

	/* Create some entities */
	nb_en_create(nb_player, NB_ROOM_WIDTH / 2, NB_ROOM_HEIGHT / 2);
	for (i = 0; i < 7; i ++)
	{
		nb_en_create(nb_coin, 32 + i * 32, 32);
		nb_en_create(nb_coin, 32 + i * 32, NB_ROOM_HEIGHT - 32);
	}
}

void nb_step()
{
	NB_INT i, j, nx, ny;
	EntityEvent ev;

	/* run entities */
	ev.type = NB_ENTITY_UPDATE;
	for (i = 0; i < NB_ENTITIES; i ++)
	{
		ev.self = nb_game.entities + i;
		
		if (ev.self->type == NB_NULL)
			continue;

		ev.self->timer += 1.0f / 60.0f;

		/* get move amount */
		nx = ev.self->sx + ev.self->rx;
		ny = ev.self->sy + ev.self->ry;

		/* try to move */
		if ((ev.self->flags & NB_ENTITY_HITS_SOLIDS) != 0)
		{
			j = nx;
			while (j != 0)
			{
				ev.self->x += nb_sign(j);
				if (nb_en_solids(ev.self))
				{
					ev.self->sx = ev.self->rx = nx = 0;
					ev.self->x -= nb_sign(j);
					break;
				}
				j -= nb_sign(j);
			}

			j = ny;
			while (j != 0)
			{
				ev.self->y += nb_sign(j);
				if (nb_en_solids(ev.self))
				{
					ev.self->sy = ev.self->ry = ny = 0;
					ev.self->y -= nb_sign(j);
					break;
				}
				j -= nb_sign(j);
			}
		}
		else
		{
			ev.self->x += nx;
			ev.self->y += ny;
		}

		/* update move remainder */
		ev.self->rx = ev.self->sx + ev.self->rx - nx;
		ev.self->ry = ev.self->sy + ev.self->ry - ny;

		/* call their update event */
		ev.self->type(&ev);
	}

	/* do overlap tests */
	ev.type = NB_ENTITY_OVERLAP;
	for (i = 0; i < NB_ENTITIES; i ++)
	{
		ev.self = nb_game.entities + i;

		if (ev.self->type == NB_NULL || ev.self->bw < 0 || ev.self->bh <= 0 || (ev.self->flags & NB_ENTITY_OVERLAP_CHECKS) == 0)
			continue;

		for (j = 0; j < NB_ENTITIES; j ++)
		{
			ev.other = nb_game.entities + j;
			if (i == j || ev.other->type == NB_NULL || ev.other->bw < 0 || ev.other->bh <= 0 || (ev.other->flags & NB_ENTITY_OVERLAP_CHECKS) == 0)
				continue;
			if (nb_en_overlaps(ev.self, ev.other))
				ev.self->type(&ev);
		}
	}

	/* make sure camera is inside the room bounds */
	nb_game.cam_x = nb_max(0, nb_min(NB_ROOM_WIDTH - NB_WIDTH, nb_game.cam_x)); 
	nb_game.cam_y = nb_max(0, nb_min(NB_ROOM_HEIGHT - NB_HEIGHT, nb_game.cam_y)); 

	/* draw bg */
	nb_clear(NB_COL_0);
	for (i = nb_game.cam_x / 32; i <= (nb_game.cam_x + NB_WIDTH) / 32; i ++)
		nb_rect(i * 32, nb_game.cam_y, 1, NB_HEIGHT, NB_COL_1);
	for (i = nb_game.cam_y / 32; i <= (nb_game.cam_y + NB_HEIGHT) / 32; i ++)
		nb_rect(nb_game.cam_x, i * 32, NB_WIDTH, 1, NB_COL_1);

	/* draw solids */
	for (i = 0; i < NB_ROOM_COLS; i ++)
	for (j = 0; j < NB_ROOM_ROWS; j ++)
		if (nb_game.tiles[i][j])
			nb_rect(i * NB_GRID_SIZE, j * NB_GRID_SIZE, NB_GRID_SIZE, NB_GRID_SIZE, NB_COL_3);

	/* draw entities */
	ev.type = NB_ENTITY_DRAW;
	for (i = 0; i < NB_ENTITIES; i ++)
	{
		ev.self = nb_game.entities + i;
		if (ev.self->type != NB_NULL)
			ev.self->type(&ev);
	}

	/* store previous button state */
	for (i = 0; i < NB_BUTTON_COUNT; i ++)
		nb_game.btn_prev[i] = nb_game.btn[i];
}

Entity* nb_en_create(EntityType type, NB_INT x, NB_INT y)
{
	NB_UINT id = 0;
	EntityEvent ev;

	for (id = 0; id < NB_ENTITIES; id ++)
		if (nb_game.entities[id].type == NB_NULL)
		{
			ev.type = NB_ENTITY_INIT;
			ev.self = nb_game.entities + id;
			*ev.self = nb_zero_entity;
			ev.self->type = type;
			ev.self->x = x;
			ev.self->y = y;
			ev.self->timer = 0.0f;
			ev.self->type(&ev);
			break;
		}

	return (id >= 0 && id < NB_ENTITIES ? nb_game.entities + id : NB_NULL);
}

NB_BOOL nb_en_overlaps(Entity* a, Entity* b)
{
	if (a->x + a->bx < b->x + b->bx + b->bw &&
		a->y + a->by < b->y + b->by + b->bh &&
		a->x + a->bx + a->bw > b->x + b->bx &&
		a->y + a->by + a->bh > b->y + b->by)
		return NB_TRUE;
	return NB_FALSE;
}

NB_BOOL nb_en_solids(Entity* a)
{
	NB_INT l, r, t, b, x, y;
	l = nb_max(0, (a->x + a->bx) / NB_GRID_SIZE);
	r = nb_min(NB_ROOM_COLS - 1, (a->x + a->bx + a->bw - 1) / NB_GRID_SIZE);
	t = nb_max(0, (a->y + a->by) / NB_GRID_SIZE);
	b = nb_min(NB_ROOM_ROWS - 1, (a->y + a->by + a->bh - 1) / NB_GRID_SIZE);
	
	for (x = l; x <= r; x ++)
	for (y = t; y <= b; y ++)
		if (nb_game.tiles[x][y]) return NB_TRUE;
	return NB_FALSE;
}

void nb_en_destroy(Entity* entity)
{
	EntityEvent ev;
	NB_INT id = entity - nb_game.entities;
	if (id >= 0 && id < NB_ENTITIES && nb_game.entities[id].type != NB_NULL)
	{
		ev.type = NB_ENTITY_DESTROY;
		ev.self = nb_game.entities + id;
		ev.self->type(&ev);
		ev.self->type = NB_NULL;
	}
}

void nb_clear(NB_COL color)
{
	NB_COL* end = nb_game.screen + NB_WIDTH * NB_HEIGHT;
	NB_COL* at = nb_game.screen;

	while (at < end)
	{
		*at = color;
		at++;
	}
}

void nb_rect(NB_INT x, NB_INT y, NB_INT w, NB_INT h, NB_COL color)
{
	NB_INT px, py, l, t, r, b;

	x -= nb_game.cam_x; y -= nb_game.cam_y;
	l = nb_max(0, x);
	t = nb_max(0, y);
	r = nb_min(NB_WIDTH, x + w);
	b = nb_min(NB_HEIGHT, y + h);

	for (py = t; py < b; py ++)
	for (px = l; px < r; px ++)
		nb_game.screen[px + py * NB_WIDTH] = color;
}

void nb_spr(NB_INT x, NB_INT y, NB_INT spr_x, NB_INT spr_y, NB_INT spr_w, NB_INT spr_h)
{
	nb_spr_ext(x, y, spr_x, spr_y, spr_w, spr_h, NB_FALSE, NB_FALSE);
}

void nb_spr_ext(NB_INT x, NB_INT y, NB_INT spr_x, NB_INT spr_y, NB_INT spr_w, NB_INT spr_h, NB_BOOL flip_x, NB_BOOL flip_y)
{
	NB_INT px, py, dst_x, dst_y, src;

	x -= nb_game.cam_x; 
	y -= nb_game.cam_y;
	spr_x = nb_max(0, spr_x * NB_GRID_SIZE);
	spr_y = nb_max(0, spr_y * NB_GRID_SIZE);
	spr_w = nb_min(NB_SPRITESHEET_WIDTH - spr_x, spr_w * NB_GRID_SIZE);
	spr_h = nb_min(NB_SPRITESHEET_HEIGHT - spr_y, spr_h * NB_GRID_SIZE);
	flip_x = (flip_x ? 1 : 0);
	flip_y = (flip_y ? 1 : 0);

	/* todo: clamp this to screen bounds before we 
	 * begin iterating, to avoid the inner if statement 
	 */
	for (px = 0; px < spr_w; px++)
	for (py = 0; py < spr_h; py++)
	{
		dst_x = x + px;
		dst_y = y + py;
		src  = (spr_x + px + (spr_w - 2 * px) * flip_x);
		src += (spr_y + py + (spr_h - 2 * py) * flip_y) * NB_SPRITESHEET_WIDTH;
		if (dst_x >= 0 && dst_y >= 0 && dst_x < NB_WIDTH && dst_y < NB_HEIGHT && nb_spritesheet[src] < NB_PALETTE)
			nb_game.screen[dst_x + dst_y * NB_WIDTH] = nb_spritesheet[src];
	}
}

NB_BOOL nb_down(Buttons btn) { return nb_game.btn[btn]; }
NB_BOOL nb_pressed(Buttons btn) { return !nb_game.btn_prev[btn] && nb_game.btn[btn]; }
NB_BOOL nb_released(Buttons btn) { return nb_game.btn_prev[btn] && !nb_game.btn[btn]; }

/* Begin Entity Types */

void nb_player(EntityEvent* ev)
{
	const NB_FLT max_speed = 3;
	const NB_FLT accel = 20;
	const NB_FLT friction = 15;

	Entity* self = ev->self;
	NB_INT frame = 0;

	if (ev->type == NB_ENTITY_INIT)
	{
		self->bx = -4;
		self->by = -8;
		self->bw = 8;
		self->bh = 8;
		self->flags = NB_ENTITY_OVERLAP_CHECKS | NB_ENTITY_HITS_SOLIDS;
	}
	else if (ev->type == NB_ENTITY_UPDATE)
	{
		if (nb_down(NB_LEFT))
		{
			nb_appr(self->sx, -max_speed, accel * NB_DELTA);
			self->flags |= NB_ENTITY_FACING_LEFT;
		}
		else if (nb_down(NB_RIGHT))
		{
			nb_appr(self->sx, max_speed, accel * NB_DELTA);
			self->flags &= ~NB_ENTITY_FACING_LEFT;
		}
		else
			nb_appr(self->sx, 0, friction * NB_DELTA);

		if (nb_down(NB_UP))
			nb_appr(self->sy, -max_speed, accel * NB_DELTA);
		else if (nb_down(NB_DOWN))
			nb_appr(self->sy, max_speed, accel * NB_DELTA);
		else
			nb_appr(self->sy, 0, friction * NB_DELTA);

		/* make camera approach player */
		nb_game.cam_x += ((self->x + self->bx + self->bw / 2 - NB_WIDTH / 2) - nb_game.cam_x) / 10;
		nb_game.cam_y += ((self->y + self->by + self->bh / 2 - NB_HEIGHT / 2) - nb_game.cam_y) / 10;
	}
	else if (ev->type == NB_ENTITY_OVERLAP)
	{
		if (ev->other->type == nb_coin)
			nb_en_destroy(ev->other);
	}
	else if (ev->type == NB_ENTITY_DRAW)
	{
		frame = ((NB_INT)(self->timer * 8) % 2);
		if (self->sx == 0 && self->sy == 0) frame = 0;
		nb_spr_ext(self->x - 8, self->y - 16, frame * 2, 0, 2, 2, self->flags & NB_ENTITY_FACING_LEFT ? 1 : 0, NB_FALSE);
	}
}

void nb_coin(EntityEvent* ev)
{
	Entity* self = ev->self;
	Entity* other;
	NB_INT frame;

	if (ev->type == NB_ENTITY_INIT)
	{
		self->bx = -2;
		self->by = -2;
		self->bw = 4;
		self->bh = 4;
		self->flags = NB_ENTITY_OVERLAP_CHECKS;
	}
	else if (ev->type == NB_ENTITY_DRAW)
	{
		frame = ((NB_INT)(self->timer * 8) % 4);
		nb_spr_ext(self->x - 4, self->y - 4, 4 + frame % 2, 0, 1, 1, frame > 1, 0);
	}
	else if (ev->type == NB_ENTITY_DESTROY)
	{
		other = nb_en_create(nb_pop, self->x, self->y);
		other->sx = 4;
		other->sy = 4;
		other = nb_en_create(nb_pop, self->x, self->y);
		other->sx = -4;
		other->sy = 4;
		other = nb_en_create(nb_pop, self->x, self->y);
		other->sx = 4;
		other->sy = -4;
		other = nb_en_create(nb_pop, self->x, self->y);
		other->sx = -4;
		other->sy = -4;
	}
}

void nb_pop(EntityEvent* ev)
{
	Entity* self = ev->self;

	if (ev->type == NB_ENTITY_INIT)
	{
		self->bw = 0;
		self->bh = 0;
	}
	else if (ev->type == NB_ENTITY_UPDATE)
	{
		nb_appr(self->sx, 0, 32 * NB_DELTA);
		nb_appr(self->sy, 0, 32 * NB_DELTA);
		if (self->timer > 0.20f)
			nb_en_destroy(self);
	}
	else if (ev->type == NB_ENTITY_DRAW)
	{
		nb_rect(self->x - 1, self->y - 1, 2, 2, NB_COL_2);
	}
}
