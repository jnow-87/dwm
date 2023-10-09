#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <utils/log.h>


/* static variables */
static FILE *log_fp = 0x0;
static bool log_debug = false;


/* local/static prototypes */
static void ptime(FILE *fp);


/* global functions */
int log_init(char const *file, bool debug){
	if(file == 0x0)
		return -1;

	log_debug = debug;
	log_fp = fopen(file, "a");

	return -(log_fp == 0x0);
}

void log_cleanup(void){
	if(log_fp)
		fclose(log_fp);
}

void log_print(log_lvl_t lvl, char const *file, size_t line, char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	log_vprint(lvl, file, line, fmt, lst);
	va_end(lst);
}

void log_vprint(log_lvl_t lvl, char const *file, size_t line, char const *fmt, va_list lst){
	if(lvl == LOG_DEBUG && !log_debug)
		return;

	ptime(log_fp);
	fprintf(log_fp, "%s:%zu:", file, line);
	vfprintf(log_fp, fmt, lst);
	fflush(log_fp);
}


/* local functions */
static void ptime(FILE *fp){
	time_t t;
	char s[32];


	time(&t);
	strftime(s, sizeof(s), "%a-%b-%Y %T%z", localtime(&t));

	fprintf(fp, "[%s] ", s);
}
