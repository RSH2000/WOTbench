typedef struct cbclient_wl_node {
char *uri;
int send_timestamp;
int iat;
int server_code;
struct cbclient_wl_node * next;
}cbclient_wl_node;


typedef struct cbclient_result_node {
int server_code;
coap_tick_t max_wait;
int resp_time;
int ready;
int service_time;
int send_time;
int receive_time;
int send_iat;
int ts[5];
}cbclient_result_node;

typedef struct req_list{
struct req_list *tail;
struct req_list *next;
struct req_list *head;
int reqnum;
int fds;
}req_list;
