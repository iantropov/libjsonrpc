#include "check_json_parser.h"
#include "check_json_parser_parse.h"
#include <event.h>

#define CB_ARG_VALUE 0xdffd
#define SUCCESS_CALL 0x02
#define ERROR_CALL 0x04

#define ARRAY "[2.15, 3]"
#define DOUBLE_ARRAY "[2.15, 3][2.15, 3]"
#define ARRAY_WITH_HALF "[2.15, 3][2."
#define ARRAY_HALF "15, 3]"

#define OBJECT "{\"Ключ_1\":123, \"Ключ_2\":false, \"Ключ_3\":\"Ключ_1212\"}"
#define OBJECT_PART_0 "{\"Ключ_1\":"
#define OBJECT_PART_1 "123,"
#define OBJECT_PART_2 "\"Ключ_2\":false"
#define OBJECT_PART_3 ", \"Ключ_3\":\"Ключ_1212\"}"


static struct json_parser *__jp;
static struct evbuffer *__buf;

static int __call_info;
static int __call_count;

static int __waiting_error;
static int __waiting_call_count;
static int __waiting_call_info;

static char *__current_json;

static void check_arg(void *arg)
{
	fail_unless(arg == (void *)CB_ARG_VALUE, "error cb_arg");
}

static void success(struct json_parser *jp, struct json_object *obj, void *arg)
{
	fail_unless(obj != NULL, "bad json_parser struct");
	fail_unless(jp != NULL, "bad result json_object");

	fail_unless(json_equals(obj, json_parser_parse(__current_json)) == 0, "Bad result");

	check_arg(arg);

	__call_count++;
	__call_info |= SUCCESS_CALL;
}

static void error(struct json_parser *jp, short what, void *arg)
{
	fail_unless(jp != NULL, "parser_error_parser_info");
	fail_unless((what & __waiting_error) != 0, "parser_error_reason");

	check_arg(arg);

	__call_count++;
	__call_info |= ERROR_CALL;
}

static void parse_part(struct json_parser *jp, struct evbuffer *buf, char *str)
{
	evbuffer_add(buf, str, strlen(str));
	json_parser_process(jp, buf->buffer, buf->buffer + buf->off);
}

static void setup()
{
	event_init();

	__jp = json_parser_new(success, error, (void *)CB_ARG_VALUE);
    fail_unless(__jp != NULL, "json_parser_init");

	__buf = evbuffer_new();

	__waiting_error = JSON_PARSE_ERROR;

	__call_count = __call_info = 0;
	__waiting_call_info = __waiting_call_count = 0;

	__current_json = ARRAY;
}

static void teardown()
{
	event_dispatch();

	json_parser_free(__jp);

	evbuffer_free(__buf);

	fail_unless(__waiting_call_count == __call_count, "Unsuspected count of cbs");
    fail_unless((__call_info & __waiting_call_info) != 0, "Unsuspected cbs are called");
}

START_TEST(test_success_0)
{
    parse_part(__jp, __buf, ARRAY);

    __waiting_call_count = 1;
    __waiting_call_info = SUCCESS_CALL;
}
END_TEST

START_TEST(test_success_1)
{
    parse_part(__jp, __buf, ARRAY);

    parse_part(__jp, __buf, ARRAY);

    __waiting_call_count = 2;
    __waiting_call_info = SUCCESS_CALL;
}
END_TEST

START_TEST(test_success_2)
{
    parse_part(__jp, __buf, DOUBLE_ARRAY);

    __waiting_call_count = 2;
    __waiting_call_info = SUCCESS_CALL;
}
END_TEST

START_TEST(test_success_3)
{
    parse_part(__jp, __buf, ARRAY_WITH_HALF);
    parse_part(__jp, __buf, ARRAY_HALF);

    __waiting_call_count = 2;
    __waiting_call_info = SUCCESS_CALL;
}
END_TEST

START_TEST(test_error_0)
{
    parse_part(__jp, __buf, "[--");

    __waiting_call_count = 1;
    __waiting_call_info = ERROR_CALL;
}
END_TEST

START_TEST(test_error_1)
{
    parse_part(__jp, __buf, ARRAY_WITH_HALF);
    parse_part(__jp, __buf, "--");

    __waiting_call_count = 2;
    __waiting_call_info = SUCCESS_CALL | ERROR_CALL;
}
END_TEST

START_TEST(test_error_2)
{
    parse_part(__jp, __buf, ARRAY_WITH_HALF);
    parse_part(__jp, __buf, "--");
    parse_part(__jp, __buf, "[12]");

    __waiting_call_count = 2;
    __waiting_call_info = SUCCESS_CALL | ERROR_CALL;
}
END_TEST

START_TEST(test_drain_0)
{
	__current_json = OBJECT;

    parse_part(__jp, __buf, OBJECT);

    fail_unless(json_parser_drain(__jp) == strlen(OBJECT), "error in drains");

    __waiting_call_count = 1;
    __waiting_call_info = SUCCESS_CALL;
}
END_TEST

START_TEST(test_drain_1)
{
	__current_json = OBJECT;

    parse_part(__jp, __buf, OBJECT_PART_0);
    fail_unless(json_parser_drain(__jp) == 0, "error in drains");

    parse_part(__jp, __buf, OBJECT_PART_1);
    fail_unless(json_parser_drain(__jp) == strlen(OBJECT_PART_0) + strlen(OBJECT_PART_1), "error in drains");

    evbuffer_drain(__buf, strlen(OBJECT_PART_0) + strlen(OBJECT_PART_1));

    parse_part(__jp, __buf, OBJECT_PART_2);
    fail_unless(json_parser_drain(__jp) == strlen(OBJECT_PART_2), "error in drains");

    evbuffer_drain(__buf, strlen(OBJECT_PART_2));

    parse_part(__jp, __buf, OBJECT_PART_3);
    fail_unless(json_parser_drain(__jp) == strlen(OBJECT_PART_3), "error in drains");

    __waiting_call_count = 1;
    __waiting_call_info = SUCCESS_CALL;
}
END_TEST


static TCase *json_parser_tcase(void)
{
	/* Json object creation test case */
	TCase *tc = tcase_create("json_parser");
	tcase_add_checked_fixture(tc, setup, teardown);

	tcase_add_test(tc, test_success_0);
	tcase_add_test(tc, test_success_1);
	tcase_add_test(tc, test_success_2);
	tcase_add_test(tc, test_success_3);

	tcase_add_test(tc, test_error_0);
	tcase_add_test(tc, test_error_1);
	tcase_add_test(tc, test_error_2);

	tcase_add_test(tc, test_drain_0);
	tcase_add_test(tc, test_drain_1);

	return tc;
}

Suite *make_json_parser_suite (void)
{
	Suite *s = suite_create ("json_parser");

	suite_add_tcase (s, json_parser_parse_tcase());

	suite_add_tcase(s, json_parser_tcase());

	return s;
}
