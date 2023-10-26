#ifndef TIMER_H
#define TIMER_H


/* prototypes */
int timer_init(void);
void timer_destroy(int fd);

int timer_set(int fd, long int ms);
void timer_ack(int fd);


#endif // TIMER_H
