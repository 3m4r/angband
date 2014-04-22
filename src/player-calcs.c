/** \file player-calcs.c
	\brief Player status calculation, signalling ui events based on 
	status changes.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2014 Nick McConnell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cave.h"
#include "dungeon.h"
#include "files.h"
#include "game-event.h"
#include "mon-msg.h"
#include "mon-util.h"
#include "obj-identify.h"
#include "obj-tval.h"
#include "obj-tvalsval.h"
#include "obj-util.h"
#include "player-timed.h"
#include "player-util.h"
#include "spells.h"
#include "squelch.h"
#include "ui.h"

/*
 * Stat Table (INT) -- Magic devices
 */
const byte adj_int_dev[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	1	/* 8 */,
	1	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	2	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	3	/* 18/00-18/09 */,
	3	/* 18/10-18/19 */,
	3	/* 18/20-18/29 */,
	3	/* 18/30-18/39 */,
	3	/* 18/40-18/49 */,
	4	/* 18/50-18/59 */,
	4	/* 18/60-18/69 */,
	5	/* 18/70-18/79 */,
	5	/* 18/80-18/89 */,
	6	/* 18/90-18/99 */,
	6	/* 18/100-18/109 */,
	7	/* 18/110-18/119 */,
	7	/* 18/120-18/129 */,
	8	/* 18/130-18/139 */,
	8	/* 18/140-18/149 */,
	9	/* 18/150-18/159 */,
	9	/* 18/160-18/169 */,
	10	/* 18/170-18/179 */,
	10	/* 18/180-18/189 */,
	11	/* 18/190-18/199 */,
	11	/* 18/200-18/209 */,
	12	/* 18/210-18/219 */,
	13	/* 18/220+ */
};

/*
 * Stat Table (WIS) -- Saving throw
 */
