/* coap -- simple implementation of a modified CoAP server with the capability of adding removing resources
 *
 * Copyright (C) 2016 - Raoufeh Hashemian <rhashem@ucalgary.ca>
 *
 * This file is part of the CoAP-Docker Project
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define THPOOL_DEBUG
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
// Added by RSH
#include "thpool.h"
#include "server.h"
/* temporary storage for dynamic resource representations */
volatile sig_atomic_t quit;
/* changeable clock base (see handle_put_time()) */
static time_t clock_offset;
static time_t my_clock_base = 0;
struct coap_resource_t *time_resource = NULL;

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

// WoT-Bench variables


// The time-stamp when the server program was started , used  for base-time
long start_time;
// Global variable to signal reading to other threads
int reading;
int Busy_Cycles;
/* SIGINT handler: set quit to 1 for graceful termination */
void
handle_sigint(int signum)
{
    quit = 1;
}

#define INDEX "This is the Server Emulator for WoT-bench benchmarking tool!"
void wait_busy_time(long service_time)
{
    struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	long exit_time =  currentTime.tv_sec * (int)1e6 + currentTime.tv_usec + service_time;
	while (1){
        gettimeofday(&currentTime, NULL);
        if (exit_time <=  currentTime.tv_sec * (int)1e6 + currentTime.tv_usec)
            break;
	}
	return;
}
int wait_busy(long service_time,int Busy_Cycles)
{
    int i=0,j=0;
    int volatile d=0;
    int volatile a[1000];
    for(i=0; i<Busy_Cycles; i++)
        a[i]=i;
    for (i=0; i<service_time; i++)
    {
        for (j=0; j<Busy_Cycles - 1; j++)
        {
            a[j]=a[j+1]+d;
            a[j+1]=d;
            d=j;
        }

    }
    return(d);

}
void  wait_sleep(long service_time)
{
    struct timespec req,rem;
    long nano_service_time=service_time*1000;
    req.tv_sec=nano_service_time/(long)1000000000;
    req.tv_nsec=nano_service_time%1000000000+1;
    rem.tv_sec=0;
    rem.tv_nsec=0;
    nanosleep(&req,&rem);
    return;
}
void
hnd_get_wot_bench(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer UNUSED_PARAM,
              coap_pdu_t *request UNUSED_PARAM,
              str *token UNUSED_PARAM,
              coap_pdu_t *response)
{
    unsigned char buf[3];
    char RES_VALUE[12];
    coap_attr_t *attr;
    double res_value;
    long service_time=0;
    long send_time;
    long bsleep_time;

    response->hdr->code = COAP_RESPONSE_CODE(205);
    coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
    coap_add_option(response, COAP_OPTION_MAXAGE,
                    coap_encode_var_bytes(buf, 0x2ffff), buf);
    // int res_value = rand()*10000+1;
    attr = coap_find_attr(resource,(const unsigned char *)"ct", 2);
    service_time=atol((const char *)attr->value.s);
    sprintf(RES_VALUE,"%ld", service_time);
    CBD_bench_ticks(&bsleep_time);
    if (Busy_Cycles == 0)
        wait_sleep(service_time);
    else
        wait_busy_time(service_time);

    coap_add_data(response,strlen((const char *)RES_VALUE),(char *)RES_VALUE);
    CBD_bench_ticks(&send_time);
    printf("%s,%ld,%ld\n",RES_VALUE,send_time - start_time,send_time - bsleep_time);
}
void
hnd_put_wot_bench(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token ,
              coap_pdu_t *response)
{
    coap_tick_t t;
    size_t size;
    unsigned char *data;

    /* FIXME: re-set my_clock_base to clock_offset if my_clock_base == 0
     * and request is empty. When not empty, set to value in request payload
     * (insist on query ?ticks). Return Created or Ok.
     */

    /* if my_clock_base was deleted, we pretend to have no such resource */
    response->hdr->code =
        my_clock_base ? COAP_RESPONSE_CODE(204) : COAP_RESPONSE_CODE(201);

    resource->dirty = 1;

    coap_get_data(request, &size, &data);

    if (size == 0)		/* re-init */
        my_clock_base = clock_offset;
    else
    {
        my_clock_base = 0;
        coap_ticks(&t);
        while(size--)
            my_clock_base = my_clock_base * 10 + *data++;
        my_clock_base -= t / COAP_TICKS_PER_SECOND;
    }
}

