#include "coap.h"
#include "coap_io.h"
typedef struct coap_resource_prop {
coap_resource_t * r;
unsigned char * name; /**< resource name */
unsigned char * title; /**< resource title */
unsigned char * ct; /**< Service time for resource */
unsigned char * rt;
unsigned char * iff;
int Obs;   /**< can be observed */
} coap_resource_prop;
#define Max_Res_Num 500
typedef struct Server_log_node {
long recieve_time;
int number_of_busy_threads;
struct Server_log_node * next;
}Server_log_node;
int wait_busy(long,int);
void  wait_sleep(long);
void handle_sigint(int);
void set_resource_parameters(coap_resource_prop * , char * );
void add_resource(coap_resource_prop *);
void init_resource_dir(str , coap_context_t *);
void print_statistics(str,Server_log_node *,Server_log_node *);
void CBD_bench_ticks(long * );
void process_request(coap_context_t * );
void wait_busy_time(long );
coap_context_t * copy_context(coap_context_t *);