const byte adj_wis_sav[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	1	/* 8 */,
	1	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	2	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	3	/* 18/00-18/09 */,
	3	/* 18/10-18/19 */,
	3	/* 18/20-18/29 */,
	3	/* 18/30-18/39 */,
	3	/* 18/40-18/49 */,
	4	/* 18/50-18/59 */,
	4	/* 18/60-18/69 */,
	5	/* 18/70-18/79 */,
	5	/* 18/80-18/89 */,
	6	/* 18/90-18/99 */,
	7	/* 18/100-18/109 */,
	8	/* 18/110-18/119 */,
	9	/* 18/120-18/129 */,
	10	/* 18/130-18/139 */,
	11	/* 18/140-18/149 */,
	12	/* 18/150-18/159 */,
	13	/* 18/160-18/169 */,
	14	/* 18/170-18/179 */,
	15	/* 18/180-18/189 */,
	16	/* 18/190-18/199 */,
	17	/* 18/200-18/209 */,
	18	/* 18/210-18/219 */,
	19	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- disarming
 */
const byte adj_dex_dis[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	0	/* 10 */,
	0	/* 11 */,
	0	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	1	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	4	/* 18/00-18/09 */,
	4	/* 18/10-18/19 */,
	4	/* 18/20-18/29 */,
	4	/* 18/30-18/39 */,
	5	/* 18/40-18/49 */,
	5	/* 18/50-18/59 */,
	5	/* 18/60-18/69 */,
	6	/* 18/70-18/79 */,
	6	/* 18/80-18/89 */,
	7	/* 18/90-18/99 */,
	8	/* 18/100-18/109 */,
	8	/* 18/110-18/119 */,
	8	/* 18/120-18/129 */,
	8	/* 18/130-18/139 */,
	8	/* 18/140-18/149 */,
	9	/* 18/150-18/159 */,
	9	/* 18/160-18/169 */,
	9	/* 18/170-18/179 */,
	9	/* 18/180-18/189 */,
	9	/* 18/190-18/199 */,
	10	/* 18/200-18/209 */,
	10	/* 18/210-18/219 */,
	10	/* 18/220+ */
};


/*
 * Stat Table (INT) -- disarming
 */
const byte adj_int_dis[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	1	/* 8 */,
	1	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	2	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	3	/* 18/00-18/09 */,
	3	/* 18/10-18/19 */,
	3	/* 18/20-18/29 */,
	4	/* 18/30-18/39 */,
	4	/* 18/40-18/49 */,
	5	/* 18/50-18/59 */,
	6	/* 18/60-18/69 */,
	7	/* 18/70-18/79 */,
	8	/* 18/80-18/89 */,
	9	/* 18/90-18/99 */,
	10	/* 18/100-18/109 */,
	10	/* 18/110-18/119 */,
	11	/* 18/120-18/129 */,
	12	/* 18/130-18/139 */,
	13	/* 18/140-18/149 */,
	14	/* 18/150-18/159 */,
	15	/* 18/160-18/169 */,
	16	/* 18/170-18/179 */,
	17	/* 18/180-18/189 */,
	18	/* 18/190-18/199 */,
	19	/* 18/200-18/209 */,
	19	/* 18/210-18/219 */,
	19	/* 18/220+ */
};

/*
 * Stat Table (DEX) -- bonus to ac (plus 128)
 */
const byte adj_dex_ta[STAT_RANGE] =
{
	128 + -4	/* 3 */,
	128 + -3	/* 4 */,
	128 + -2	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 1	/* 15 */,
	128 + 1	/* 16 */,
	128 + 1	/* 17 */,
	128 + 2	/* 18/00-18/09 */,
	128 + 2	/* 18/10-18/19 */,
	128 + 2	/* 18/20-18/29 */,
	128 + 2	/* 18/30-18/39 */,
	128 + 2	/* 18/40-18/49 */,
	128 + 3	/* 18/50-18/59 */,
	128 + 3	/* 18/60-18/69 */,
	128 + 3	/* 18/70-18/79 */,
	128 + 4	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};

/*
 * Stat Table (STR) -- bonus to dam (plus 128)
 */
const byte adj_str_td[STAT_RANGE] =
{
	128 + -2	/* 3 */,
	128 + -2	/* 4 */,
	128 + -1	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 1	/* 16 */,
	128 + 2	/* 17 */,
	128 + 2	/* 18/00-18/09 */,
	128 + 2	/* 18/10-18/19 */,
	128 + 3	/* 18/20-18/29 */,
	128 + 3	/* 18/30-18/39 */,
	128 + 3	/* 18/40-18/49 */,
	128 + 3	/* 18/50-18/59 */,
	128 + 3	/* 18/60-18/69 */,
	128 + 4	/* 18/70-18/79 */,
	128 + 5	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 10	/* 18/140-18/149 */,
	128 + 11	/* 18/150-18/159 */,
	128 + 12	/* 18/160-18/169 */,
	128 + 13	/* 18/170-18/179 */,
	128 + 14	/* 18/180-18/189 */,
	128 + 15	/* 18/190-18/199 */,
	128 + 16	/* 18/200-18/209 */,
	128 + 18	/* 18/210-18/219 */,
	128 + 20	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- bonus to hit (plus 128)
 */
const byte adj_dex_th[STAT_RANGE] =
{
	128 + -3	/* 3 */,
	128 + -2	/* 4 */,
	128 + -2	/* 5 */,
	128 + -1	/* 6 */,
	128 + -1	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 1	/* 16 */,
	128 + 2	/* 17 */,
	128 + 3	/* 18/00-18/09 */,
	128 + 3	/* 18/10-18/19 */,
	128 + 3	/* 18/20-18/29 */,
	128 + 3	/* 18/30-18/39 */,
	128 + 3	/* 18/40-18/49 */,
	128 + 4	/* 18/50-18/59 */,
	128 + 4	/* 18/60-18/69 */,
	128 + 4	/* 18/70-18/79 */,
	128 + 4	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};


/*
 * Stat Table (STR) -- bonus to hit (plus 128)
 */
const byte adj_str_th[STAT_RANGE] =
{
	128 + -3	/* 3 */,
	128 + -2	/* 4 */,
	128 + -1	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 0	/* 16 */,
	128 + 0	/* 17 */,
	128 + 1	/* 18/00-18/09 */,
	128 + 1	/* 18/10-18/19 */,
	128 + 1	/* 18/20-18/29 */,
	128 + 1	/* 18/30-18/39 */,
	128 + 1	/* 18/40-18/49 */,
	128 + 1	/* 18/50-18/59 */,
	128 + 1	/* 18/60-18/69 */,
	128 + 2	/* 18/70-18/79 */,
	128 + 3	/* 18/80-18/89 */,
	128 + 4	/* 18/90-18/99 */,
	128 + 5	/* 18/100-18/109 */,
	128 + 6	/* 18/110-18/119 */,
	128 + 7	/* 18/120-18/129 */,
	128 + 8	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};


/*
 * Stat Table (STR) -- weight limit in deca-pounds
 */
const byte adj_str_wgt[STAT_RANGE] =
{
	5	/* 3 */,
	6	/* 4 */,
	7	/* 5 */,
	8	/* 6 */,
	9	/* 7 */,
	10	/* 8 */,
	11	/* 9 */,
	12	/* 10 */,
	13	/* 11 */,
	14	/* 12 */,
	15	/* 13 */,
	16	/* 14 */,
	17	/* 15 */,
	18	/* 16 */,
	19	/* 17 */,
	20	/* 18/00-18/09 */,
	22	/* 18/10-18/19 */,
	24	/* 18/20-18/29 */,
	26	/* 18/30-18/39 */,
	28	/* 18/40-18/49 */,
	30	/* 18/50-18/59 */,
	30	/* 18/60-18/69 */,
	30	/* 18/70-18/79 */,
	30	/* 18/80-18/89 */,
	30	/* 18/90-18/99 */,
	30	/* 18/100-18/109 */,
	30	/* 18/110-18/119 */,
	30	/* 18/120-18/129 */,
	30	/* 18/130-18/139 */,
	30	/* 18/140-18/149 */,
	30	/* 18/150-18/159 */,
	30	/* 18/160-18/169 */,
	30	/* 18/170-18/179 */,
	30	/* 18/180-18/189 */,
	30	/* 18/190-18/199 */,
	30	/* 18/200-18/209 */,
	30	/* 18/210-18/219 */,
	30	/* 18/220+ */
};


/*
 * Stat Table (STR) -- weapon weight limit in pounds
 */
const byte adj_str_hold[STAT_RANGE] =
{
	4	/* 3 */,
	5	/* 4 */,
	6	/* 5 */,
	7	/* 6 */,
	8	/* 7 */,
	10	/* 8 */,
	12	/* 9 */,
	14	/* 10 */,
	16	/* 11 */,
	18	/* 12 */,
	20	/* 13 */,
	22	/* 14 */,
	24	/* 15 */,
	26	/* 16 */,
	28	/* 17 */,
	30	/* 18/00-18/09 */,
	30	/* 18/10-18/19 */,
	35	/* 18/20-18/29 */,
	40	/* 18/30-18/39 */,
	45	/* 18/40-18/49 */,
	50	/* 18/50-18/59 */,
	55	/* 18/60-18/69 */,
	60	/* 18/70-18/79 */,
	65	/* 18/80-18/89 */,
	70	/* 18/90-18/99 */,
	80	/* 18/100-18/109 */,
	80	/* 18/110-18/119 */,
	80	/* 18/120-18/129 */,
	80	/* 18/130-18/139 */,
	80	/* 18/140-18/149 */,
	90	/* 18/150-18/159 */,
	90	/* 18/160-18/169 */,
	90	/* 18/170-18/179 */,
	90	/* 18/180-18/189 */,
	90	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (STR) -- digging value
 */
const byte adj_str_dig[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	1	/* 5 */,
	2	/* 6 */,
	3	/* 7 */,
	4	/* 8 */,
	4	/* 9 */,
	5	/* 10 */,
	5	/* 11 */,
	6	/* 12 */,
	6	/* 13 */,
	7	/* 14 */,
	7	/* 15 */,
	8	/* 16 */,
	8	/* 17 */,
	9	/* 18/00-18/09 */,
	10	/* 18/10-18/19 */,
	12	/* 18/20-18/29 */,
	15	/* 18/30-18/39 */,
	20	/* 18/40-18/49 */,
	25	/* 18/50-18/59 */,
	30	/* 18/60-18/69 */,
	35	/* 18/70-18/79 */,
	40	/* 18/80-18/89 */,
	45	/* 18/90-18/99 */,
	50	/* 18/100-18/109 */,
	55	/* 18/110-18/119 */,
	60	/* 18/120-18/129 */,
	65	/* 18/130-18/139 */,
	70	/* 18/140-18/149 */,
	75	/* 18/150-18/159 */,
	80	/* 18/160-18/169 */,
	85	/* 18/170-18/179 */,
	90	/* 18/180-18/189 */,
	95	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (STR) -- help index into the "blow" table
 */
const byte adj_str_blow[STAT_RANGE] =
{
	3	/* 3 */,
	4	/* 4 */,
	5	/* 5 */,
	6	/* 6 */,
	7	/* 7 */,
	8	/* 8 */,
	9	/* 9 */,
	10	/* 10 */,
	11	/* 11 */,
	12	/* 12 */,
	13	/* 13 */,
	14	/* 14 */,
	15	/* 15 */,
	16	/* 16 */,
	17	/* 17 */,
	20 /* 18/00-18/09 */,
	30 /* 18/10-18/19 */,
	40 /* 18/20-18/29 */,
	50 /* 18/30-18/39 */,
	60 /* 18/40-18/49 */,
	70 /* 18/50-18/59 */,
	80 /* 18/60-18/69 */,
	90 /* 18/70-18/79 */,
	100 /* 18/80-18/89 */,
	110 /* 18/90-18/99 */,
	120 /* 18/100-18/109 */,
	130 /* 18/110-18/119 */,
	140 /* 18/120-18/129 */,
	150 /* 18/130-18/139 */,
	160 /* 18/140-18/149 */,
	170 /* 18/150-18/159 */,
	180 /* 18/160-18/169 */,
	190 /* 18/170-18/179 */,
	200 /* 18/180-18/189 */,
	210 /* 18/190-18/199 */,
	220 /* 18/200-18/209 */,
	230 /* 18/210-18/219 */,
	240 /* 18/220+ */
};


/*
 * Stat Table (DEX) -- index into the "blow" table
 */
const byte adj_dex_blow[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	1	/* 15 */,
	1	/* 16 */,
	2	/* 17 */,
	2	/* 18/00-18/09 */,
	2	/* 18/10-18/19 */,
	3	/* 18/20-18/29 */,
	3	/* 18/30-18/39 */,
	4	/* 18/40-18/49 */,
	4	/* 18/50-18/59 */,
	5	/* 18/60-18/69 */,
	5	/* 18/70-18/79 */,
	6	/* 18/80-18/89 */,
	6	/* 18/90-18/99 */,
	7	/* 18/100-18/109 */,
	7	/* 18/110-18/119 */,
	8	/* 18/120-18/129 */,
	8	/* 18/130-18/139 */,
	8	/* 18/140-18/149 */,
	9	/* 18/150-18/159 */,
	9	/* 18/160-18/169 */,
	9	/* 18/170-18/179 */,
	10	/* 18/180-18/189 */,
	10	/* 18/190-18/199 */,
	11	/* 18/200-18/209 */,
	11	/* 18/210-18/219 */,
	11	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- chance of avoiding "theft" and "falling"
 */
const byte adj_dex_safe[STAT_RANGE] =
{
	0	/* 3 */,
	1	/* 4 */,
	2	/* 5 */,
	3	/* 6 */,
	4	/* 7 */,
	5	/* 8 */,
	5	/* 9 */,
	6	/* 10 */,
	6	/* 11 */,
	7	/* 12 */,
	7	/* 13 */,
	8	/* 14 */,
	8	/* 15 */,
	9	/* 16 */,
	9	/* 17 */,
	10	/* 18/00-18/09 */,
	10	/* 18/10-18/19 */,
	15	/* 18/20-18/29 */,
	15	/* 18/30-18/39 */,
	20	/* 18/40-18/49 */,
	25	/* 18/50-18/59 */,
	30	/* 18/60-18/69 */,
	35	/* 18/70-18/79 */,
	40	/* 18/80-18/89 */,
	45	/* 18/90-18/99 */,
	50	/* 18/100-18/109 */,
	60	/* 18/110-18/119 */,
	70	/* 18/120-18/129 */,
	80	/* 18/130-18/139 */,
	90	/* 18/140-18/149 */,
	100	/* 18/150-18/159 */,
	100	/* 18/160-18/169 */,
	100	/* 18/170-18/179 */,
	100	/* 18/180-18/189 */,
	100	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (CON) -- base regeneration rate
 */
const byte adj_con_fix[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	0	/* 10 */,
	0	/* 11 */,
	0	/* 12 */,
	0	/* 13 */,
	1	/* 14 */,
	1	/* 15 */,
	1	/* 16 */,
	1	/* 17 */,
	2	/* 18/00-18/09 */,
	2	/* 18/10-18/19 */,
	2	/* 18/20-18/29 */,
	2	/* 18/30-18/39 */,
	2	/* 18/40-18/49 */,
	3	/* 18/50-18/59 */,
	3	/* 18/60-18/69 */,
	3	/* 18/70-18/79 */,
	3	/* 18/80-18/89 */,
	3	/* 18/90-18/99 */,
	4	/* 18/100-18/109 */,
	4	/* 18/110-18/119 */,
	5	/* 18/120-18/129 */,
	6	/* 18/130-18/139 */,
	6	/* 18/140-18/149 */,
	7	/* 18/150-18/159 */,
	7	/* 18/160-18/169 */,
	8	/* 18/170-18/179 */,
	8	/* 18/180-18/189 */,
	8	/* 18/190-18/199 */,
	9	/* 18/200-18/209 */,
	9	/* 18/210-18/219 */,
	9	/* 18/220+ */
};


/*
 * Stat Table (CON) -- extra 1/100th hitpoints per level
 */
const int adj_con_mhp[STAT_RANGE] =
{
	-250	/* 3 */,
	-150	/* 4 */,
	-100	/* 5 */,
	 -75	/* 6 */,
	 -50	/* 7 */,
	 -25	/* 8 */,
	 -10	/* 9 */,
	  -5	/* 10 */,
	   0	/* 11 */,
	   5	/* 12 */,
	  10	/* 13 */,
	  25	/* 14 */,
	  50	/* 15 */,
	  75	/* 16 */,
	 100	/* 17 */,
	 150	/* 18/00-18/09 */,
	 175	/* 18/10-18/19 */,
	 200	/* 18/20-18/29 */,
	 225	/* 18/30-18/39 */,
	 250	/* 18/40-18/49 */,
	 275	/* 18/50-18/59 */,
	 300	/* 18/60-18/69 */,
	 350	/* 18/70-18/79 */,
	 400	/* 18/80-18/89 */,
	 450	/* 18/90-18/99 */,
	 500	/* 18/100-18/109 */,
	 550	/* 18/110-18/119 */,
	 600	/* 18/120-18/129 */,
	 650	/* 18/130-18/139 */,
	 700	/* 18/140-18/149 */,
	 750	/* 18/150-18/159 */,
	 800	/* 18/160-18/169 */,
	 900	/* 18/170-18/179 */,
	1000	/* 18/180-18/189 */,
	1100	/* 18/190-18/199 */,
	1250	/* 18/200-18/209 */,
	1250	/* 18/210-18/219 */,
	1250	/* 18/220+ */
};

const int adj_mag_study[STAT_RANGE] =
{
	  0	/* 3 */,
	  0	/* 4 */,
	 10	/* 5 */,
	 20	/* 6 */,
	 30	/* 7 */,
	 40	/* 8 */,
	 50	/* 9 */,
	 60	/* 10 */,
	 70	/* 11 */,
	 80	/* 12 */,
	 85	/* 13 */,
	 90	/* 14 */,
	 95	/* 15 */,
	100	/* 16 */,
	105	/* 17 */,
	110	/* 18/00-18/09 */,
	115	/* 18/10-18/19 */,
	120	/* 18/20-18/29 */,
	130	/* 18/30-18/39 */,
	140	/* 18/40-18/49 */,
	150	/* 18/50-18/59 */,
	160	/* 18/60-18/69 */,
	170	/* 18/70-18/79 */,
	180	/* 18/80-18/89 */,
	190	/* 18/90-18/99 */,
	200	/* 18/100-18/109 */,
	210	/* 18/110-18/119 */,
	220	/* 18/120-18/129 */,
	230	/* 18/130-18/139 */,
	240	/* 18/140-18/149 */,
	250	/* 18/150-18/159 */,
	250	/* 18/160-18/169 */,
	250	/* 18/170-18/179 */,
	250	/* 18/180-18/189 */,
	250	/* 18/190-18/199 */,
	250	/* 18/200-18/209 */,
	250	/* 18/210-18/219 */,
	250	/* 18/220+ */
};

/*
 * Stat Table (INT/WIS) -- extra 1/100 mana-points per level
 */
const int adj_mag_mana[STAT_RANGE] =
{
	  0	/* 3 */,
	 10	/* 4 */,
	 20	/* 5 */,
	 30	/* 6 */,
	 40	/* 7 */,
	 50	/* 8 */,
	 60	/* 9 */,
	 70	/* 10 */,
	 80	/* 11 */,
	 90	/* 12 */,
	100	/* 13 */,
	110	/* 14 */,
	120	/* 15 */,
	130	/* 16 */,
	140	/* 17 */,
	150	/* 18/00-18/09 */,
	160	/* 18/10-18/19 */,
	170	/* 18/20-18/29 */,
	180	/* 18/30-18/39 */,
	190	/* 18/40-18/49 */,
	200	/* 18/50-18/59 */,
	225	/* 18/60-18/69 */,
	250	/* 18/70-18/79 */,
	300	/* 18/80-18/89 */,
	350	/* 18/90-18/99 */,
	400	/* 18/100-18/109 */,
	450	/* 18/110-18/119 */,
	500	/* 18/120-18/129 */,
	550	/* 18/130-18/139 */,
	600	/* 18/140-18/149 */,
	650	/* 18/150-18/159 */,
	700	/* 18/160-18/169 */,
	750	/* 18/170-18/179 */,
	800	/* 18/180-18/189 */,
	800	/* 18/190-18/199 */,
	800	/* 18/200-18/209 */,
	800	/* 18/210-18/219 */,
	800	/* 18/220+ */
};

/*
 * This table is used to help calculate the number of blows the player can
 * make in a single round of attacks (one player turn) with a normal weapon.
 *
 * This number ranges from a single blow/round for weak players to up to six
 * blows/round for powerful warriors.
 *
 * Note that certain artifacts and ego-items give "bonus" blows/round.
 *
 * First, from the player class, we extract some values:
 *
 *    Warrior --> num = 6; mul = 5; div = MAX(30, weapon_weight);
 *    Mage    --> num = 4; mul = 2; div = MAX(40, weapon_weight);
 *    Priest  --> num = 4; mul = 3; div = MAX(35, weapon_weight);
 *    Rogue   --> num = 5; mul = 4; div = MAX(30, weapon_weight);
 *    Ranger  --> num = 5; mul = 4; div = MAX(35, weapon_weight);
 *    Paladin --> num = 5; mul = 5; div = MAX(30, weapon_weight);
 * (all specified in p_class.txt now)
 *
 * To get "P", we look up the relevant "adj_str_blow[]" (see above),
 * multiply it by "mul", and then divide it by "div", rounding down.
 *
 * To get "D", we look up the relevant "adj_dex_blow[]" (see above).
 *
 * Then we look up the energy cost of each blow using "blows_table[P][D]".
 * The player gets blows/round equal to 100/this number, up to a maximum of
 * "num" blows/round, plus any "bonus" blows/round.
 */
const byte blows_table[12][12] =
{
	/* P */
   /* D:   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11+ */
   /* DEX: 3,   10,  17,  /20, /40, /60, /80, /100,/120,/150,/180,/200 */

	/* 0  */
	{  100, 100, 95,  85,  75,  60,  50,  42,  35,  30,  25,  23 },

	/* 1  */
	{  100, 95,  85,  75,  60,  50,  42,  35,  30,  25,  23,  21 },

	/* 2  */
	{  95,  85,  75,  60,  50,  42,  35,  30,  26,  23,  21,  20 },

	/* 3  */
	{  85,  75,  60,  50,  42,  36,  32,  28,  25,  22,  20,  19 },

	/* 4  */
	{  75,  60,  50,  42,  36,  33,  28,  25,  23,  21,  19,  18 },

	/* 5  */
	{  60,  50,  42,  36,  33,  30,  27,  24,  22,  21,  19,  17 },

	/* 6  */
	{  50,  42,  36,  33,  30,  27,  25,  23,  21,  20,  18,  17 },

	/* 7  */
	{  42,  36,  33,  30,  28,  26,  24,  22,  20,  19,  18,  17 },

	/* 8  */
	{  36,  33,  30,  28,  26,  24,  22,  21,  20,  19,  17,  16 },

	/* 9  */
	{  35,  32,  29,  26,  24,  22,  21,  20,  19,  18,  17,  16 },

	/* 10 */
	{  34,  30,  27,  25,  23,  22,  21,  20,  19,  18,  17,  16 },

	/* 11+ */
	{  33,  29,  26,  24,  22,  21,  20,  19,  18,  17,  16,  15 },
   /* DEX: 3,   10,  17,  /20, /40, /60, /80, /100,/120,/150,/180,/200 */
};


/*
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(void)
{
	int i, j, k, levels;
	int num_allowed, num_known;
	int percent_spells;

	const magic_type *s_ptr;

	s16b old_spells;

	const char *p = ((player->class->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

	/* Hack -- must be literate */
	if (!player->class->spell_book) return;

	/* Hack -- wait for creation */
	if (!character_generated) return;

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Save the new_spells value */
	old_spells = player->upkeep->new_spells;

	/* Determine the number of spells allowed */
	levels = player->lev - player->class->spell_first + 1;

	/* Hack -- no negative spells */
	if (levels < 0) levels = 0;

	/* Number of 1/100 spells per level */
	percent_spells = adj_mag_study[player->state.stat_ind[player->class->spell_stat]];

	/* Extract total allowed spells (rounded up) */
	num_allowed = (((percent_spells * levels) + 50) / 100);

	/* Assume none known */
	num_known = 0;

	/* Count the number of spells we know */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Count known spells */
		if (player->spell_flags[j] & PY_SPELL_LEARNED)
		{
			num_known++;
		}
	}

	/* See how many spells we must forget or may learn */
	player->upkeep->new_spells = num_allowed - num_known;

	/* Forget spells which are too hard */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Get the spell */
		j = player->spell_order[i];

		/* Skip non-spells */
		if (j >= 99) continue;

		/* Get the spell */
		s_ptr = &player->class->spells.info[j];

		/* Skip spells we are allowed to know */
		if (s_ptr->slevel <= player->lev) continue;

		/* Is it known? */
		if (player->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			player->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			player->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg("You have forgotten the %s of %s.", p,
			           get_spell_name(player->class->spell_book, j));

			/* One more can be learned */
			player->upkeep->new_spells++;
		}
	}

	/* Forget spells if we know too many spells */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Stop when possible */
		if (player->upkeep->new_spells >= 0) break;

		/* Get the (i+1)th spell learned */
		j = player->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) continue;

		/* Forget it (if learned) */
		if (player->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			player->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			player->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg("You have forgotten the %s of %s.", p,
			           get_spell_name(player->class->spell_book, j));

			/* One more can be learned */
			player->upkeep->new_spells++;
		}
	}

	/* Check for spells to remember */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* None left to remember */
		if (player->upkeep->new_spells <= 0) break;

		/* Get the next spell we learned */
		j = player->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) break;

		/* Get the spell */
		s_ptr = &player->class->spells.info[j];

		/* Skip spells we cannot remember */
		if (s_ptr->slevel > player->lev) continue;

		/* First set of spells */
		if (player->spell_flags[j] & PY_SPELL_FORGOTTEN)
		{
			/* No longer forgotten */
			player->spell_flags[j] &= ~PY_SPELL_FORGOTTEN;

			/* Known once more */
			player->spell_flags[j] |= PY_SPELL_LEARNED;

			/* Message */
			msg("You have remembered the %s of %s.",
			           p, get_spell_name(player->class->spell_book, j));

			/* One less can be learned */
			player->upkeep->new_spells--;
		}
	}

	/* Assume no spells available */
	k = 0;

	/* Count spells that can be learned */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Get the spell */
		s_ptr = &player->class->spells.info[j];

		/* Skip spells we cannot remember or don't exist */
		if (s_ptr->slevel > player->lev || s_ptr->slevel == 0) continue;

		/* Skip spells we already know */
		if (player->spell_flags[j] & PY_SPELL_LEARNED)
		{
			continue;
		}

		/* Count it */
		k++;
	}

	/* Cannot learn more spells than exist */
	if (player->upkeep->new_spells > k) player->upkeep->new_spells = k;

	/* Spell count changed */
	if (old_spells != player->upkeep->new_spells)
	{
		/* Message if needed */
		if (player->upkeep->new_spells)
		{
			/* Message */
			msg("You can learn %d more %s%s.",
			           player->upkeep->new_spells, p,
			           (player->upkeep->new_spells != 1) ? "s" : "");
		}

		/* Redraw Study Status */
		player->upkeep->redraw |= (PR_STUDY | PR_OBJECT);
	}
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(void)
{
	int msp, levels, cur_wgt, max_wgt;

	object_type *o_ptr;

	bool old_cumber_glove = player->state.cumber_glove;
	bool old_cumber_armor = player->state.cumber_armor;

	/* Hack -- Must be literate */
	if (!player->class->spell_book)
	{
		player->msp = 0;
		player->csp = 0;
		player->csp_frac = 0;
		return;
	}

	/* Extract "effective" player level */
	levels = (player->lev - player->class->spell_first) + 1;
	if (levels > 0)
	{
		msp = 1;
		msp += adj_mag_mana[player->state.stat_ind[player->class->spell_stat]] * levels / 100;
	}
	else
	{
		levels = 0;
		msp = 0;
	}

	/* Process gloves for those disturbed by them */
	if (player_has(PF_CUMBER_GLOVE))
	{
		/* Assume player is not encumbered by gloves */
		player->state.cumber_glove = FALSE;

		/* Get the gloves */
		o_ptr = &player->inventory[INVEN_HANDS];

		/* Normal gloves hurt mage-type spells */
		if (o_ptr->kind &&
			!of_has(o_ptr->flags, OF_FREE_ACT) && 
			!kf_has(o_ptr->kind->kind_flags, KF_SPELLS_OK) &&
			(o_ptr->modifiers[OBJ_MOD_DEX] <= 0))
		{
			/* Encumbered */
			player->state.cumber_glove = TRUE;

			/* Reduce mana */
			msp = (3 * msp) / 4;
		}
	}

	/* Assume player not encumbered by armor */
	player->state.cumber_armor = FALSE;

	/* Weigh the armor */
	cur_wgt = 0;
	cur_wgt += player->inventory[INVEN_BODY].weight;
	cur_wgt += player->inventory[INVEN_HEAD].weight;
	cur_wgt += player->inventory[INVEN_ARM].weight;
	cur_wgt += player->inventory[INVEN_OUTER].weight;
	cur_wgt += player->inventory[INVEN_HANDS].weight;
	cur_wgt += player->inventory[INVEN_FEET].weight;

	/* Determine the weight allowance */
	max_wgt = player->class->spell_weight;

	/* Heavy armor penalizes mana */
	if (((cur_wgt - max_wgt) / 10) > 0)
	{
		/* Encumbered */
		player->state.cumber_armor = TRUE;

		/* Reduce mana */
		msp -= ((cur_wgt - max_wgt) / 10);
	}

	/* Mana can never be negative */
	if (msp < 0) msp = 0;

	/* Maximum mana has changed */
	if (player->msp != msp)
	{
		/* Save new limit */
		player->msp = msp;

		/* Enforce new limit */
		if (player->csp >= msp)
		{
			player->csp = msp;
			player->csp_frac = 0;
		}

		/* Display mana later */
		player->upkeep->redraw |= (PR_MANA);
	}

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "glove state" changes */
	if (old_cumber_glove != player->state.cumber_glove)
	{
		/* Message */
		if (player->state.cumber_glove)
		{
			msg("Your covered hands feel unsuitable for spellcasting.");
		}
		else
		{
			msg("Your hands feel more suitable for spellcasting.");
		}
	}

	/* Take note when "armor state" changes */
	if (old_cumber_armor != player->state.cumber_armor)
	{
		/* Message */
		if (player->state.cumber_armor)
		{
			msg("The weight of your armor encumbers your movement.");
		}
		else
		{
			msg("You feel able to move more freely.");
		}
	}
}


