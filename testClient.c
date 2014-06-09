#define _GNU_OURCE
#include "buxton.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	BuxtonClient client;
	int fd;

	if ((fd = buxton_open(&client)) < 0) {
		printf("couldn't connect\n");
		return -1;
}

/* Manipulate data, register for notifications, ... */

buxton_close(client);
return 0;
}
