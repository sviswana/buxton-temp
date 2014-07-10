#include <errno.h>
#include <string.h>
#include <poll.h>
#include <inttypes.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "buxton.h"

static BuxtonClient client=NULL;
static int fd =-1;
static struct pollfd pfd[1];
static int r;
static char * _layer = NULL;
static char * _group = NULL;
typedef struct vstatus {
	int status;
	BuxtonDataType type;
	union {
		char * sval;
		int32_t i32val;
		uint32_t ui32val;
		int64_t i64val;
		uint64_t ui64val;
		float fval;
		double dval;
		bool bval;
	};
} vstatus;
//static struct vstatus getVal;
/*Save errno*/
int saved_errno;
void save_errno(void)
{
	saved_errno = errno;
}
/*Open client connection*/
void sbuxton_open(void)
{
	if ((fd= buxton_open(&client)) <0 ){
		printf("Couldn't connect.\n");
		return;
	}
	printf("Connection successful.\n");
}
/*Make sure client connection is open*/
void client_connection(void)
{
	/*Check if client connection is open*/
	if(!client){
		/*Open connection if needed*/
		sbuxton_open();
	}
}
/*Public poll timeout setter*/
/*Polling*/
void run_poll(int timeout)
{
	pfd[0].fd = fd;
	pfd[0].events = POLLIN;
	pfd[0].revents = 0;
	r = poll(pfd, 1, timeout);
	
	if (r <= 0){
		printf("poll error\n");
		return;
	}
	if (!buxton_client_handle_response(client)){
		printf("bad response from daemon\n");
		return;
	}
}
/*Create group callback*/
void cg_cb(BuxtonResponse response, void *data)
{
	int *status = (int *)data;
	*status = 0;
	if (buxton_response_status(response) != 0){
		printf("Failed to create group.\n");
	} else {
		printf("Created group.\n");
		*status = 1;
	}
}
/*Initialization of group*/
void buxtond_set_group(char *group, char *layer)
{
	client_connection();
	save_errno();
	if(_layer){
		free(_layer);
	}
	if(_group){
		free(_group);
	}
	_layer = strdup(layer);
	if(!_layer){
		printf("Layer assignment failed. Aborting operation.\n");
		return;
	}
	_group = strdup(group);
	if(!_group){
		printf("Group assignment failed. Aborting operation.\n");
		return;
	}
	BuxtonKey g = buxton_key_create(_group, NULL, _layer, STRING);
	int status;
	if (buxton_create_group(client, g, cg_cb, &status, true)){
		printf("Create group call failed.\n");
	} else {
		printf("Switched to group: %s, layer: %s.\n", buxton_key_get_group(g), buxton_key_get_layer(g));
		errno = saved_errno;
	}
	buxton_key_free(g);
}
/*buxton_set_value callback for all buxton data types*/
void bs_cb(BuxtonResponse response, void *data){
	struct vstatus *ret = (struct vstatus*)data;
	//struct vstatus set = *ret;
	char * type = "Houston";
	ret->status = 0;
	if (buxton_response_status(response)){
		printf("Failed to set %s.\n", type);
		return;
	}
	ret->status =1;
	switch(ret->type){
		case STRING:
		{
			char * val = ret->sval;
			type = "string";
			break;
		}
		case INT32:
		{
			int32_t val = ret->i32val;
			type = "int32_t";
			/*GoToBuxtonResponseStatusCheck*/
			printf("Value to be set: %i(int32_t)\n", val);
			break;
		}
		case UINT32:
		{
			uint32_t val = ret->ui32val;
			type = "uint32_t";
			break;
		}
		case INT64:
		{
			int64_t val = ret->i64val;
			type = "int64_t";
			break;
		}
		case UINT64:
		{
			uint64_t val = ret->ui64val;
			type = "uint64_t";
			break;
		}
		case FLOAT:
		{
			float val = ret->fval;
			type = "float";
			break;
		}
		case DOUBLE:
		{
			double val = ret->dval;
			type = "double";
			break;
		}
		case BOOLEAN:
		{
			bool val = ret->bval;
			type = "bool";
			break;
		}
	}
	printf("Success: %s value has been set. ", type);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k), buxton_key_get_layer(k));
	buxton_key_free(k);	

}
void bg_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret= (struct vstatus *)data;
	struct vstatus get = *ret;
	char *type = "Houston";
	switch (get.type){
		case STRING:
		{
			ret->sval = *(char**)buxton_response_value(response);
			type = "string";
			break;
		}
		case INT32:
		{
			ret->i32val = *(int32_t*)buxton_response_value(response);
			type = "int32_t";
			break;
		}
		case UINT32:
		{
			ret->ui32val = *(uint32_t*)buxton_response_value(response);
			type = "uint32_t";
			break;
		}
		case INT64:
		{
			ret->i64val = *(int64_t*)buxton_response_value(response);
			type = "int64_t";
			break;
		}
		case UINT64:
		{
			ret->ui64val = *(uint64_t*)buxton_response_value(response);
			type = "uint64_t";
			break;
		}
		case FLOAT:
		{
			ret->fval = *(float*)buxton_response_value(response);
			type = "float";
			break;
		}
		case DOUBLE:
		{
			ret->dval = *(double*)buxton_response_value(response);
			type = "double";
			break;
		}
		case BOOLEAN:
		{
			ret->bval = *(bool*)buxton_response_value(response);
			type = "bool";
			break;
		}
	}
	ret->status = 0;
	if (buxton_response_status(response)){
		printf("Failed to get %s. \n", type);
		return;
	}
	ret->status = 1;
}
/*buxton_set_value callback and function for int32_t*/
void bsi32_cb(BuxtonResponse response, void *data)
{
	int32_t *ret=(int32_t*)data;
	if (buxton_response_status(response)){
		printf("Failed to set int32_t.\n");
		return;
	}
	printf("Value has been set: %i(int32_t)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k), 
		buxton_key_get_layer(k));	
}
void buxtond_set_int32(char *key, int32_t value)
{
	/*make sure client connection is open*/
	client_connection();
	/*check if a key has been created*/
	/*create key */
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, INT32);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = INT32;
	ret.i32val = value;
	save_errno();
	/*call buxton_set_value for type INT32*/
	if (buxton_set_value(client, _key, &value, bs_cb, &ret, true)){
		printf("Set int32_t call failed.\n");
		return;
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for int32_t*/
void bgi32_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get int32_t.\n");
		return;
	}
	ret->status=1;
	ret->i32val = *(int32_t*)buxton_response_value(response);
}
int32_t buxtond_get_int32(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, INT32);
	/*return value*/
	struct vstatus ret;
	ret.type = INT32;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bg_cb, &ret, true)){
		printf("Get int32_t call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.i32val;
}
/*buxton_set_value callback and function for string*/
void bss_cb(BuxtonResponse response, void *data)
{
	char **ret=(char**)data;
	if (buxton_response_status(response)){
		printf("Failed to set string.\n");
		return;
	}
	printf("Value has been set: %s(string)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k),
		buxton_key_get_layer(k));
}
void buxtond_set_string(char *key, char *value )
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, STRING);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = STRING;
	ret.sval = value;
	save_errno();
	/*set value*/
	if (buxton_set_value(client, _key, &value, bs_cb, &ret, true)){
		printf("Set string call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for string*/
void bgs_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get string.\n");
		return;
	}
	ret->status=1;
	ret->sval = *(char**)buxton_response_value(response);
}
char* buxtond_get_string(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, STRING);
	/*return value*/
	struct vstatus ret;
	ret.type = STRING;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bg_cb, &ret, true)){
		printf("Get string call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.sval;
}
/*buxton_set_value callback and function for uint32_t*/
void bsui32_cb(BuxtonResponse response, void *data)
{
	uint32_t *ret=(uint32_t*)data;
	if (buxton_response_status(response)){
		printf("Failed to set uint32_t.\n");
		return;
	}
	printf("Value has been set: %i(uint32_t)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k),
		buxton_key_get_layer(k));
}
void buxtond_set_uint32(char *key, uint32_t value)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, UINT32);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = UINT32;
	ret.ui32val = value;
	save_errno();
	if (buxton_set_value(client,_key, &value, bs_cb, &ret, true)){
		printf("Set uint32_t call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for uint32_t*/
void bgui32_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get uint32_t.\n");
		return;
	}		
	ret->status = 1;
	ret->i32val = *(uint32_t*)buxton_response_value(response);
}
uint32_t buxtond_get_uint32(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, UINT32);
	/*return value*/
	struct vstatus ret;
	ret.type = UINT32;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bg_cb, &ret, true)){
		printf("Get uint32_t call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.ui32val;
}
/*buxton_set_value callback and function for int64_t*/
void bsi64_cb(BuxtonResponse response, void *data)
{
	int64_t *ret = (int64_t*)data;
	if (buxton_response_status(response)){
		printf("Failed to set int64_t.\n");
		return;
	}
	printf("Value has been set: ""%"PRId64"(int64_t)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k),
		buxton_key_get_layer(k));
}
void buxtond_set_int64(char *key, int64_t value)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, INT64);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = INT64;
	ret.i64val = value;
	save_errno();
	if(buxton_set_value(client, _key, &value, bs_cb, &ret, true)){
		printf("Set int64_t call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for int64_t*/
void bgi64_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get int64_t.\n");
		return;
	}
	ret->status = 1;
	ret->i64val = *(int64_t*)buxton_response_value(response);
}
int64_t buxtond_get_int64(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, INT64);
	/*return value*/
	struct vstatus ret;
	ret.type = INT64;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bg_cb, &ret, true)){
		printf("Get int64_t call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.i64val;
}
/*buxton_set_value callback and function for uint64_t*/
void bsui64_cb(BuxtonResponse response, void *data)
{
	uint64_t *ret = (uint64_t*)data;
	if (buxton_response_status(response)){
		printf("Failed to set uint64_t.\n");
		return;
	}
	printf("Value has been set: ""%"PRIu64"(uint64_t)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k),
		buxton_key_get_layer(k));
}
void buxtond_set_uint64(char *key, uint64_t value)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, UINT64);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = UINT64;
	ret.ui64val = value;
	save_errno();
	if(buxton_set_value(client, _key, &value, bs_cb, &ret, true)){
		printf("Set uint64_t call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for uint64_t*/
void bgui64_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get uint64_t.\n");
		return;
	}
	ret->status = 1;
	ret->ui64val = *(uint64_t*)buxton_response_value(response);
}
uint64_t buxtond_get_uint64(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, UINT64);
	/*return value*/
	struct vstatus ret;
	ret.type = UINT64;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bg_cb, &ret, true)){
		printf("Get uint64_t call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.ui64val;
}
/*buxton_set_value callback and function for float*/
void bsf_cb(BuxtonResponse response, void *data)
{
	float *ret = (float*)data;
	if (buxton_response_status(response)){
		printf("Failed to set float.\n");
		return;
	}
	printf("Value has been set: %f(float)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k),
		buxton_key_get_layer(k));
}
void buxtond_set_float(char *key, float value)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, FLOAT);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = FLOAT;
	ret.fval = value;
	save_errno();
	if(buxton_set_value(client, _key, &value, bs_cb, &ret, true)){
		printf("Set float call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for float*/
void bgf_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get float.\n");
		return;
	}
	ret->status = 1;
	ret->fval = *(float*)buxton_response_value(response);
}
float buxtond_get_float(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, FLOAT);
	/*return value*/
	struct vstatus ret;
	ret.type = FLOAT;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bg_cb, &ret, true)){
		printf("Get float call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.fval;
}
/*buxton_set_value callback and function for double*/
void bsd_cb(BuxtonResponse response, void *data)
{
	double *ret = (double*)data;
	if (buxton_response_status(response)){
		printf("Failed to set double.\n");
		return;
	}
	printf("Value has been set: %e(double)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k),
		buxton_key_get_layer(k));
}
void buxtond_set_double(char *key, double value)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, DOUBLE);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = DOUBLE;
	ret.dval = value;
	save_errno();
	if(buxton_set_value(client, _key, &value, bs_cb, &ret, true)){
		printf("Set double call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for double*/
void bgd_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get double.\n");
		return;
	}
	ret->status = 1;
	ret->dval = *(double*)buxton_response_value(response);
}
double buxtond_get_double(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, DOUBLE);
	/*return value*/
	struct vstatus ret;
	ret.type = DOUBLE;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bg_cb, &ret, true)){
		printf("Get double call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.dval;
}
/*buxton_set_value callback and function for boolean*/
void bsb_cb(BuxtonResponse response, void *data)
{
	bool *ret = (bool*)data;
	if(buxton_response_status(response)){
		printf("Failed to set boolean.\n");
		return;
	}
	printf("Value has been set: %i(bool)\n", *ret);
	BuxtonKey k = buxton_response_key(response);
	printf("Key: %s, Group: %s, Layer: %s.\n", buxton_key_get_name(k), buxton_key_get_group(k),
		buxton_key_get_layer(k));
}
void buxtond_set_bool(char *key, bool value)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, BOOLEAN);
	/*Return value and status*/
	struct vstatus ret;
	ret.type = BOOLEAN;
	ret.bval = value;
	save_errno();
	if(buxton_set_value(client, _key, &value, bs_cb, &ret, true)){
		printf("Set bool call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
}
/*buxton_get_value callback and function for boolean*/
void bgb_cb(BuxtonResponse response, void *data)
{
	struct vstatus *ret = (struct vstatus*)data;
	ret->status=0;
	if (buxton_response_status(response)){
		printf("Failed to get boolean.\n");
		return;
	}
	ret->status = 1;
	ret->bval = *(bool*)buxton_response_value(response);
}
bool buxtond_get_bool(char *key)
{
	/*make sure client connection is open*/
	client_connection();
	/*create key*/
	BuxtonKey _key = buxton_key_create(_group, strdup(key), _layer, BOOLEAN);
	/*return value*/
	struct vstatus ret;
	ret.type = BOOLEAN;
	save_errno();
	/*get value*/
	if (buxton_get_value(client, _key, bgb_cb, &ret, true)){
		printf("Get bool call failed.\n");
	}
	if (!ret.status){
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	buxton_key_free(_key);
	return ret.bval;
}
/*create a client side group TODO: create BuxtonGroup type*/
BuxtonKey buxton_group_create(char *name, char *layer)
{
	client_connection();
	BuxtonKey ret = buxton_key_create(name, NULL, layer, STRING);
	return ret;
}
/*create a buxtond side group*/
void buxtond_create_group(BuxtonKey group)
{
	client_connection();
	if (buxton_create_group(client, group, NULL, NULL, true)){
		printf("Create group call failed.\n");
		buxton_key_free(group);
		return;
	}
	/*TODO*/
	//buxton_key_free(group);
}
BuxtonKey buxtond_create_group2(char *group_name, char *layer)
{
	client_connection();
	BuxtonKey group = buxton_key_create(group_name, NULL, layer, STRING);
	if (buxton_create_group(client, group, NULL, NULL, true)){
		printf("Create group call failed.\n");
		buxton_key_free(group);
		return;
	}
	return group;
	/*Buxton free key? TODO*/
}
/*buxton_remove_group callback and function*/
void rg_cb(BuxtonResponse response, void *data)
{
	if (buxton_response_status(response) != 0){
		printf("Failed to remove group.\n");
	} else {
		printf("Removed group.\n");
	}
}
void buxtond_remove_group(BuxtonKey group)
{
	client_connection();
	if (buxton_remove_group(client, group, rg_cb, NULL, true)){
		printf("Remove group call failed.\n");
	}
}
void buxtond_remove_group2(char *group_name, char *layer)
{
	client_connection();
	BuxtonKey group = buxton_group_create(group_name, layer);
	if (buxton_remove_group(client, group, rg_cb, NULL, true)){
		printf("Remove group call failed.\n");
	}
	buxton_key_free(group);
}
/*buxton_key_free insert char key name and type*/
void buxtond_key_free(char * key_name, BuxtonDataType type)
{
	BuxtonKey k = buxton_key_create(_group, strdup(key_name), _layer, type);
	buxton_key_free(k);
}

/*DEMONSTRATION*/
int main(void)
{
	/*Create group*/
	errno = 0;
	buxtond_set_group("tg_s0", "user");
	printf("set_group: 'tg_s0', 'user', Error number: %s.\n", strerror(errno));
	/*Test string setting*/
	srand(time(NULL));
	char * s="Watermelon";
	printf("value should be set to %s.\n", s);
	errno = 0;
	buxtond_set_string("tk_s1", s);
	printf("set_string: 'tg_s0', 'tk_s1', Error number: %s.\n", strerror(errno));
	/*Test string getting*/
	char * sv = buxtond_get_string("tk_s1");
	printf("Got value: %s(string).\n", sv);		
	printf("get_string: 'tk_s1', Error number: %s.\n", strerror(errno));
	/*Create group*/
	errno = 0;
	buxtond_set_group("tg_s1", "user");
	printf("set_group: 'tg_s1', Error number: %s.\n", strerror(errno));
	/*Test int32 setting*/
	srand(time(NULL));
	int32_t i=rand()%100+1;
	printf("value should be set to %i.\n",i);
	errno = 0;
	buxtond_set_int32("tk_i32", i);
	printf("set_int32: 'tg_s1', 'tk_i32', Error number: %s.\n", strerror(errno));

	errno = 0;
	buxtond_set_group("tg_s2", "user");
	printf("set_group: 'tg_s2', Error number: %s.\n", strerror(errno));
	srand(time(NULL));
	int32_t i2=rand()%1000+1;
	printf("Second value should be set to %i.\n", i2);
	errno = 0;
	buxtond_set_int32("tk_i32b", i2);
	printf("set_int32: 'tg_s2', 'tk_i32b', Error number: %s.\n", strerror(errno));
	/*Test int32 getting*/
	errno = 0;
	buxtond_set_group("tg_s1", "user");
	printf("set_group: 'tg_s1', Error number: %s.\n", strerror(errno));
	errno = 0;
	int32_t iv = buxtond_get_int32("tk_i32");
	printf("get_int32: 'tg_s1', 'tk_i32', Error number: %s.\n", strerror(errno));
	printf("Got value: %i(int32_t).\n", iv);
	errno = 0;
	buxtond_set_group("tg_s2", "user");
	printf("set_group: 'tg_s2', Error number: %s.\n", strerror(errno));
	errno = 0;
	int32_t i2v = buxtond_get_int32("tk_i32b");
	printf("Got value: %i(int32_t).\n", i2v);
	printf("get_int32: 'tg_s2', 'tk_i32b', Error number: %s.\n", strerror(errno));
	/*Test uint32 setting*/
	errno = 0;
	buxtond_set_group("tg_s3", "user");
	printf("set_group: 'tg_s3', Error number: %s.\n", strerror(errno));
	uint32_t ui32 = rand()%50+1;
	printf("value should be set to %u.\n", ui32);
	errno = 0;
	buxtond_set_uint32("tk_ui32", ui32);
	printf("set_uint32: 'tg_s3', 'tk_ui32', Error number: %s.\n", strerror(errno));
	/*Test uint32 getting*/
	errno = 0;
	uint32_t ui32v = buxtond_get_uint32("tk_ui32");
	printf("Got value: %i(uint32_t).\n", ui32v);
	printf("get_uint32: 'tg_s3', 'tk_ui32', Error number: %s.\n", strerror(errno));
	/*Test  int64 setting*/
	int64_t i64 = rand()%1000+1;
	printf("value should be set to ""%"PRId64".\n", i64);
	errno = 0;
	buxtond_set_int64("tk_i64", i64);
	/*Test int64 getting*/
	errno = 0;
	int64_t i64v = buxtond_get_int64("tk_i64");
	printf("Got value: ""%"PRId64"(int64_t).\n", i64v);
	printf("get_int64: 'tg_s3', 'tk_i64', Error number: %s.\n", strerror(errno));
	/*Test uint64 setting*/
	errno = 0;
	buxtond_set_group("tg_s0", "user");
	uint64_t ui64 = rand()%500+1;
	printf("value should be set to ""%"PRIu64".\n", ui64);
	errno = 0;
	buxtond_set_uint64("tk_ui64", ui64);
	/*Test uint64 getting*/
	errno = 0;
	uint64_t ui64v = buxtond_get_uint64("tk_ui64");
	printf("Got value: ""%"PRIu64"(uint64_t).\n", ui64v);
	printf("get_uint64: 'tg_s0', 'tk_ui64', Error number: %s.\n", strerror(errno));
	/*Test float setting*/
	float f = rand()%9+1;
	printf("value should be set to %e.\n", f);
	errno = 0;
	buxtond_set_float("tk_f", f);
	/*Test float getting*/
	errno = 0;
	float fv = buxtond_get_float("tk_f");
	printf("Got value: %e(float).\n", fv);
	printf("get_float: 'tg_s0', 'tk_f', Error number: %s.\n", strerror(errno));
	/*Test double setting*/
	double d = rand()%7000+1;
	printf("value should be set to %e.\n", d);
	errno = 0;
	buxtond_set_double("tk_d", d);
	/*Test double getting*/
	errno = 0;
	double dv = buxtond_get_double("tk_d");
	printf("Got value: %e(double).\n", dv);
	printf("get_double: 'tg_s0', 'tk_f', Error number: %s.\n", strerror(errno));
	/*Test boolean setting*/
	bool b = true;
	printf("value should be set to %i.\n", b);
	errno = 0;
	buxtond_set_bool("tk_b", b);
	/*Test boolean getting*/
	errno = 0;
	bool bv = buxtond_get_bool("tk_b");
	printf("Got value: %i(bool).\n", bv);		
	printf("get_bool: 'tg_s0', 'tk_b', Error number: %s.\n", strerror(errno));
	/*Close connection*/
	errno = 0;
	buxtond_remove_group2("tg_s1", "user");
	printf("Remove group: 'tg_s1', 'user' Error number: %s.\n", strerror(errno));
	buxtond_remove_group2("tg_s0", "user");
	buxtond_remove_group2("tg_s2", "user");
	buxtond_remove_group2("tg_s3", "user");
	buxton_close(client);
}
