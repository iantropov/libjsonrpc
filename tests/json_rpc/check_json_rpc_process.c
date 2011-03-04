#include "check_json_rpc_process.h"

#include <event.h>

struct timeval tm;

void test(int sock, short t, void *arg)
{
	fail_unless(1 == 2, "good news guy");
}

START_TEST (test_valid)
{
	tm.tv_usec = 0;
	struct event *ev = (struct event *)malloc(sizeof(struct event));
	fail_unless(ev != NULL, "malloc error");
	event_init();
	event_set(ev, (int)0, EV_TIMEOUT, test, NULL);
	int ret = event_add(ev, &tm);
	fail_unless(ret == 0, "event_add fail");
	event_dispatch();
}
END_TEST


TCase *json_rpc_process_tcase(void)
{
	/* Json object creation test case */
	TCase *tc = tcase_create ("json_rpc_process");
	tcase_add_test (tc, test_valid);

	return tc;
}
