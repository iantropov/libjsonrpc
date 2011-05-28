/*
 * main.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include <stdio.h>

static void fail_unless(int a, char *str)
{
	if (!a)
		fprintf(stderr, "%s\n", str);
}


void test()
{

}

int main()
{
//	setup();
	test();
//	teardown();

	return 0;
}

//	eh = evhttps_start("127.0.0.1", 8888, "serv.pem", "serv.pem", "1234");
