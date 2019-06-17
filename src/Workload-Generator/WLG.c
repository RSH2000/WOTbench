#define MAX_FILE_NAME_LENGTH 1000
#define PACKAGE_VERSION "1.0"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <unistd.h>


static const char URI_TEMPLATE[] = "coap://[RSHIPRSH";
static const char RES_TEMPLATE[] = ",NA,NA,FALSE";
typedef struct WLGen_node{
    struct WLGen_node * next;
    char * name;
    long value;
    long ait;
    int code;
}WLGen_node;
typedef struct Test_spec{
double mean_iat;
double service_time;
double duration;
int Num_client;
char distribution;
}Test_spec;
double exp_rand(double mean){
double rand_num;
    rand_num=(double)(rand()) / (double)(RAND_MAX);
      //    printf("\nuniform: %f %d ",z,RAND_MAX);
      //    printf("\nexpo: %f", -res_value * log(z));
     rand_num = -mean * log(rand_num);
     return(rand_num);
}
double uniform_rand(double mean){
double rand_num;
    rand_num=(double)(rand()) / (double)(RAND_MAX);
     rand_num = mean * 2 * rand_num;
     return(rand_num);
}

void
usage( const char *program, const char *version) {
  const char *p;

  p = strrchr( program, '/' );
  if ( p )
    program = ++p;

  fprintf( stderr, "%s v%s -- CoAPBench Workload Generator\n"
       "(c) 2016 Raoufeh Hashemian <rhashem@ucalgary.ca>\n\n"
       "usage: %s [-O output workload file] [-R input resource file]\n\n"
       "\t-S list of servers file\n"
       "\t-t request inter arrival time\n"
       "\t-v num\t\tverbosity level (default: 3)\n",
       program, version, program );
}
//****** get parameters from line
void split_line(WLGen_node * tmp,char line[])
{
    char * pch=NULL;
    char tsstr[10];
    int a=-2;
    pch=strchr(line,',');
    if(pch)
    {
        a=strlen(pch+1);
        tmp->name=(char *)malloc((a+1)*sizeof(char));
        strncpy(tmp->name,pch+1,a-1);
        strncpy (tsstr,line,pch-line);
        tmp->value=atof(tsstr);
        return;
    }
    else{
        a=strlen(line);
        tmp->name=(char *)malloc((a+1)*sizeof(char));
        strncpy(tmp->name,line,a-1);
        tmp->value=-1;
    }
    return;
}

//***** Read file *****
WLGen_node *
read_Srvfile(int * count,char *file_name) {
  char * line= malloc(1000*sizeof(char));

  WLGen_node *tmp=NULL;
  WLGen_node *prev=NULL;
  WLGen_node *Llist=NULL;
  FILE* F=fopen(file_name,"r+");
  if(!F){printf("Failed to open input file: %s!\n",file_name);exit(1);}
  while(fgets(line,1000,F)){
        printf("%s\n",line);
       if(strlen(line)>4){
        tmp= malloc(sizeof(WLGen_node));
        if(!tmp){
            printf("Failed to allocate memory for resource properties\n");
            exit(1);}
        split_line(tmp,line);
        (*count)++;
       if(!Llist)
            Llist=tmp;
            else
                prev->next=tmp;
            prev=tmp;
    }
  }
  printf("The workload is being generated for %d Servers\n",*count);
  return(Llist);
}
// Writing the seq th resource file
void
write_Resfile(WLGen_node * ResHead,char * file_name,int seq){
    char fname[1000];
    WLGen_node * tmp=ResHead;
    WLGen_node * tmpdel=NULL;
    FILE* F;
    sprintf(fname,"%s%d",file_name,seq);
    F=fopen(fname,"w+");
  if(!F){printf("Failed to open resource file: %s%d!\n",file_name,seq);exit(1);}
  while(tmp){
    fprintf(F,"%s\n",tmp->name);
//    printf("%s\n",tmp->name);
    tmpdel=tmp;
    tmp=tmp->next;
    free(tmpdel->name);
    free(tmpdel);
}
fclose(F);
}
//********Check if the resource exists *****
int new_Res(WLGen_node* Res,int Servicetime){
int Res_is_new=1;
WLGen_node* tmp=Res;
while (tmp){
    //    printf("%f\n",tmp->value);
    if((int)(tmp->value)==Servicetime){
        Res_is_new=0;
   //     printf("repeated\n");
    break;}
    tmp=tmp->next;
}
return(Res_is_new);
}
//***** Create workload ******
 WLGen_node *
 creat_workload(char * Resource_File,WLGen_node * Srv,Test_spec  test,int seq){
 WLGen_node * Res=NULL;
 WLGen_node * tmpRes=NULL;
 WLGen_node * prevRes=NULL;

 WLGen_node * wl=NULL;
 WLGen_node * tmpwl=NULL;
 WLGen_node * prevwl=NULL;
 char uri[1000];
 char resline[1000];
 int Servicetime=0;  //service time in usec
 long TimeStamp=0; // time stamp in usec
 long LastTimeStamp=0;
 int inter_arrival_time = 0;
while(TimeStamp < test.duration*1000000){
    Servicetime= exp_rand(test.service_time);
    switch (test.distribution) {
case 'e':
    Servicetime= exp_rand(test.service_time);
    inter_arrival_time = (int)exp_rand(test.mean_iat);
    break;
case'u':
    inter_arrival_time = (int)uniform_rand(test.mean_iat);
    break;
default:
       Servicetime = test.service_time;
       inter_arrival_time = test.mean_iat;
}

    if (Servicetime<50)
        Servicetime=50;

    TimeStamp = TimeStamp + inter_arrival_time;
    tmpwl=(WLGen_node*)malloc(sizeof(WLGen_node));
    if(!tmpwl){printf("Unable to allocate memory\n"); exit(1);}
    tmpwl->value=TimeStamp;
    tmpwl->code=seq;

    sprintf(uri,"coap://%s/res%d",Srv->name,Servicetime);
  // printf("Next Entry is: %ld,coap://%s/res%d\n",TimeStamp,Srv->name,Servicetime);
    //adding the workload entry
    tmpwl->name=(char *)malloc((strlen(uri)+1)*sizeof(char));
    tmpwl->next=NULL;
    tmpwl->ait= TimeStamp - LastTimeStamp;
    strcpy(tmpwl->name,uri);
    if(!wl){
        wl=tmpwl;
        prevwl=tmpwl;
    }
    else{
         prevwl->next=tmpwl;
         prevwl=tmpwl;
    }
    // adding the resource entry
    if(new_Res(Res,Servicetime)){
    sprintf(resline,"res%d,%d us,%d%s",Servicetime,Servicetime,Servicetime,RES_TEMPLATE);
    tmpRes=(WLGen_node*)malloc(sizeof(WLGen_node));
    tmpRes->value=Servicetime;
    tmpRes->name=(char *)malloc((strlen(resline)+1)*sizeof(char));
    tmpRes->next=NULL;
    strcpy(tmpRes->name,resline);
   // printf("%s\n",tmpRes->name);
    if(!Res){
        Res=tmpRes;
        prevRes=tmpRes;
    //            printf("first res node\n");

    }
    else{
         prevRes->next=tmpRes;
         prevRes=tmpRes;
   //              printf("next res node%f\n",prevRes->value);

    }
    }
    LastTimeStamp = TimeStamp;
  //  printf("%ld\n",TimeStamp);
}//duration is in seconds and the inter-arrival time is in usec
 write_Resfile(Res,Resource_File,seq);
 return(wl);
 }