/*
 * Calculate the players (maximal) hit points
 *
 * Adjust current hitpoints if necessary
 */
static void calc_hitpoints(void)
{
	long bonus;
	int mhp;

	/* Get "1/100th hitpoint bonus per level" value */
	bonus = adj_con_mhp[player->state.stat_ind[A_CON]];

	/* Calculate hitpoints */
	mhp = player->player_hp[player->lev-1] + (bonus * player->lev / 100);

	/* Always have at least one hitpoint per level */
	if (mhp < player->lev + 1) mhp = player->lev + 1;

	/* New maximum hitpoints */
	if (player->mhp != mhp)
	{
		/* Save new limit */
		player->mhp = mhp;

		/* Enforce new limit */
		if (player->chp >= mhp)
		{
			player->chp = mhp;
			player->chp_frac = 0;
		}

		/* Display hitpoints (later) */
		player->upkeep->redraw |= (PR_HP);
	}
}


/*
 * Calculate and set the current light radius.
 *
 * The brightest wielded object counts as the light source; radii do not add
 * up anymore.
 *
 * Note that a cursed light source no longer emits light.
 */
static void calc_torch(void)
{
	int i;

	s16b old_light = player->state.cur_light;
	s16b new_light = 0;

	/* Ascertain lightness if in the town */
	if (!player->depth && ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2))) {
		new_light = 0;
		if (old_light != new_light) {
			/* Update the visuals */
			player->state.cur_light = new_light;
			player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		}
		return;
	}

	/* Examine all wielded objects, use the brightest */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)	{
		int amt = 0;
		object_type *o_ptr = &player->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->kind) continue;

		/* Light radius is now a modifier */
		amt = o_ptr->modifiers[OBJ_MOD_LIGHT];

		/* Cursed objects emit no light */
		if (of_has(o_ptr->flags, OF_LIGHT_CURSE))
			amt = 0;

		/* Examine actual lights */
		if (tval_is_light(o_ptr) && !of_has(o_ptr->flags, OF_NO_FUEL) &&
				o_ptr->timeout == 0)
			/* Lights without fuel provide no light */
			amt = 0;

		/* Alter player->state.cur_light if reasonable */
	    new_light += amt;
	}

	/* Limit light */
	new_light = MIN(new_light, 5);
	new_light = MAX(new_light, 0);

	/* Notice changes in the "light radius" */
	if (old_light != new_light) {
		/* Update the visuals */
		player->state.cur_light = new_light;
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/**
 * Populates `chances` with the player's chance of digging through
 * the diggable terrain types in one turn out of 1600.
 */
void calc_digging_chances(player_state *state, int chances[DIGGING_MAX])
{
	int i;

	chances[DIGGING_RUBBLE] = state->skills[SKILL_DIGGING] * 8;
	chances[DIGGING_MAGMA] = (state->skills[SKILL_DIGGING] - 10) * 4;
	chances[DIGGING_QUARTZ] = (state->skills[SKILL_DIGGING] - 20) * 2;
	chances[DIGGING_GRANITE] = (state->skills[SKILL_DIGGING] - 40) * 1;
	/* Approximate a 1/1200 chance per skill point over 30 */
	chances[DIGGING_DOORS] = (state->skills[SKILL_DIGGING] * 4 - 119) / 3;

	/* Don't let any negative chances through */
	for (i = 0; i < DIGGING_MAX; i++)
		chances[i] = MAX(0, chances[i]);
}

/*
 * Calculate the blows a player would get.
 *
 * \param o_ptr is the object for which we are calculating blows
 * \param state is the player state for which we are calculating blows
 * \param extra_blows is the number of +blows available from this object and
 * this state
 *
 * N.B. state->num_blows is now 100x the number of blows.
 */
int calc_blows(const object_type *o_ptr, player_state *state, int extra_blows)
{
	int blows;
	int str_index, dex_index;
	int div;
	int blow_energy;

	/* Enforce a minimum "weight" (tenth pounds) */
	div = ((o_ptr->weight < player->class->min_weight) ? player->class->min_weight :
		o_ptr->weight);

	/* Get the strength vs weight */
	str_index = adj_str_blow[state->stat_ind[A_STR]] *
			player->class->att_multiply / div;

	/* Maximal value */
	if (str_index > 11) str_index = 11;

	/* Index by dexterity */
	dex_index = MIN(adj_dex_blow[state->stat_ind[A_DEX]], 11);

	/* Use the blows table to get energy per blow */
	blow_energy = blows_table[str_index][dex_index];

	blows = MIN((10000 / blow_energy), (100 * player->class->max_attacks));

	/* Require at least one blow */
	return MAX(blows + (100 * extra_blows), 100);
}


/*
 * Computes current weight limit.
 */
static int weight_limit(player_state *state)
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[state->stat_ind[A_STR]] * 100;

	/* Return the result */
	return (i);
}


