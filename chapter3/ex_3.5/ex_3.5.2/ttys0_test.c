#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <termios.h>
#include <pthread.h>

const char *info = "hello hello hello ccccccccccccccccccccccc\n";
int write_flag = 0;
int quit_flag = 0;

void sighandle(int signo)
{
	write_flag = 1;
}

void sigquit(int signo)
{
	quit_flag = 1;
}

void *output(void *arg)
{
	int ret = 0;
	char buffer[64] = {0};
	int fd = (int)arg;

	while (!quit_flag)
	{
		ret = read(fd, buffer, 16);
		if (ret > 0) {
			buffer[ret] ='\0';
			printf("%s", buffer);
		}

		sleep(1);
	}

	return 0;
}

int main (int argc, char *argv[])
{
	int ret = 0;
	int fd;
	struct termios ttys0_term;
	pthread_t pthread_n;

	signal(SIGALRM, sighandle);
	signal(SIGTERM, sigquit);
	signal(SIGINT, sigquit);

	fd = open("/dev/ttyS0", O_RDWR);
	if (fd < 0 ) {
		fprintf(stderr, "fail open /dev/ttyS0\n");
		return -1;
	}

	ret =  tcgetattr(fd, &ttys0_term);
	if (ret < 0) {
		perror("get attr fail");
		goto end;
	}
	/*set input out baudrate*/
	cfsetispeed(&ttys0_term, B57600);
	cfsetospeed(&ttys0_term, B57600);

	ret = tcsetattr(fd, TCSAFLUSH, &ttys0_term);
	if (ret < 0 ) {
		perror("set speed fail");
		goto end;
	}

	ttys0_term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
	ret = tcsetattr(fd, TCSAFLUSH, &ttys0_term);
	if (ret < 0 ) {
		perror("set input parameter fail");
		goto end;
	}

	ttys0_term.c_oflag &= ~OPOST;

	ret = tcsetattr(fd, TCSAFLUSH, &ttys0_term);
	if (ret < 0 ) {
		perror("set control output  parameter fail");
		goto end;
	}

	ttys0_term.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

	ret = tcsetattr(fd, TCSAFLUSH, &ttys0_term);
	if (ret < 0 ) {
		perror("set control local  parameter fail");
		goto end;
	}

	ttys0_term.c_cflag &= ~(CSIZE | PARENB);
	ttys0_term.c_cflag |= CS8 | CREAD;
	ret = tcsetattr(fd, TCSAFLUSH, &ttys0_term);
	if (ret < 0 ) {
		perror("set control control  parameter fail");
		goto end;
	}

	ret = pthread_create(&pthread_n, NULL, output, (void *)fd);
	if (ret < 0) {
		fprintf(stderr, "create thread fail\n");
		goto end;
	}

	while (!quit_flag) {
		if (write_flag) {
			write(fd, info, strlen(info));
			write_flag = 0;
		}
		sleep(1);
	}

	pthread_join(pthread_n, NULL);
end:
	close(fd);

	return 0;
}
