#define GNU_SOURCE
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "buxton.h"

static BuxtonClient client=NULL;
static int fd =-1;
void sbuxton_open(void)
{
	if ((fd= buxton_open(&client)) < 0){
		printf("Couldn't connect.\n");
		return;
	}
	printf("Connection successful.\n");
}
void bss_cb(BuxtonResponse response, void *data)
{
	char **ret=(char**)data;
	if(buxton_response_status(response)){
		printf("Failed to set string.\n");
		return;
	}
	printf("Value has been set: %s\n", *ret);
}
void buxtond_set_string(BuxtonKey key, char * value)
{
	if (!client){
		sbuxton_open;
	}
	if(buxton_set_value(client, key, &value, bss_cb, &value, true)){
		printf("Set string call failed. \n");
	}
}

int main(void){
	char * s="Watermelon\n";
	printf("%s", s);
	BuxtonKey key;
	BuxtonKey group;
	/*open client*/
	if (buxton_open(&client)<0){
		printf("Failed to open client.\n");
		return -1;
	}
	
	printf("Connection successful.\n");
	/*create a group*/
	group = buxton_key_create("tg_s0", NULL, "user", STRING);
	/*create a key*/
	key=buxton_key_create("tg_s0","tk_s0","user",STRING);	
	/*print group name*/
	printf("group: %s \n", buxton_key_get_group(key));	
	/*print the key name*/
	printf("key: %s \n", buxton_key_get_name(key));
	/*print the layer name*/
	printf("layer: %s \n", buxton_key_get_layer(key));
	/*set a group*/
	if (buxton_create_group(client, group, NULL, NULL, true)){
		printf("Create group call failed.\n");
		buxton_key_free(group);
		return;
	}
	buxton_key_free(group);
	/*set a value*/
	buxtond_set_string(key, s);
	/*free the key*/
	buxton_key_free(key);
	/*close the client*/
	buxton_close(client);



}
