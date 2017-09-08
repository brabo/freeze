/*
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
 *
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: brabo
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

void give_me_a_break(int signo)
{
    printf("Saw signal # %d\r\n", signo);
    if (signo == SIGALRM) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        printf("SIGALRM incomming!\r\n");
        printf("After: tv_sec: %ld tv_usec: %d\r\n", tv.tv_sec, tv.tv_usec);
    }
}

int init(void)
{
    struct sigaction act = {};
    act.sa_handler = give_me_a_break;

    if (sigaction(SIGALRM, &act, NULL) < 0) {
        perror("Setting alarm signal");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct timeval tv;
    int ret;
    init();

    printf("Setting alarm 1 second.\r\n");
    ret = alarm(1);
    printf("alarm() returned %d\r\n", ret);

    printf("Setting alarm 2 seconds.\r\n");
    gettimeofday(&tv, NULL);
    printf("Before: tv_sec: %ld tv_usec: %ld\r\n", tv.tv_sec, tv.tv_usec);
    ret = alarm(2);
    printf("alarm() returned %d\r\n", ret);
    sleep(3);

    printf("Setting alarm 5 seconds.\r\n");
    gettimeofday(&tv, NULL);
    printf("Before: tv_sec: %ld tv_usec: %ld\r\n", tv.tv_sec, tv.tv_usec);
    ret = alarm(5);
    printf("alarm() returned %d\r\n", ret);
    sleep(6);

    printf("Setting alarm 15 seconds.\r\n");
    gettimeofday(&tv, NULL);
    printf("Before: tv_sec: %ld tv_usec: %ld\r\n", tv.tv_sec, tv.tv_usec);
    ret = ualarm(15, 0);
    printf("ualarm() returned %d\r\n", ret);
    sleep(20);

    return 0;
}
