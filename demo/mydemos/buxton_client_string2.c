#define GNU_SOURCE
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "buxton.h"
/*
void usv_cb(BuxtonResponse response, void *data)
{
	if (buxton_response_status(response)!= 0){
		printf("Failed to unset value.\n");	
	} else {
		printf("Unset value.\n");
	}
}*/
void sv_cb(BuxtonResponse response, void *data)
{
	char **ret=(char**)data;
	if (buxton_response_status(response)){
		printf("failed to set value\n");
		return;
	}
	printf("Value has been set: %s\n", *ret);
}
void my_set_group(BuxtonClient client)
{
	BuxtonKey group;
	/*create a group*/
	group=buxton_key_create("string_group",NULL,"user",STRING);
	if(buxton_create_group(client,group,NULL,NULL,true)){
		printf("create group call failed.\n");
		buxton_key_free(group);
		return;
	}
	buxton_key_free(group);
}
void my_set_key(BuxtonClient client, BuxtonKey key)
{
	char * rand_key_value;
	/*random integer value for the key*/
	srand(time(NULL));
	*rand_key_value=(char)rand()%100+33;
	printf("random value=%s\n", rand_key_value);
	if(buxton_set_value(client, key, &rand_key_value,sv_cb,&rand_key_value,true)){
		printf("set string call failed.\n");
	}
			
}
struct response_info{
	int status;
	char *value;
};

void gv_cb(BuxtonResponse response, void * data)
{
	struct response_info* ret=(struct response_info*)data;
	
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get value\n");
		return;
	}
	ret->status=1;
	ret->value=*(char**)buxton_response_value(response);
}
int main(void){
	BuxtonClient client;
	int fd;/*file descriptor*/
	BuxtonKey key;
	struct response_info gotten;
	/*open client*/
	if (buxton_open(&client)<0){
		printf("Failed to open client.\n");
		return -1;
	}
	
	printf("Connection successful.\n");

	/*create a key*/
	key=buxton_key_create("string_group","test_string","user",STRING);

	/*get value*/
	if(buxton_get_value(client, key, gv_cb, &gotten,true)){
		printf("get_value call failed.\n");
	}
	if(!gotten.status){
		printf("No value has been set.\n");
		/*create a group and set a key value*/	
		printf("setting value...\n");
		my_set_group(client);
		my_set_key(client,key);
	}else{
	/*print the gotten value*/
		my_set_key(client,key);
		printf("the value is was changed from %s.\n", gotten.value);
	}
	/*free the key*/
	buxton_key_free(key);
	/*close the client*/
	buxton_close(client);



}