/*
 * Computes weight remaining before burdened.
 */
int weight_remaining(void)
{
	int i;

	/* Weight limit based only on strength */
	i = 60 * adj_str_wgt[player->state.stat_ind[A_STR]]
		- player->upkeep->total_weight - 1;

	/* Return the result */
	return (i);
}


/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.
 *
 * See also calc_mana() and calc_hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not very painful.
 *
 * The "weapon" and "bow" do *not* add to the bonuses to hit or to
 * damage, since that would affect non-combat things.  These values
 * are actually added in later, at the appropriate place.
 *
 * If id_only is true, calc_bonuses() will only use the known
 * information of objects; thus it returns what the player _knows_
 * the character state to be.
 */
void calc_bonuses(object_type inventory[], player_state *state, bool id_only)
{
	int i, j, hold;

	int extra_blows = 0;
	int extra_shots = 0;
	int extra_might = 0;

	object_type *o_ptr;

	bitflag f[OF_SIZE];
	bitflag collect_f[OF_SIZE];

	/*** Reset ***/

	memset(state, 0, sizeof *state);

	/* Set various defaults */
	state->speed = 110;
	state->num_blows = 100;


	/*** Extract race/class info ***/

	/* Base infravision (purely racial) */
	state->see_infra = player->race->infra;

	/* Base skills */
	for (i = 0; i < SKILL_MAX; i++)
		state->skills[i] = player->race->r_skills[i] + player->class->c_skills[i];


	/*** Analyze player ***/

	/* Extract the player flags */
	player_flags(collect_f);


	/*** Analyze equipment ***/

	/* Scan the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Extract the item flags */
		if (id_only)
			object_flags_known(o_ptr, f);
		else
			object_flags(o_ptr, f);

		of_union(collect_f, f);

		/* Affect stats */
		state->stat_add[A_STR] += o_ptr->modifiers[OBJ_MOD_STR];
		state->stat_add[A_INT] += o_ptr->modifiers[OBJ_MOD_INT];
		state->stat_add[A_WIS] += o_ptr->modifiers[OBJ_MOD_WIS];
		state->stat_add[A_DEX] += o_ptr->modifiers[OBJ_MOD_DEX];
		state->stat_add[A_CON] += o_ptr->modifiers[OBJ_MOD_CON];

		/* Affect stealth */
		state->skills[SKILL_STEALTH] += o_ptr->modifiers[OBJ_MOD_STEALTH];

		/* Affect searching ability (factor of five) */
		state->skills[SKILL_SEARCH] += (o_ptr->modifiers[OBJ_MOD_SEARCH] * 5);

		/* Affect searching frequency (factor of five) */
		state->skills[SKILL_SEARCH_FREQUENCY] += 
			(o_ptr->modifiers[OBJ_MOD_SEARCH] * 5);

		/* Affect infravision */
		state->see_infra += o_ptr->modifiers[OBJ_MOD_INFRA];

		/* Affect digging (factor of 20) */
		state->skills[SKILL_DIGGING] += (o_ptr->modifiers[OBJ_MOD_TUNNEL] * 20);

		/* Affect speed */
		state->speed += o_ptr->modifiers[OBJ_MOD_SPEED];

		/* Affect blows */
		extra_blows += o_ptr->modifiers[OBJ_MOD_BLOWS];

		/* Affect shots */
		extra_shots += o_ptr->modifiers[OBJ_MOD_SHOTS];

		/* Affect Might */
		extra_might += o_ptr->modifiers[OBJ_MOD_MIGHT];

		/* Modify the base armor class */
		state->ac += o_ptr->ac;

		/* The base armor class is always known */
		state->dis_ac += o_ptr->ac;

		/* Apply the bonuses to armor class */
		if (!id_only || object_is_known(o_ptr))
			state->to_a += o_ptr->to_a;

		/* Apply the mental bonuses to armor class, if known */
		if (object_defence_plusses_are_visible(o_ptr))
			state->dis_to_a += o_ptr->to_a;

		/* Hack -- do not apply "weapon" bonuses */
		if (i == INVEN_WIELD) continue;

		/* Hack -- do not apply "bow" bonuses */
		if (i == INVEN_BOW) continue;

		/* Apply the bonuses to hit/damage */
		if (!id_only || object_is_known(o_ptr))
		{
			state->to_h += o_ptr->to_h;
			state->to_d += o_ptr->to_d;
		}

		/* Apply the mental bonuses tp hit/damage, if known */
		if (object_attack_plusses_are_visible(o_ptr))
		{
			state->dis_to_h += o_ptr->to_h;
			state->dis_to_d += o_ptr->to_d;
		}
	}


	/*** Update all flags ***/

	of_union(state->flags, collect_f);

	/*** Handle stats ***/

	/* Calculate stats */
	for (i = 0; i < A_MAX; i++)
	{
		int add, top, use, ind;

		/* Extract modifier */
		add = state->stat_add[i];

		/* Modify the stats for race/class */
		add += (player->race->r_adj[i] + player->class->c_adj[i]);

		/* Extract the new "stat_top" value for the stat */
		top = modify_stat_value(player->stat_max[i], add);

		/* Save the new value */
		state->stat_top[i] = top;

		/* Extract the new "stat_use" value for the stat */
		use = modify_stat_value(player->stat_cur[i], add);

		/* Save the new value */
		state->stat_use[i] = use;

		/* Values: n/a */
		if (use <= 3) ind = 0;

		/* Values: 3, 4, ..., 18 */
		else if (use <= 18) ind = (use - 3);

		/* Ranges: 18/00-18/09, ..., 18/210-18/219 */
		else if (use <= 18+219) ind = (15 + (use - 18) / 10);

		/* Range: 18/220+ */
		else ind = (37);

		assert((0 <= ind) && (ind < STAT_RANGE));

		/* Save the new index */
		state->stat_ind[i] = ind;
	}


	/*** Temporary flags ***/

	/* Apply temporary "stun" */
	if (player->timed[TMD_STUN] > 50)
	{
		state->to_h -= 20;
		state->dis_to_h -= 20;
		state->to_d -= 20;
		state->dis_to_d -= 20;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 8 / 10;
	}
	else if (player->timed[TMD_STUN])
	{
		state->to_h -= 5;
		state->dis_to_h -= 5;
		state->to_d -= 5;
		state->dis_to_d -= 5;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 9 / 10;
	}

	/* Invulnerability */
	if (player->timed[TMD_INVULN])
	{
		state->to_a += 100;
		state->dis_to_a += 100;
	}

	/* Temporary blessing */
	if (player->timed[TMD_BLESSED])
	{
		state->to_a += 5;
		state->dis_to_a += 5;
		state->to_h += 10;
		state->dis_to_h += 10;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 105 / 100;
	}

	/* Temporary shield */
	if (player->timed[TMD_SHIELD])
	{
		state->to_a += 50;
		state->dis_to_a += 50;
	}

	/* Temporary stoneskin */
	if (player->timed[TMD_STONESKIN])
	{
		state->to_a += 40;
		state->dis_to_a += 40;
		state->speed -= 5;
	}

	/* Temporary resistance to fear */
	if (player->timed[TMD_BOLD])
		of_on(state->flags, OF_PROT_FEAR);

	/* Temporary "Hero" */
	if (player->timed[TMD_HERO])
	{
		state->to_h += 12;
		state->dis_to_h += 12;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 105 / 100;
	}

	/* Temporary "Berserk" */
	if (player->timed[TMD_SHERO])
	{
		state->to_h += 24;
		state->dis_to_h += 24;
		state->to_a -= 10;
		state->dis_to_a -= 10;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 9 / 10;
	}

	/* Temporary "fast" */
	if (player->timed[TMD_FAST] || player->timed[TMD_SPRINT])
		state->speed += 10;

	/* Temporary "slow" */
	if (player->timed[TMD_SLOW])
		state->speed -= 10;

	/* Temporary infravision boost */
	if (player->timed[TMD_SINFRA])
		state->see_infra += 5;

	/* Temporary ESP */
	if (player->timed[TMD_TELEPATHY])
		of_on(state->flags, OF_TELEPATHY);

	/* Temporary see invisible */
	if (player->timed[TMD_SINVIS])
		of_on(state->flags, OF_SEE_INVIS);

	/* Fear / terror flags */
	if (player->timed[TMD_AFRAID] || player->timed[TMD_TERROR])
		of_on(state->flags, OF_AFRAID);
	if (player->timed[TMD_TERROR])
		state->speed += 10;

	/* Resist confusion */
	if (player->timed[TMD_OPP_CONF])
		of_on(state->flags, OF_PROT_CONF);

	/* Confusion */
	if (player->timed[TMD_CONFUSED])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 75 / 100;

	/* Amnesia */
	if (player->timed[TMD_AMNESIA])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 8 / 10;

	/* Poison */
	if (player->timed[TMD_POISONED])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 95 / 100;

	/* Hallucination */
	if (player->timed[TMD_IMAGE])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 8 / 10;

	/*** Analyze flags ***/

	/* Check for fear */
	if (of_has(state->flags, OF_AFRAID)) {
		state->to_h -= 20;
		state->dis_to_h -= 20;
		state->to_a += 8;
		state->dis_to_a += 8;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 95 / 100;
	}

	/*** Analyze weight ***/

	/* Extract the current weight (in tenth pounds) */
	j = player->upkeep->total_weight;

	/* Extract the "weight limit" (in tenth pounds) */
	i = weight_limit(state);

	/* Apply "encumbrance" from weight */
	if (j > i / 2) state->speed -= ((j - (i / 2)) / (i / 10));

	/* Searching slows the player down */
	if (player->searching) state->speed -= 10;

	/* Sanity check on extreme speeds */
	if (state->speed < 0) state->speed = 0;
	if (state->speed > 199) state->speed = 199;

	/*** Apply modifier bonuses ***/

	/* Actual Modifier Bonuses (Un-inflate stat bonuses) */
	state->to_a += ((int)(adj_dex_ta[state->stat_ind[A_DEX]]) - 128);
	state->to_d += ((int)(adj_str_td[state->stat_ind[A_STR]]) - 128);
	state->to_h += ((int)(adj_dex_th[state->stat_ind[A_DEX]]) - 128);
	state->to_h += ((int)(adj_str_th[state->stat_ind[A_STR]]) - 128);

	/* Displayed Modifier Bonuses (Un-inflate stat bonuses) */
	state->dis_to_a += ((int)(adj_dex_ta[state->stat_ind[A_DEX]]) - 128);
	state->dis_to_d += ((int)(adj_str_td[state->stat_ind[A_STR]]) - 128);
	state->dis_to_h += ((int)(adj_dex_th[state->stat_ind[A_DEX]]) - 128);
	state->dis_to_h += ((int)(adj_str_th[state->stat_ind[A_STR]]) - 128);


	/*** Modify skills ***/

	/* Affect Skill -- disarming (DEX and INT) */
	state->skills[SKILL_DISARM] += adj_dex_dis[state->stat_ind[A_DEX]];
	state->skills[SKILL_DISARM] += adj_int_dis[state->stat_ind[A_INT]];

	/* Affect Skill -- magic devices (INT) */
	state->skills[SKILL_DEVICE] += adj_int_dev[state->stat_ind[A_INT]];

	/* Affect Skill -- saving throw (WIS) */
	state->skills[SKILL_SAVE] += adj_wis_sav[state->stat_ind[A_WIS]];

	/* Affect Skill -- digging (STR) */
	state->skills[SKILL_DIGGING] += adj_str_dig[state->stat_ind[A_STR]];

	/* Affect Skills (Level, by Class) */
	for (i = 0; i < SKILL_MAX; i++)
		state->skills[i] += (player->class->x_skills[i] * player->lev / 10);

	/* Limit Skill -- digging from 1 up */
	if (state->skills[SKILL_DIGGING] < 1) state->skills[SKILL_DIGGING] = 1;

	/* Limit Skill -- stealth from 0 to 30 */
	if (state->skills[SKILL_STEALTH] > 30) state->skills[SKILL_STEALTH] = 30;
	if (state->skills[SKILL_STEALTH] < 0) state->skills[SKILL_STEALTH] = 0;

	/* Apply Skill -- Extract noise from stealth */
	state->noise = (1L << (30 - state->skills[SKILL_STEALTH]));

	/* Obtain the "hold" value */
	hold = adj_str_hold[state->stat_ind[A_STR]];


	/*** Analyze current bow ***/

	/* Examine the "current bow" */
	o_ptr = &inventory[INVEN_BOW];

	/* Assume not heavy */
	state->heavy_shoot = FALSE;

	/* It is hard to hold a heavy bow */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy bow */
		state->to_h += 2 * (hold - o_ptr->weight / 10);
		state->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy Bow */
		state->heavy_shoot = TRUE;
	}

	/* Analyze launcher */
	if (o_ptr->kind)
	{
		/* Get to shoot */
		state->num_shots = 1;

		/* Analyze the launcher */
		switch (o_ptr->sval)
		{
			/* Sling and ammo */
			case SV_SLING:
			{
				state->ammo_tval = TV_SHOT;
				state->ammo_mult = 2;
				break;
			}

			/* Short Bow and Arrow */
			case SV_SHORT_BOW:
			{
				state->ammo_tval = TV_ARROW;
				state->ammo_mult = 2;
				break;
			}

			/* Long Bow and Arrow */
			case SV_LONG_BOW:
			{
				state->ammo_tval = TV_ARROW;
				state->ammo_mult = 3;
				break;
			}

			/* Light Crossbow and Bolt */
			case SV_LIGHT_XBOW:
			{
				state->ammo_tval = TV_BOLT;
				state->ammo_mult = 3;
				break;
			}

			/* Heavy Crossbow and Bolt */
			case SV_HEAVY_XBOW:
			{
				state->ammo_tval = TV_BOLT;
				state->ammo_mult = 4;
				break;
			}
		}

		/* Apply special flags */
		if (o_ptr->kind && !state->heavy_shoot)
		{
			/* Extra shots */
			state->num_shots += extra_shots;

			/* Extra might */
			state->ammo_mult += extra_might;

			/* Hack -- Rangers love Bows */
			if (player_has(PF_EXTRA_SHOT) &&
			    (state->ammo_tval == TV_ARROW))
			{
				/* Extra shot at level 20 */
				if (player->lev >= 20) state->num_shots++;

				/* Extra shot at level 40 */
				if (player->lev >= 40) state->num_shots++;
			}
		}

		/* Require at least one shot */
		if (state->num_shots < 1) state->num_shots = 1;
	}


	/*** Analyze weapon ***/

	/* Examine the "current weapon" */
	o_ptr = &inventory[INVEN_WIELD];

	/* Assume not heavy */
	state->heavy_wield = FALSE;

	/* It is hard to hold a heavy weapon */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy weapon */
		state->to_h += 2 * (hold - o_ptr->weight / 10);
		state->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy weapon */
		state->heavy_wield = TRUE;
	}

	/* Non-object means barehanded attacks */
	if (!o_ptr->kind)
		assert(o_ptr->weight == 0);

	/* Normal weapons */
	if (!state->heavy_wield)
	{
		/* Calculate number of blows */
		state->num_blows = calc_blows(o_ptr, state, extra_blows);

		/* Boost digging skill by weapon weight */
		state->skills[SKILL_DIGGING] += (o_ptr->weight / 10);
	}

	/* Assume okay */
	state->icky_wield = FALSE;

	/* Priest weapon penalty for non-blessed edged weapons */
	if (player_has(PF_BLESS_WEAPON) && !player_of_has(player, OF_BLESSED) &&
		tval_is_pointy(o_ptr))
	{
		/* Reduce the real bonuses */
		state->to_h -= 2;
		state->to_d -= 2;

		/* Reduce the mental bonuses */
		state->dis_to_h -= 2;
		state->dis_to_d -= 2;

		/* Icky weapon */
		state->icky_wield = TRUE;
	}

	return;
}

