/*
 * string_functions.c
 *
 *  Created on: Feb 27, 2011
 *      Author: ant
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define UNI_LEN 4

static void utf_1(unsigned short x, unsigned char *res)
{
/*	make from 16bit hex number a 3 bytes utf8 symbol from third area */
	unsigned char p1 = 224, p2 = 128, p3 = p2;
	unsigned short xp;
	/*sizeof x = 16*/
	xp = x;
	xp >>= 12;
	p1 += xp;

	xp = x;
	xp <<= 4;
	xp >>= 10;
	p2 += xp;

	xp = x;
	xp %= 64;
	p3 += xp;

	res[0] = p1;
	res[1] = p2;
	res[2] = p3;
}

static void utf_2(unsigned short x, unsigned char *res)
{
/*	make from 11bit hex number a 2 bytes utf8 symbol from second area */
	unsigned char p1 = 192, p2 = 128;
	unsigned short xp;
	/*sizeof x = 16*/
	xp = x;
	xp >>= 6;
	p1 += xp;

	xp = x;
	xp %= 64;
	p2 += xp;

	res[0] = p1;
	res[1] = p2;
}

static void esc_seq(char *str, char *dest, int *off, int *dest_off)
{
	char temp_str[UNI_LEN + 1];
	unsigned short cv;
	unsigned char res_1[3], res_2[2];
	switch (str[++(*off)]) {
	case 'b':
		strcat(dest, "\b");
		break;
	case 'n':
		strcat(dest, "\n");
		break;
	case 'r':
		strcat(dest, "\r");
		break;
	case 'f':
		strcat(dest, "\f");
		break;
	case 't':
		strcat(dest, "\t");
		break;
	case '\\':
		strcat(dest, "\\");
		break;
	case '/':
		strcat(dest, "/");
		break;
	case '\"':
		strcat(dest, "\"");
		break;
	case 'u':
		memset(temp_str, '\0', UNI_LEN);
		memcpy(temp_str, str + ++(*off), UNI_LEN);
		sscanf(temp_str, "%x", (int *)&cv);
		if ((cv >= 0x0) && (cv <= 0x7f))
			memcpy(dest + (*dest_off), &cv, 1);
		if ((cv >= 0x80) && (cv <= 0x7ff)) {
			utf_2(cv, res_2);
			memcpy(dest + (*dest_off), res_2, 2);
			dest_off++;
		}
		if ((cv >= 0x800) && (cv <= 0xffff)) {
			utf_1(cv, res_1);
			memcpy(dest + (*dest_off), res_1, 3);
			(*dest_off) += 2;
		}
		(*off) += 3;
		break;
	}
	(*off)++;
	(*dest_off)++;
}

char *interpretate_esc_seq(char *str, int len)
{
	int off = 0, dest_off = 0;
	char *dest;
	unsigned char s;

	dest = (char *)malloc(len + 1);
	if (dest == NULL)
		return NULL;
	/*
	uni_area_1 = 0xc0..0xdf uni;
	uni_area_2 = 0xe0..0xef uni uni;
	uni_area_3 = 0xf0..0xf7 uni uni uni;
	*/
	while (off < len) {
		s = str[off];
		if ((s >= 0xc0) && (s <= 0xdf)) {
			memcpy(dest + dest_off, str + off, 2);
			off += 2;
			dest_off += 2;
			continue;
		}
		if ((s >= 0xe0) && (s <= 0xef)) {
			memcpy(dest + dest_off, str + off, 3);
			off += 3;
			dest_off += 3;
			continue;
		}
		if ((s >= 0xf0) && (s <= 0xf7)) {
			memcpy(dest + dest_off, str + off, 4);
			off += 4;
			dest_off += 4;
			continue;
		}
		if (str[off] == '\\') {
			esc_seq(str, dest, &off, &dest_off);
			continue;
		}
		dest[dest_off++] = str[off++];
	}
	dest[len] = '\0';
	return dest;
}

char *string_copy(char *source)
{
	int len = strlen(source);
	char *dest = (char *)malloc(len + 1);
	if (dest == NULL)
		return NULL;

	return strcpy(dest, source);
}