void
write_workload(WLGen_node * wlHead,char * file_name,int Num_client){
    WLGen_node * tmp=NULL;
    FILE** F;
    FILE* M;
    int i=0;
    char file_index[4];
    char name[MAX_FILE_NAME_LENGTH+4];
    F=(FILE**)malloc(sizeof(FILE*)*Num_client);
    for(i=0;i<Num_client;i++){
            sprintf(file_index, "%d", i);
            strcpy(name,file_name);
            strcat(name,file_index);
            printf("Saving Workload file %d: %s\n",i,name);
            F[i]=fopen(name,"w+");
            if(!F[i])
                {printf("Failed to open workload file: %s!\n",name);exit(1);}
     }
     M=fopen(file_name,"w+");
            if(!M)
                {printf("Failed to open workload file: %s!\n",file_name);exit(1);}
    // printf("writing req\n");
     while(wlHead){
        for(i=0;i<Num_client;i++){
            fprintf(F[i],"%d,%ld,%ld,%s\n",wlHead->code,wlHead->value,wlHead->ait,wlHead->name);
            fprintf(M,"%d,%ld,%ld,%s\n",wlHead->code,wlHead->value,wlHead->ait,wlHead->name);
          //printf("%d,%ld,%ld,%s\n",wlHead->code,wlHead->value,wlHead->ait,wlHead->name);
            tmp=wlHead;
            wlHead=wlHead->next;
            free(tmp->name);
            free(tmp);
            if(!wlHead)
                break;
        }
    }
    for(i=0;i< Num_client;i++)
        fclose(F[i]);
    fclose(M);
    return;

}
//Sorts the list of workloads based on timestamp
WLGen_node *
sort_workload(WLGen_node * head){

    WLGen_node  *tmpPtr = head;
    WLGen_node  *tmpNxt = head->next;
    WLGen_node  *testptr = head;

    double  tmpd;
    int tmpi;
    char * tmps;
    int i=0;
    while(tmpNxt != NULL){
           while(tmpNxt != tmpPtr){
                    if(tmpNxt->value < tmpPtr->value){
                            tmpd = tmpPtr->value;
                            tmpPtr->value = tmpNxt->value;
                            tmpNxt->value = tmpd;
                            tmps = tmpPtr->name;
                            tmpPtr->name = tmpNxt->name;
                            tmpNxt->name = tmps;
                            tmpi = tmpPtr->code;
                            tmpPtr->code = tmpNxt->code;
                            tmpNxt->code = tmpi;
                    }
                    tmpPtr = tmpPtr->next;
            }
            tmpPtr = head;
            tmpNxt = tmpNxt->next;
            printf("%d\n",i++);
    }
    testptr=tmpPtr;
         return tmpPtr; // Place holder
}
// Combines two workloads
WLGen_node *
combine_workloads(WLGen_node **WLHeads,int SrvCount){
    WLGen_node  *tmp=NULL;
    int i=0;
    for(i=0;i<SrvCount;i++){
           // printf("server%d\n",i);

     if(!tmp)
            tmp=WLHeads[0];
    else{
        tmp->next=WLHeads[i];
        tmp=WLHeads[i];
    }
        while(tmp->next){
    //        printf("%s\n",tmp->name);
            tmp=tmp->next;
        }
   //  printf("end of combine for server %d\n",i);

    }
 //   printf("combined wl\n");
 if(SrvCount>1)
sort_workload(WLHeads[0]);
return(WLHeads[0]);
}
// finds minimum of array
int
find_min(WLGen_node ** srvnods,int SrvCount)
{
    int index=0;
    long min_timestamp = -1;
    for (int i=0;i<SrvCount;i++){
    //printf("\n %d",i);
    if(srvnods[i]){
        if ((srvnods[i]->value<min_timestamp) || (min_timestamp<0)){
            index=i;
        //  printf("\n %d  %d",index,srvnods[i]->value);
            min_timestamp = srvnods[i]->value;
        }
    }
   // else
      //  printf("\nend of list %d\n",i);
}
        return(index);

}
// Combines two workloads
WLGen_node *
combine_sort(WLGen_node **WLHeads,int SrvCount){
    WLGen_node  * tmp[SrvCount];
    WLGen_node *head=NULL;
    WLGen_node *last=NULL;
    int i=0;
    int nextindex=0;
   // long lastreq[SrvCount];
    int alldone=0;
    for(i=0;i<SrvCount;i++)
            tmp[i]=WLHeads[i];
   // printf("tmps were initialized\n");
           while(!alldone){
    nextindex=find_min(tmp,SrvCount);
  //   printf("\n %d  %d",nextindex,tmp[nextindex]->value);
    if(!head){
        head=tmp[nextindex];
        last=head;
        }
        else{
            last->next=tmp[nextindex];
            last=tmp[nextindex];
            }
        tmp[nextindex]=tmp[nextindex]->next;
      for(i=0;i<SrvCount;i++){
        if (tmp[i] != NULL){
            alldone=0;
            break;
        }
     //   printf("list %d is all done",i);
        alldone=1;
      }

    }
return(head);
}


