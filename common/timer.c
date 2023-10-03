#include <sys/timerfd.h>
#include <timer.h>


/* global functions */
int timer_init(void){
	int fd;


	fd = timerfd_create(CLOCK_MONOTONIC, 0);

	if(fd == -1)
		return -1;

	timer_set(fd, 0);

	return fd;
}

int timer_set(int fd, long int ms){
	struct itimerspec time;


	time.it_value.tv_nsec = ms * 1000000;
	time.it_interval.tv_nsec = time.it_value.tv_nsec;
	time.it_value.tv_sec = 0;
	time.it_interval.tv_sec = 0;

	return -(timerfd_settime(fd, 0, &time, 0x0) != 0);
}
