#define GNU_SOURCE
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "buxton.h"

void sv_cb(BuxtonResponse response, void *data)
{
	int32_t *ret=(int32_t*)data;
	if (buxton_response_status(response)){
		printf("failed to set value\n");
		return;
	}
	printf("Value has been set: %i\n", *ret);
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
	int32_t rand_key_value;
	/*random integer value for the key*/
	srand(time(NULL));
	rand_key_value=rand()%100+1;
	printf("random value=%i\n", rand_key_value);
	if(buxton_set_value(client, key, &rand_key_value,sv_cb,&rand_key_value,true)){
		printf("set value call failed.\n");
	}
			
}
struct response_info{
	int32_t status;
	int32_t value;
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
	ret->value=*(int32_t*)buxton_response_value(response);
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
	key=buxton_key_create("test_group","test_key_name","user",INT32);	
	/*print group name*/
	printf("group: %s \n", buxton_key_get_group(key));	
	/*print the key name*/
	printf("key: %s \n", buxton_key_get_name(key));
	/*print the layer name*/
	printf("layer: %s \n", buxton_key_get_layer(key));

	/*get value*/
	if(buxton_get_value(client, key, gv_cb, &gotten,true)){
		printf("get_value call failed.\n");
	}
	if(!gotten.status){
		printf("No value has been set.\n");
		/*create a group and set a key value*/	
		printf("setting value...\n");
		my_set_group(client,key);
		my_set_key(client,key);
	}else{
	/*print the gotten value*/
		my_set_key(client,key);
		printf("the value is was changed from %i.\n", gotten.value);
	}
	/*free the key*/
	buxton_key_free(key);
	/*close the client*/
	buxton_close(client);



}
