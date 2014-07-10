/*The following program uses the Buxton API to create a key, "test_key_name", within a group, "test_group".
* If no group exists, the program will create a group and then set a value for the key.  If the key value 
* is already set, the program will generate a new random value and set that new value as the key value. */
#define GNU_SOURCE
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include "buxton.h"

void n_cb(BuxtonResponse response, void *data)
{
	bool *status = (bool *)data;
	BuxtonKey key;
	int32_t *value;
	char *name;
	
	if (buxton_response_status(response)){
		*status = false;
		printf("Failed to get notification.\n");
		return;
	}

	key = buxton_response_key(response);
	name = buxton_key_get_name(key);

	value = (int32_t*)buxton_response_value(response);
	if(value){
		printf("key %s was updated with new value %d.\n", name,*value);
	} else {
		printf("key %s was removed\n",name);
	}

	buxton_key_free(key);
	free(value);
	free(name);
}
/*pollfd structures:
	struct pollfd{
		int fd;		//file descriptor
		short events;	//requested events
		short revents; 	//returned events
	}
*/
int main(void)
{
	BuxtonClient client;
	BuxtonKey key;
	/*notification status*/
	bool status = true;
	/*set up polling array*/
	struct pollfd pfd[1];
	/*return for poll(). >0 on success, 0 on timeout, <0 on error*/
	int r;
	/*file descriptor*/
	int fd;
	/*number of times to repoll*/
	int repoll_count = 5;
	/*open buxton client connection*/
	if ((fd = buxton_open(&client)) < 0){
		printf("failed to open client connection.\n");
		return -1;
	}
	printf("Connection successful.\n");
	/*create a key*/
	key = buxton_key_create("test_group", "test_key_name", NULL, INT32);
	/*make sure key was created*/
	if (!key){
		printf("No key was returned by buxton_key_create.\n");
		return -1;
	}else{
		printf("A client side key, %s, was ", buxton_key_get_name(key));
		printf("created within group %s ", buxton_key_get_group(key));
		printf("and layer %s\n.",buxton_key_get_layer(key));
	}
	/*register key for notifications*/
	if (buxton_register_notification(client, key, n_cb, &status, false)) {
		printf("notification registration call failed.\n");
		return -1;
	}
	printf("registered for key notification.\n");
repoll:
	pfd[0].fd = fd;
	pfd[0].events = POLLIN;
	pfd[0].revents = 0;
	r = poll(pfd, 1, 5000);
	
	if (r < 0) {
		printf("poll error\n");
		return -1;
	} else if (r == 0) {
		if (repoll_count-- > 0) {
			goto out;
		}
		goto repoll;
	}

	if (!buxton_client_handle_response(client)) {
		printf("bad response from daemon\n");
                return -1;
	}

	if (!status) {
		printf("Failed to register for notification\n");
		return -1;
	}

	goto repoll;

out:
	if (buxton_unregister_notification(client, key, NULL, NULL, true)) {
		printf("Unregistration of notification failed\n");
		return -1;
	}

	buxton_key_free(key);
	buxton_close(client);

	return 0;
}