void
hnd_get_index(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer UNUSED_PARAM,
              coap_pdu_t *request UNUSED_PARAM,
              str *token UNUSED_PARAM,
              coap_pdu_t *response)
{
    unsigned char buf[3];
    printf("default Handler Called\n");
    usleep(500);
    printf("slept for 500 usec\n");

    response->hdr->code = COAP_RESPONSE_CODE(205);

    coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

    coap_add_option(response, COAP_OPTION_MAXAGE,
                    coap_encode_var_bytes(buf, 0x2ffff), buf);

    coap_add_data(response, strlen(INDEX), (unsigned char *)INDEX);
}

void
set_resource_parameters(coap_resource_prop * p, char * line)
{
    unsigned char * pch = strtok (line,",");
    int i=0;
    while (pch != NULL)
    {
        i++;
        switch (i)
        {

        case 1:
            p->name=malloc(strlen(pch)*sizeof(unsigned char));
            strcpy(p->name,pch);
            break;
        case 2:
            p->title=malloc(strlen(pch)*sizeof(unsigned char));
            strcpy(p->title,pch);
            break;
        case 3:
            p->ct=malloc(strlen(pch)*sizeof(unsigned char));
            strcpy(p->ct ,pch);
            break;
        case 4:
            p->rt=malloc(strlen(pch)*sizeof(unsigned char));
            strcpy(p->rt,pch);
            break;
        case 5:
            p->iff=malloc(strlen(pch)*sizeof(unsigned char));
            strcpy(p->iff,pch);
            break;
        case 6:
            if (strcmp(pch,"FALSE"))
                p->Obs=0;
            else
                p->Obs=1;
            break;
        default:
            printf("Error parsing resource entry");
        }
        pch = strtok (NULL, ",");
    }
}
void add_resource(coap_resource_prop *p)
{

    p->r = coap_resource_init(p->name, strlen(p->name), 0);

    coap_register_handler(p->r, COAP_REQUEST_GET, hnd_get_wot_bench);
    coap_register_handler(p->r, COAP_REQUEST_PUT, hnd_put_wot_bench);
    coap_add_attr(p->r, (unsigned char *)"ct", 2, p->ct, strlen(p->ct), 0);
    coap_add_attr(p->r, (unsigned char *)"title", 5, p->title,strlen(p->title), 0);
    coap_add_attr(p->r, (unsigned char *)"rt", 2, p->rt,strlen(p->rt), 0);
    coap_add_attr(p->r, (unsigned char *)"if", 2, p->iff,strlen(p->iff), 0);
    p->r->observable = p->Obs;
}
void
init_resource_dir(str resources_file, coap_context_t *ctx)
{
    char  line[1000];
    FILE* rfile = NULL;
    if (!resources_file.s || (resources_file.length && resources_file.s[0] == '-'))
    {

       printf("No resource file specified, exiting ....");
       exit(-1);
    }
    else
    {
        if (!(rfile = fopen((char *)resources_file.s, "r")))
        {
           printf("Unable to open resources files, exiting ....");
           exit(-1);
        }
    }
    coap_resource_prop * p=NULL;
    while(fgets(line,1000,rfile))
    {
        if(strlen(line)>4)
        {
            p= malloc(sizeof(coap_resource_prop));
            if(!p)
            {
                printf("Failed to allocate memory for resource properties, exiting ...\n");
                exit(-1);
            }
            set_resource_parameters(p,line);
            add_resource(p);
            coap_add_resource(ctx, p->r);
            free(p);
        }
    }
    coap_resource_t *r;
    r = coap_resource_init(NULL, 0, 0);
    coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);
    coap_add_resource(ctx, r);
}

coap_context_t *
get_context(const char *node, const char *port)
{
    coap_context_t *ctx = NULL;
    int s;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
    hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

    s = getaddrinfo(node, port, &hints, &result);
    if ( s != 0 )
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return NULL;
    }

    /* iterate through results until success */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        coap_address_t addr;

        if (rp->ai_addrlen <= sizeof(addr.addr))
        {
            coap_address_init(&addr);
            addr.size = rp->ai_addrlen;
            memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

            ctx = coap_new_context(&addr);
            if (ctx)
            {
                goto finish;
            }
        }
    }

    fprintf(stderr, "no context available for interface '%s'\n", node);

finish:
    freeaddrinfo(result);
    return ctx;
}