/*
 * Calculate bonuses, and print various things on changes.
 */
static void update_bonuses(void)
{
	int i;

	player_state *state = &player->state;
	player_state old = player->state;


	/*** Calculate bonuses ***/

	calc_bonuses(player->inventory, &player->state, FALSE);


	/*** Notice changes ***/

	/* Analyze stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Notice changes */
		if (state->stat_top[i] != old.stat_top[i])
		{
			/* Redisplay the stats later */
			player->upkeep->redraw |= (PR_STATS);
		}

		/* Notice changes */
		if (state->stat_use[i] != old.stat_use[i])
		{
			/* Redisplay the stats later */
			player->upkeep->redraw |= (PR_STATS);
		}

		/* Notice changes */
		if (state->stat_ind[i] != old.stat_ind[i])
		{
			/* Change in CON affects Hitpoints */
			if (i == A_CON)
			{
				player->upkeep->update |= (PU_HP);
			}

			/* Change in INT may affect Mana/Spells */
			else if (i == A_INT)
			{
				if (player->class->spell_stat == A_INT)
				{
					player->upkeep->update |= (PU_MANA | PU_SPELLS);
				}
			}

			/* Change in WIS may affect Mana/Spells */
			else if (i == A_WIS)
			{
				if (player->class->spell_stat == A_WIS)
				{
					player->upkeep->update |= (PU_MANA | PU_SPELLS);
				}
			}
		}
	}


	/* Hack -- Telepathy Change */
	if (of_has(state->flags, OF_TELEPATHY) != of_has(old.flags, OF_TELEPATHY))
	{
		/* Update monster visibility */
		player->upkeep->update |= (PU_MONSTERS);
	}

	/* Hack -- See Invis Change */
	if (of_has(state->flags, OF_SEE_INVIS) != of_has(old.flags, OF_SEE_INVIS))
	{
		/* Update monster visibility */
		player->upkeep->update |= (PU_MONSTERS);
	}

	/* Redraw speed (if needed) */
	if (state->speed != old.speed)
	{
		/* Redraw speed */
		player->upkeep->redraw |= (PR_SPEED);
	}

	/* Redraw armor (if needed) */
	if ((state->dis_ac != old.dis_ac) || (state->dis_to_a != old.dis_to_a))
	{
		/* Redraw */
		player->upkeep->redraw |= (PR_ARMOR);
	}

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "heavy bow" changes */
	if (old.heavy_shoot != state->heavy_shoot)
	{
		/* Message */
		if (state->heavy_shoot)
			msg("You have trouble wielding such a heavy bow.");
		else if (player->inventory[INVEN_BOW].kind)
			msg("You have no trouble wielding your bow.");
		else
			msg("You feel relieved to put down your heavy bow.");
	}

	/* Take note when "heavy weapon" changes */
	if (old.heavy_wield != state->heavy_wield)
	{
		/* Message */
		if (state->heavy_wield)
			msg("You have trouble wielding such a heavy weapon.");
		else if (player->inventory[INVEN_WIELD].kind)
			msg("You have no trouble wielding your weapon.");
		else
			msg("You feel relieved to put down your heavy weapon.");	
	}

	/* Take note when "illegal weapon" changes */
	if (old.icky_wield != state->icky_wield)
	{
		/* Message */
		if (state->icky_wield)
			msg("You do not feel comfortable with your weapon.");
		else if (player->inventory[INVEN_WIELD].kind)
			msg("You feel comfortable with your weapon.");
		else
			msg("You feel more comfortable after removing your weapon.");
	}
}




