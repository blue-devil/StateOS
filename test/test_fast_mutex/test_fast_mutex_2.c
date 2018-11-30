#include "test.h"

static void proc2()
{
	unsigned event;

	event = mut_wait(mut2);                      assert_success(event);
	event = mut_give(mut2);                      assert_success(event);
	                                             assert(!"test program cannot be caught here");
}

static void proc1()
{
	unsigned event;
	                                             assert_stopped(tsk2);
	        tsk_startFrom(tsk2, proc2);
	event = tsk_kill(tsk2);                      assert_success(event);
	        tsk_stop();
}

static void test()
{
	unsigned event;

	event = mut_wait(mut2);                      assert_success(event);
	                                             assert_stopped(tsk1);
	        tsk_startFrom(tsk1, proc1);
	event = mut_give(mut2);                      assert_success(event);
	event = tsk_join(tsk1);                      assert_success(event);
}

void test_fast_mutex_2()
{
	int i;
	TEST_Notify();
	for (i = 0; i < PASS; i++)
		test();
}