// ********copy context *************
coap_context_t *
copy_context(coap_context_t *src)
{
    coap_context_t * dst =coap_malloc_type(COAP_CONTEXT, sizeof(coap_context_t));
    memset(dst, 0, sizeof( coap_context_t ) );
    dst->endpoint=src->endpoint;
    dst->resources = src->resources;
    dst->sockfd= src->sockfd;
    dst->network_read=src->network_read;
    dst->network_send=src->network_send;
    dst->message_id=src->message_id;
return dst;
}
// **** reimplimented the coap_read function to signal reading****
int coap_server_read(coap_context_t * ctx){
  ssize_t bytes_read = -1;
  coap_packet_t *packet;
  coap_address_t src;
  //printf("\ngoing to read packet\n");
  coap_address_init(&src);
  bytes_read = ctx->network_read(ctx->endpoint, &packet);
  reading=0;
  if ( bytes_read < 0 ) {
    warn("coap_read: recvfrom");
    return 0;
  }
  else{
       // printf("\nHandling message ...");
  coap_handle_message(ctx, packet);
  coap_free_packet(packet);
  }
  return bytes_read;
}
// ************ processing request by thread ********//
void process_request(coap_context_t * ctx_org)
{
    struct timeval tv, *timeout;
    coap_queue_t *nextpdu;
    coap_tick_t now;
    long fin_time,sst_time;
    CBD_bench_ticks(&sst_time);
    coap_context_t *ctx =coap_malloc_type(COAP_CONTEXT, sizeof(coap_context_t));
    memset(ctx, 0, sizeof( coap_context_t ) );
    ctx->endpoint=ctx_org->endpoint;
    ctx->resources = ctx_org->resources;
    ctx->sockfd= ctx_org->sockfd;
    ctx->network_read=ctx_org->network_read;
    ctx->network_send=ctx_org->network_send;
    ctx->message_id=ctx_org->message_id;
    //copy_context(ctx_org);/* read received data */
    int  result=coap_server_read(ctx);
    if(result)
    {
        CBD_bench_ticks(&fin_time);
        nextpdu = coap_peek_next(ctx);
        coap_ticks(&now);
        while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime)  /* If the sendqueue is not empty, no ACK was received, so try to retrasmit */
        {
            coap_retransmit(ctx, coap_pop_next(ctx) );
            nextpdu = coap_peek_next(ctx);
        }
        if ( nextpdu && nextpdu->t <= COAP_RESOURCE_CHECK_TIME )
        {
            /* set timeout if there is a pdu to send before our automatic timeout occurs */
            tv.tv_usec = ((nextpdu->t) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
            tv.tv_sec = (nextpdu->t) / COAP_TICKS_PER_SECOND;
            timeout = &tv;
        }
        else
        {
            tv.tv_usec = 0;
            tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
            timeout = &tv;
        }

        if (ctx->observe)
        {
            while(ctx->observe)
            {

#ifndef WITHOUT_OBSERVE
                /* check if we have to send observe notifications */
                coap_check_notify(ctx);
#endif /* WITHOUT_OBSERVE */
                usleep(1000);
            }
        }

    }
    free(ctx);
    return;
}
// *************Timer function ********
void
CBD_bench_ticks(long * t)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *t = tv.tv_sec*1000000 +tv.tv_usec ;
}