/*** Monster and object tracking functions ***/

/*
 * Track the given monster
 */
void health_track(struct player_upkeep *upkeep, struct monster *m_ptr)
{
	upkeep->health_who = m_ptr;
	upkeep->redraw |= PR_HEALTH;
}

/*
 * Track the given monster race
 */
void monster_race_track(struct player_upkeep *upkeep, monster_race *race)
{
	/* Save this monster ID */
	upkeep->monster_race = race;

	/* Window stuff */
	upkeep->redraw |= (PR_MONSTER);
}

/*
 * Track the given object
 */
void track_object(struct player_upkeep *upkeep, int item)
{
	upkeep->object_idx = item;
	upkeep->object_kind = NULL;
	upkeep->redraw |= (PR_OBJECT);
}

/*
 * Track the given object kind
 */
void track_object_kind(struct player_upkeep *upkeep, struct object_kind *kind)
{
	upkeep->object_idx = NO_OBJECT;
	upkeep->object_kind = kind;
	upkeep->redraw |= (PR_OBJECT);
}

/*
 * Is the given item tracked?
 */
bool tracked_object_is(struct player_upkeep *upkeep, int item)
{
	return (upkeep->object_idx == item);
}



/*** Generic "deal with" functions ***/

/*
 * Handle "player->upkeep->notice"
 */
