#ifndef LOG_H
#define LOG_H


#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>


/* macros */
#define ERROR(fmt, ...)		({ log_print(LOG_ERROR, __FILE__, __LINE__, "error: " fmt, ##__VA_ARGS__); -1; })
#define STRERROR(fmt, ...)	ERROR(fmt ": %s\n", ##__VA_ARGS__, strerror(errno))
#define EEXIT(fmt, ...)		{ ERROR(fmt, ##__VA_ARGS__); exit(1); }
#define INFO(fmt, ...)		log_print(LOG_INFO, __FILE__, __LINE__, "info: " fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)		log_print(LOG_DEBUG, __FILE__, __LINE__, "debug: " fmt, ##__VA_ARGS__)


/* types */
typedef enum{
	LOG_ERROR = 1,
	LOG_INFO,
	LOG_DEBUG,
} log_lvl_t;


/* prototypes */
int log_init(char const *file, bool debug);
void log_cleanup(void);

void log_print(log_lvl_t lvl, char const *file, size_t line, char const *fmt, ...);
void log_vprint(log_lvl_t lvl, char const *file, size_t line, char const *fmt, va_list lst);


#endif // LOG_H