int
main(int argc, char **argv) {
      int opt;
    struct timeval tv, *timeout;
    int log_level = 3;
    char Resource_File[MAX_FILE_NAME_LENGTH]="Resources";
    char Server_File[MAX_FILE_NAME_LENGTH]="Servers";
    char Workload_File[MAX_FILE_NAME_LENGTH]="Workload";
    WLGen_node * ResHead=NULL;
    WLGen_node * SrvHead=NULL;
    WLGen_node * Workload=NULL;
    WLGen_node * Srv;
    int ResCount=0;
    int SrvCount=0;
    Test_spec test;
    int i=0;
    WLGen_node ** WLHeads;
    test.Num_client = 1;
    printf("Welcome to CoAPBench Workload Generator");
  while ((opt = getopt(argc, argv, "c:d:b:h:O:R:S:s:t:v:")) != -1) {
    switch (opt) {
      case 'c':
      test.Num_client=atoi(optarg);
      break;
      case 'b':
      test.distribution=optarg[0];
      break;
      case 'd':
      test.duration=atoi(optarg);
      break;
      case 'h':
      usage( argv[0], PACKAGE_VERSION );
      exit( 1 );
      break;
    case 'O' :
      strcpy(Workload_File,optarg);
      break;
    case 'R' :
      strcpy(Resource_File,optarg);
      break;
    case 'S' :
      strcpy(Server_File,optarg);
      break;
    case 's' :
      test.service_time=atoi(optarg);
      break;
    case 't' :
      test.mean_iat=atoi(optarg);
      break;
    case 'v' :
      log_level = strtol(optarg, NULL, 10);
      break;
    default:
      usage( argv[0], PACKAGE_VERSION );
      exit( 1 );
    }
  }
 printf("Input was saved\n");
srand (time(NULL));
SrvHead=read_Srvfile(&SrvCount,Server_File);
if(!SrvCount){printf("Not server specified in the file\n");exit(1);}
WLHeads=(WLGen_node **)malloc(SrvCount*sizeof(WLGen_node *));
Srv=SrvHead;
for (i=0;i<SrvCount;i++){
   usleep(100000);
  // printf("Generating workload for server number %d with IP:%s\n",i,Srv->name);
   WLHeads[i]=creat_workload(Resource_File,Srv,test,i);
   Srv=Srv->next;
}
 printf("Workloads were generated\n");

//Resources=combine_resources(ResHeads);
Workload=combine_sort(WLHeads,SrvCount);
write_workload(Workload,Workload_File,test.Num_client);
return(0);
}
