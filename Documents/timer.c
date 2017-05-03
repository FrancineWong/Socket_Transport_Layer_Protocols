#include <stdio.h>

#define TRUE	1
#define FALSE	0

#define MAX_TIMERS	...	/* number of timers */
typedef ... TIME;		/* how time is actually stored */
#define VERY_LONG_TIME	...	/* longest time possible */

struct timer {
	int inuse;		/* TRUE if in use */
	TIME time;		/* relative time to wait */
	char *event;		/* set to TRUE at timeout */
} timers[MAX_TIMERS];		/* set of timers */

void timers_init() {
	struct timer *t;

	for (t=timers;t<&timers[MAX_TIMERS];t++)
		t->inuse = FALSE;
}
/********************************************************************************/
volatile TIME time_now;

struct timer *timer_next = NULL;/* timer we expect to run down next */
TIME time_timer_set;		/* time when physical timer was set */

void timers_update();		/* see discussion below */

/************************************************************************************/
//similar to stoptimer
void timer_undeclare(t)
struct timer *t;
{
	disable_interrupts();
	if (!t->inuse) {
		enable_interrupts();
		return;
	}

	t->inuse = FALSE;

	/* check if we were waiting on this one */
	if (t == timer_next) {
		timers_update(time_now - time_timer_set);
		if (timer_next) {
			start_physical_timer(timer_next->time);
			time_timer_set = time_now;
		}
	}
	enable_interrupts();
}

/************************************************************************************/
void timers_update(time)
TIME time;
{
	static struct timer timer_last = {
		FALSE			/* in use */,
		VERY_LONG_TIME		/* time */,
		NULL			/* event pointer */
	};

	struct timer *t;

	timer_next = &timer_last;

	for (t=timers;t<&timers[MAX_TIMERS];t++) {
		if (t->inuse) {
			if (time < t->time) { /* unexpired */
				t->time -= time;
				if (t->time < timer_next->time)
					timer_next = t;
			} else { /* expired */
				/* tell scheduler */
				*t->event = TRUE;
				t->inuse = 0;	/* remove timer */
			}
		}
	}

	/* reset timer_next if no timers found */
	if (!timer_next->inuse) timer_next = 0;
}

/************************************************************************************/
struct timer *
timer_declare(time,event)
unsigned int time;		/* time to wait in 10msec ticks */
char *event;
{
	struct timer *t;

	disable_interrupts();

	for (t=timers;t<&timers[MAX_TIMERS];t++) {
		if (!t->inuse) break;
	}

	/* out of timers? */
	if (t == &timers[MAX_TIMERS]) {
		enable_interrupts();
		return(0);
	}

	/* install new timer */
	t->event = event;
	t->time = time;
	if (!timer_next) {
		/* no timers set at all, so this is shortest */
		time_timer_set = time_now;
		start_physical_timer((timer_next = t)->time);
	} else if ((time + time_now) < (timer_next->time + time_timer_set)) {
		/* new timer is shorter than current one, so */
		timers_update(time_now - time_timer_set);
		time_timer_set = time_now;
		start_physical_timer((timer_next = t)->time);
	} else {
		/* new timer is longer, than current one */
	}
	t->inuse = TRUE;
	enable_interrupts();
	return(t);
}

/************************************************************************************/
void
timer_interrupt_handler() {
	timers_update(time_now - time_timer_set);
	
	/* start physical timer for next shortest time if one exists */
	if (timer_next) {
		time_timer_set = time_now;
		start_physical_timer(timer_next->time);
	}
}