void notice_stuff(struct player_upkeep *upkeep)
{
	/* Notice stuff */
	if (!upkeep->notice) return;


	/* Deal with autoinscribe stuff */
	if (upkeep->notice & PN_AUTOINSCRIBE)
	{
		upkeep->notice &= ~(PN_AUTOINSCRIBE);
		autoinscribe_pack();
		autoinscribe_ground();
	}

	/* Deal with squelch stuff */
	if (upkeep->notice & PN_SQUELCH)
	{
		upkeep->notice &= ~(PN_SQUELCH);
		squelch_drop();
	}

	/* Combine the pack */
	if (upkeep->notice & PN_COMBINE)
	{
		upkeep->notice &= ~(PN_COMBINE);
		combine_pack();
	}

	/* Reorder the pack */
	if (upkeep->notice & PN_REORDER)
	{
		upkeep->notice &= ~(PN_REORDER);
		reorder_pack();
	}

	/* Sort the quiver */
	if (upkeep->notice & PN_SORT_QUIVER)
	{
		upkeep->notice &= ~(PN_SORT_QUIVER);
		sort_quiver();
	}

	/* Dump the monster messages */
	if (upkeep->notice & PN_MON_MESSAGE)
	{
		upkeep->notice &= ~(PN_MON_MESSAGE);

		/* Make sure this comes after all of the monster messages */
		flush_all_monster_messages();
	}
}