// *****************saving statistics **************
void print_statistics(str results_file,Server_log_node * head,Server_log_node * tail)
{
    static FILE *sfile=NULL;
    Server_log_node * tmp;
    int nofile=0;
    int i=0;
    if (!results_file.s || (results_file.length && results_file.s[0] == '-'))
    {

        sfile = stdout;
        nofile=1;
    }
    else
    {
        if (!(sfile = fopen((char *)results_file.s, "w")))
        {
            perror("fopen");
            exit(1);
        }
    }
    fprintf(sfile,"index,Receive time,Number of active Req");
    while(head !=tail){
        i++;
        fprintf(sfile,"\n%d,%ld,%d",i,head->recieve_time,head->number_of_busy_threads);
        tmp=head;
        head=head->next;
        free(tmp);
    }
    if(!nofile)
        fclose(sfile);
    return;
}
void
usage( const char *program)
{
    const char *p;

    p = strrchr( program, '/' );
    if ( p )
        program = ++p;
    fprintf( stderr,"Welcome to WoT-Bench Server Emulator\n");

    fprintf( stderr, "This program is a multi-threaded CoAP server \n"
             "developed as part of WoT-Bench project\n\n"
             "usage: %s [-R resource_file] [-T number of threads0] [-O output_stat_file \n\n"
             "\t-R the path to the file that contains resource tree information\n"
             "\t-O the path to the file to save server logs\n"
             "\t-T num\t\tnumber of threads, default 5\n"
             "\t-D duration of time to run the server default 3600 second\n"
             "\t-v num\t\tverbosity level (default: 3)\n",p);
}
int
main(int argc, char **argv)
{
    coap_context_t  *ctx;
    fd_set readfds;
    int tr=0;
    int success=0;
    int result;
    struct timeval tv;
    char addr_str[NI_MAXHOST] = "::";
    char port_str[NI_MAXSERV] = "5683";
    int opt;
    coap_log_t log_level = LOG_WARNING;
    Server_log_node * log_list;
    Server_log_node * current_req_log;
    long recieve_time=0;
    long now=0;
    int number_of_threads=5; // default number of threads
    int Duration=3600;
    static str results_file = { 0, NULL }; /* output file name */
    static str resources_file = { 0, NULL }; /* output file name */
    Busy_Cycles = 0;
    while ((opt = getopt(argc, argv, "A:B:D:O:P:R:T:v:")) != -1)
    {
        switch (opt)
        {
        case 'A' :
            strncpy(addr_str, optarg, NI_MAXHOST-1);
            addr_str[NI_MAXHOST - 1] = '\0';
            break;
        case 'B' :
            Busy_Cycles=atoi(optarg);
            break;
        case 'p' :
            strncpy(port_str, optarg, NI_MAXSERV-1);
            port_str[NI_MAXSERV - 1] = '\0';
            break;
        case 'D' :
            Duration=atoi(optarg);
            break;
        case 'O' :
            results_file.length = strlen(optarg);
            results_file.s = (unsigned char *)coap_malloc(results_file.length + 1);
            if (!results_file.s)
            {
                fprintf(stderr, "\nError: Cannot set output file: insufficient memory, exiting CDB-server ...\n");
                exit(-1);
            }
            else
                memcpy(results_file.s, optarg, results_file.length + 1);
            break;
        case 'T' :
            number_of_threads=atoi(optarg);
            printf("Initializing a thread pool with %d initial threads\n\n",number_of_threads);
            break;
        case 'R' :
            resources_file.length = strlen(optarg);
            resources_file.s = (unsigned char *)coap_malloc(resources_file.length + 1);
            if (!resources_file.s)
            {
                fprintf(stderr, "\nError: Cannot set resource file: insufficient memory, exiting CDB-server...\n");
                exit(-1);
            }
            else
                memcpy(resources_file.s, optarg, resources_file.length + 1);
            break;
        case 'v' :
            log_level = strtol(optarg, NULL, 10);
            break;
        default:
            usage( argv[0]);
            exit( 1 );
        }
    }


    coap_set_log_level(log_level);
    signal(SIGINT, handle_sigint);
    log_list = (Server_log_node *)malloc(sizeof(Server_log_node));
    current_req_log= log_list;

    ctx = get_context(addr_str, port_str);
    if (!ctx){
        printf("Unable to create context, exiting ....");
        exit(-1);
    }
    init_resource_dir(resources_file,ctx);
    threadpool thpool = thpool_init(number_of_threads);

    reading=0;
    CBD_bench_ticks(&start_time);
    quit=0;
    while ( !quit)
    {
        FD_ZERO(&readfds);
        FD_SET( ctx->sockfd, &readfds );
        if(reading)
            continue;
        CBD_bench_ticks(&now);
        if ((now-start_time)/1000000 >Duration)
            break;
        tv.tv_sec=1;
        tv.tv_usec=2;
       // printf("\nchecking for packet ...");
        result = select( ctx->sockfd+1, &readfds, 0, 0, &tv );
        if ( result < 0 )  		/* error */
        {
            if (errno != EINTR)
                perror("select");
        }
        else if ( result > 0 )  	/* initialize a thread to read from socket */
        {
            if ( FD_ISSET( ctx->sockfd, &readfds ) )
            {
                //printf("\nReceived a request ...\n");
                reading = 1;
                CBD_bench_ticks(&recieve_time);
                success=-1;
                tr=0;
                while(success)
                {
                    success=thpool_add_work(thpool, (void*)process_request,(void *)ctx);
                    tr++;
                }
                if(tr>1)
                    printf("\nError: Threads were initiated in %d tries.",tr);
                current_req_log->recieve_time= recieve_time- start_time;
               // current_req_log->number_of_busy_threads=thpool_num_threads_working(thpool);
               current_req_log->number_of_busy_threads=0;
                current_req_log->next = (Server_log_node *)malloc(sizeof(Server_log_node));
                current_req_log=current_req_log->next;
            }
        }
    }
    thpool_wait(thpool);
    printf("\nStopping the server ... \nSaving statistics ...\n");
    print_statistics(results_file,log_list,current_req_log);
    coap_free_context( ctx );
    thpool_destroy(thpool);
    sleep(1);
    return 0;
}
