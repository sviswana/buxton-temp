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

/*Open client connection*/
void sbuxton_open(void)

/*Make sure client connection is open*/
void client_connection(void)

/*Public poll timeout setter*/

/*Polling*/
void run_poll(int timeout)

/*buxton_set_value callback and function for int32_t*/
void bsi32_cb(BuxtonResponse response, void *data)
void buxtond_set_int32(BuxtonKey key, int32_t value)

/*buxton_set_value callback and function for string*/
void bss_cb(BuxtonResponse response, void *data)
void buxtond_set_string(BuxtonKey key, char *value)

/*buxton_set_value callback and function for uint32_t*/
void bsui32_cb(BuxtonResponse response, void *data)
void buxtond_set_uint32(BuxtonKey key, uint32_t value)

/*buxton_set_value callback and function for int64_t*/
void bsi64_cb(BuxtonResponse response, void *data)
void buxtond_set_int64(BuxtonKey key, int64_t value)

/*buxton_set_value callback and function for uint64_t*/
void bsui64_cb(BuxtonResponse response, void *data)
void buxtond_set_uint64(BuxtonKey key, uint64_t value)

/*buxton_set_value callback and function for float*/
void bsf_cb(BuxtonResponse response, void *data)
void buxtond_set_float(BuxtonKey key, float value)

/*buxton_set_value callback and function for double*/
void bsd_cb(BuxtonResponse response, void *data)
void buxtond_set_double(BuxtonKey key, double value)

/*buxton_set_value callback and function for boolean*/
void bsb_cb(BuxtonResponse response, void *data)
void buxtond_set_bool(BuxtonKey key, bool value)

/*create a client side group TODO: create BuxtonGroup type*/
BuxtonKey buxton_group_create(char *name, char *layer)

/*create a buxtond side group*/
void buxtond_create_group(BuxtonKey group)
BuxtonKey buxtond_create_group2(char *group_name, char *layer)

/*buxton_remove_group callback and function*/
void rg_cb(BuxtonResponse response, void *data)
void buxtond_remove_group(BuxtonKey group)
void buxtond_remove_group2(char *group_name, char *layer)

/*buxton_get_value callback and function*/
/* Questions: separate functions for different types, params- key and group, w/o type possible? */

/*buxton_notifications*/
void buxtond_register_notification(BuxtonKey key)
/* Question: key name and group name only?*/

/*Questions and Feedback:*/
	/*Possible to do different key create?*/
	/*"Primary keys": group, key name; layer, group;*/
	/*Other*/