/*
 * Handle "player->upkeep->update"
 */
void update_stuff(struct player_upkeep *upkeep)
{
	/* Update stuff */
	if (!upkeep->update) return;


	if (upkeep->update & (PU_BONUS))
	{
		upkeep->update &= ~(PU_BONUS);
		update_bonuses();
	}

	if (upkeep->update & (PU_TORCH))
	{
		upkeep->update &= ~(PU_TORCH);
		calc_torch();
	}

	if (upkeep->update & (PU_HP))
	{
		upkeep->update &= ~(PU_HP);
		calc_hitpoints();
	}

	if (upkeep->update & (PU_MANA))
	{
		upkeep->update &= ~(PU_MANA);
		calc_mana();
	}

	if (upkeep->update & (PU_SPELLS))
	{
		upkeep->update &= ~(PU_SPELLS);
		calc_spells();
	}


	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;


	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;


	if (upkeep->update & (PU_FORGET_VIEW))
	{
		upkeep->update &= ~(PU_FORGET_VIEW);
		forget_view(cave);
	}

	if (upkeep->update & (PU_UPDATE_VIEW))
	{
		upkeep->update &= ~(PU_UPDATE_VIEW);
		update_view(cave, player);
	}


	if (upkeep->update & (PU_FORGET_FLOW))
	{
		upkeep->update &= ~(PU_FORGET_FLOW);
		cave_forget_flow(cave);
	}

	if (upkeep->update & (PU_UPDATE_FLOW))
	{
		upkeep->update &= ~(PU_UPDATE_FLOW);
		cave_update_flow(cave);
	}


	if (upkeep->update & (PU_DISTANCE))
	{
		upkeep->update &= ~(PU_DISTANCE);
		upkeep->update &= ~(PU_MONSTERS);
		update_monsters(TRUE);
	}

	if (upkeep->update & (PU_MONSTERS))
	{
		upkeep->update &= ~(PU_MONSTERS);
		update_monsters(FALSE);
	}


	if (upkeep->update & (PU_PANEL))
	{
		upkeep->update &= ~(PU_PANEL);
		event_signal(EVENT_PLAYERMOVED);
	}
}



struct flag_event_trigger
{
	u32b flag;
	game_event_type event;
};



/*
 * Events triggered by the various flags.
 */
static const struct flag_event_trigger redraw_events[] =
{
	{ PR_MISC,    EVENT_RACE_CLASS },
	{ PR_TITLE,   EVENT_PLAYERTITLE },
	{ PR_LEV,     EVENT_PLAYERLEVEL },
	{ PR_EXP,     EVENT_EXPERIENCE },
	{ PR_STATS,   EVENT_STATS },
	{ PR_ARMOR,   EVENT_AC },
	{ PR_HP,      EVENT_HP },
	{ PR_MANA,    EVENT_MANA },
	{ PR_GOLD,    EVENT_GOLD },
	{ PR_HEALTH,  EVENT_MONSTERHEALTH },
	{ PR_DEPTH,   EVENT_DUNGEONLEVEL },
	{ PR_SPEED,   EVENT_PLAYERSPEED },
	{ PR_STATE,   EVENT_STATE },
	{ PR_STATUS,  EVENT_STATUS },
	{ PR_STUDY,   EVENT_STUDYSTATUS },
	{ PR_DTRAP,   EVENT_DETECTIONSTATUS },

	{ PR_INVEN,   EVENT_INVENTORY },
	{ PR_EQUIP,   EVENT_EQUIPMENT },
	{ PR_MONLIST, EVENT_MONSTERLIST },
	{ PR_ITEMLIST, EVENT_ITEMLIST },
	{ PR_MONSTER, EVENT_MONSTERTARGET },
	{ PR_OBJECT, EVENT_OBJECTTARGET },
	{ PR_MESSAGE, EVENT_MESSAGE },
};

/*
 * Handle "player->upkeep->redraw"
 */
void redraw_stuff(struct player_upkeep *upkeep)
{
	size_t i;

	/* Redraw stuff */
	if (!upkeep->redraw) return;

	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;

	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;

	/* For each listed flag, send the appropriate signal to the UI */
	for (i = 0; i < N_ELEMENTS(redraw_events); i++)
	{
		const struct flag_event_trigger *hnd = &redraw_events[i];

		if (upkeep->redraw & hnd->flag)
			event_signal(hnd->event);
	}

	/* Then the ones that require parameters to be supplied. */
	if (upkeep->redraw & PR_MAP)
	{
		/* Mark the whole map to be redrawn */
		event_signal_point(EVENT_MAP, -1, -1);
	}

	upkeep->redraw = 0;

	/*
	 * Do any plotting, etc. delayed from earlier - this set of updates
	 * is over.
	 */
	event_signal(EVENT_END);
}


/*
 * Handle "player->upkeep->update" and "player->upkeep->redraw"
 */
void handle_stuff(struct player_upkeep *upkeep)
{
	if (upkeep->update) update_stuff(upkeep);
	if (upkeep->redraw) redraw_stuff(upkeep);
}

