/*
 * https.h
 *
 *  Created on: Mar 8, 2011
 *      Author: ant
 */

#ifndef HTTPS_H_
#define HTTPS_H_

#include <evhttp.h>

struct evhttp *evhttps_start(const char *address, u_short port, char *certfile, char *keyfile, char *password);

#endif /* HTTPS_H_ */
