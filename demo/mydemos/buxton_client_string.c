#define GNU_SOURCE
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "buxton.h"

void sv_cb(BuxtonResponse response, void *data)
{
	char **ret=(char**)data;
	printf("In sv_cb. String=%s\n", *ret);
	if (buxton_response_status(response)){
		printf("failed to set value\n");
		return;
	}
	printf("Value has been set: %s\n", *ret);
}
void my_set_group(BuxtonClient client, BuxtonKey key)
{
	BuxtonKey group;
	/*create a group*/
	group=buxton_key_create("test_group",NULL,"user",STRING);
	if(buxton_create_group(client,group,NULL,NULL,true)){
		printf("create group call failed.\n");
		buxton_key_free(group);
		return;
	}
	buxton_key_free(group);
}
void my_set_key(BuxtonClient client, BuxtonKey key)
{
	char * string="Watermelon";
	if(buxton_set_value(client, key, &string,sv_cb,&string,true)){
		printf("set value call failed.\n");
	}
			
}

int main(void){
	char * s="Watermelon\n";
	printf("%s", s);
	BuxtonClient client;
	int fd;/*file descriptor*/
	BuxtonKey key;
	/*open client*/
	if (buxton_open(&client)<0){
		printf("Failed to open client.\n");
		return -1;
	}
	
	printf("Connection successful.\n");

	/*create a key*/
	key=buxton_key_create("test_group","test_key_name","user",INT32);	
	/*print group name*/
	printf("group: %s \n", buxton_key_get_group(key));	
	/*print the key name*/
	printf("key: %s \n", buxton_key_get_name(key));
	/*print the layer name*/
	printf("layer: %s \n", buxton_key_get_layer(key));
	/*set a value*/
	my_set_key(client, key);
	/*free the key*/
	buxton_key_free(key);
	/*close the client*/
	buxton_close(client);



}
