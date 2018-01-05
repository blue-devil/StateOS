/******************************************************************************

    @file    StateOS: os_mut.c
    @author  Rajmund Szymanski
    @date    01.01.2018
    @brief   This file provides set of functions for StateOS.

 ******************************************************************************

    StateOS - Copyright (C) 2013 Rajmund Szymanski.

    This file is part of StateOS distribution.

    StateOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.

    StateOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

 ******************************************************************************/

#include "inc/os_mut.h"

/* -------------------------------------------------------------------------- */
void mut_init( mut_t *mut )
/* -------------------------------------------------------------------------- */
{
	assert(!port_isr_inside());
	assert(mut);

	port_sys_lock();

	memset(mut, 0, sizeof(mut_t));

	port_sys_unlock();
}

/* -------------------------------------------------------------------------- */
mut_t *mut_create( void )
/* -------------------------------------------------------------------------- */
{
	mut_t *mut;

	assert(!port_isr_inside());

	port_sys_lock();

	mut = core_sys_alloc(sizeof(mut_t));
	mut_init(mut);
	mut->res = mut;

	port_sys_unlock();

	return mut;
}

/* -------------------------------------------------------------------------- */
void mut_kill( mut_t *mut )
/* -------------------------------------------------------------------------- */
{
	assert(!port_isr_inside());
	assert(mut);

	port_sys_lock();

	core_all_wakeup(mut, E_STOPPED);

	port_sys_unlock();
}

/* -------------------------------------------------------------------------- */
void mut_delete( mut_t *mut )
/* -------------------------------------------------------------------------- */
{
	port_sys_lock();

	mut_kill(mut);
	core_sys_free(mut->res);

	port_sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_mut_wait( mut_t *mut, cnt_t time, unsigned(*wait)(void*,cnt_t) )
/* -------------------------------------------------------------------------- */
{
	unsigned event = E_TIMEOUT;

	assert(mut);

	port_sys_lock();

	if (mut->owner == 0)
	{
		mut->owner = System.cur;
		event = E_SUCCESS;
	}
	else
	if (mut->owner != System.cur)
	{
		event = wait(mut, time);
	}
	
	port_sys_unlock();

	return event;
}

/* -------------------------------------------------------------------------- */
unsigned mut_waitUntil( mut_t *mut, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	assert(!port_isr_inside());

	return priv_mut_wait(mut, time, core_tsk_waitUntil);
}

/* -------------------------------------------------------------------------- */
unsigned mut_waitFor( mut_t *mut, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	assert(!port_isr_inside());

	return priv_mut_wait(mut, delay, core_tsk_waitFor);
}

/* -------------------------------------------------------------------------- */
unsigned mut_give( mut_t *mut )
/* -------------------------------------------------------------------------- */
{
	unsigned event = E_TIMEOUT;
	
	assert(!port_isr_inside());
	assert(mut);

	port_sys_lock();

	if (mut->owner == System.cur)
	{
		mut->owner = core_one_wakeup(mut, E_SUCCESS);
		event = E_SUCCESS;
	}

	port_sys_unlock();

	return event;
}

/* -------------------------------------------------------------------------- */
