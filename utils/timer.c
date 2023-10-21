#include <stdint.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <utils/timer.h>


/* global functions */
int timer_init(void){
	int fd;


	fd = timerfd_create(CLOCK_MONOTONIC, 0);

	if(fd == -1)
		return -1;

	timer_set(fd, 0);

	return fd;
}

void timer_destroy(int fd){
	if(fd != -1)
		close(fd);
}

int timer_set(int fd, long int ms){
	struct itimerspec time;


	time.it_value.tv_nsec = (ms % 1000) * 1000000;
	time.it_value.tv_sec = ms / 1000;
	time.it_interval.tv_nsec = time.it_value.tv_nsec;
	time.it_interval.tv_sec = time.it_value.tv_sec;

	return -(timerfd_settime(fd, 0, &time, 0x0) != 0);
}

void timer_ack(int fd){
	uint64_t data;


	read(fd, &data, sizeof(data));
}
