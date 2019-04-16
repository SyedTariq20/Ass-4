#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <signal.h> 
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h> 
#include <fcntl.h>
#include "queue.h"
#define Quantum 10
#define NANOSEC 1000000000
#define MILLISEC 1000000
#define SNAME "/mysem13_543"


typedef struct pcBlock 
{
	float CPU_time;
	float total_time;
	long int last_burst;
	int pid;
	int priority;
	int status;
	
}pcBlock;

typedef struct Clock 
{
	int	sec;
	long int nano;
	int pid;
	int quantum;
	int count;
	
} Clock;


FILE *fptr;
sem_t *sem;
int pcbid,clockid;
pcBlock *PCB;
Clock *logclock;
int lines = 0;

void sigintHandler(int sig_num) 
{ 

    if(sig_num == 2)
    	fprintf(stderr,"Program interrupted\n");
    else
	fprintf(stderr,"Program exceded time limit\n");
    int i;
	fprintf(stderr,"clock status : sec = %d, nano %d\n", logclock -> sec, logclock -> nano);
	fprintf(stderr,"Total Childs Forked : %d\n",logclock -> count);
	
   for(i = 0; i < 18; i++)
   {
       if(PCB[i].pid > 0 )
       {
           //fprintf(stderr,"Killing Child : %d --- %d\n",PCB[i].pid,i);
	       kill(PCB[i].pid, SIGTERM);
       }
    }
	fprintf(stderr,"Removed remaing childs\n");
    
    
  
	shmdt(PCB);
	shmdt(logclock);
	fprintf(stderr,"Shared Memory detached\n");
	fclose(fptr);
    sem_unlink(SNAME);
	fprintf(stderr,"Semaphore unlinked\n");
    shmctl(pcbid,IPC_RMID,NULL);
	shmctl(clockid,IPC_RMID,NULL);
	fprintf(stderr,"Cleared Shared Memory\n");
    abort(); 
    fflush(stdout); 
} 


int create_Child()
{
	
	int i;
	for(i  = 0; i < 18; i++ )
		if(PCB[i].pid == -1)
			return i;
	return -1;
}

int get_PcbByPid(int pid)
{
	int i;
	for(i  = 0; i < 18; i++ )
		if(PCB[i].pid == pid)			
			return i;
	return -1;
}

int get_Priority()
{
	if(rand()%100 > 85 )
		return 1;			//High Priority
	else 
		return 0;		        //Low Priority
		
}

void advance_Clock()
{
	logclock -> sec += 1;
	if(logclock -> nano > NANOSEC)
		{
			logclock -> sec += logclock -> nano/NANOSEC;
			logclock -> nano %= NANOSEC;
		}	
	
}


void clear_PCblock(int loc)
{
		PCB[loc].CPU_time = 0.0;
		PCB[loc].total_time = 0.0;
		PCB[loc].last_burst = 0;
		PCB[loc].pid = -1;
		PCB[loc].priority = -1;
		PCB[loc].status = 0;
}


int main (int argc,char *argv[]) 
{ 
	char arg1[10];
	char arg2[10];
	char arg3[20];
	char *filename = "default";
	int StartSec = 0,EndSec = 0, dSec = 0;
	long int StartNano = 0, EndNano = 0, dNano =0,diff =0;

	int c, i,status,pid,t=2;
	int TimeToGenrate = 0;
	struct Queue hq;
	struct Queue lq;
	signal(SIGALRM, sigintHandler);
    signal(SIGINT, sigintHandler);
	
	
	while ((c = getopt (argc, argv, "hl:t:")) != -1)
	switch (c)
    {
		case 'h':
			printf("\nOPTIONS :\n");
            printf("-h for HELP \n");
			printf("-l filename  (File where output to be stored)\n");
			printf("-t z (where z is maximum no.of sec program allowed to run\n");
			return 1;
		case 'l':
			filename = optarg;
			break;
		case 't':
			t = atoi(optarg);
			break;	
		case '?':
			if (optopt == 'z' || optopt == 'l' || optopt == 's')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
			return 1;
		default:
        abort ();
      }

	fptr = fopen(filename, "w");
    if(fptr == NULL)
    {
      fprintf(stderr,"File Open ERROR \n");
      exit(1);
    }
	fprintf(stderr,"Log file : %s\n",filename);
	key_t key = ftok(".",'m');
	key_t key2 = ftok(".",'s');
	int sizepcb = 18 * sizeof(pcBlock);
	if((sem = sem_open(SNAME, O_CREAT, 0666, 0)) == SEM_FAILED )
		fprintf(stderr,"Sem open error\n");
	
	pcbid = shmget(key,sizepcb,0666|IPC_CREAT);
	if ( pcbid == -1 )
		fprintf(stderr,"OSS : Error in shmget1");
	
	PCB = (pcBlock *)shmat(pcbid, NULL, 0);
	
	clockid = shmget(key2,sizeof(logclock),0666|IPC_CREAT);
	if ( clockid == -1 )
		fprintf(stderr,"OSS : Error in shmget2");
	
	logclock = (Clock *)shmat(clockid, NULL, 0);
	
	logclock -> sec = 0;
	logclock -> nano = 0;
	logclock -> pid = -1;
	logclock -> quantum = 0;
	logclock -> count  = 0;
	
	
	
	
	for(i  = 0; i < 18; i++ )
	{
		PCB[i].CPU_time = 0.0;
		PCB[i].total_time = 0.0;
		PCB[i].last_burst = 0;
		PCB[i].pid = -1;
		PCB[i].priority = -1;
		PCB[i].status = 0;
	}
	
	//PCB[0].CPU_time = -90;
	
	
	
	
	snprintf(arg1,10,"%d", pcbid);
	snprintf(arg2,10,"%d",clockid);
	snprintf(arg3,20,"%s", SNAME);
	
	alarm(t);
	       
    
 	int loc = -1,p;
	srand(time(NULL));
	init(&hq);
	init(&lq);
	 
	while(1){
		
		if( (loc = create_Child()) >= 0  && logclock -> sec  >= TimeToGenrate)
		{
			
			if( (pid = fork()) == 0)
			{
				
				execlp("./user","./user",arg1,arg2,arg3,(char *)NULL);
				fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
				exit(0);
			}
			
			TimeToGenrate += rand()%3; 
			PCB[loc].priority = get_Priority();  
			PCB[loc].pid = pid;
			if(PCB[loc].priority == 0)
			{
				if(lines < 10000)
					fprintf(fptr,"OSS : Generating process with PID %d and putting it in queue 1 at time %d:%ld\n",pid,logclock -> sec,logclock -> nano,lines++);
				push(&lq, pid);
			}
			else
			{
				if(lines < 10000)
					fprintf(fptr,"OSS : Generating process with PID %d and putting it in queue 0 at time %d:%ld\n",pid,logclock -> sec,logclock -> nano,lines++);
				push(&hq,pid);
			}
			loc = -1;	 
		}
		
	
		logclock -> nano += rand()%1000;	
		if(logclock -> nano > NANOSEC)
		{
			logclock -> sec += logclock -> nano/NANOSEC;
			logclock -> nano %= NANOSEC;
		}	
		
		if((p = pop(&hq)) > 0)
		{
			if(lines < 10000)
		        fprintf(fptr,"OSS :  Dispatching process with PID : %d from queue 1 at time %d:%ld\n",p,logclock -> sec,logclock -> nano,lines++);
			logclock -> pid = p;
			logclock -> quantum = Quantum/2;
			EndSec = logclock -> sec;
			EndNano = logclock -> nano;
			dSec = EndSec - StartSec;
			dNano = EndNano - StartNano;
			diff = dSec * NANOSEC + dNano;
			if(lines < 10000)
				fprintf(fptr,"OSS : Total time this dispatch was %ld\n",diff,lines++);
			sem_wait(sem);
			loc = get_PcbByPid(p);
			if(PCB[loc].status == 1)
			{
				if(lines < 10000)
					fprintf(fptr,"OSS : Reciving pid : %d executed %ld\n",p,PCB[loc].last_burst,lines++);
				waitpid(p, &status,0);
				if(lines < 10000)
					fprintf(fptr,"OSS : pid : %d completed execution CPU : %f  Total : %f\n",p,PCB[loc].CPU_time,PCB[loc].total_time,lines++);
				clear_PCblock(loc);
			}
			else
			{
			if(lines < 10000)		
				fprintf(fptr,"OSS : Reciving pid : %d executed %ld\n",p,PCB[loc].last_burst,lines++);
			if(PCB[loc].last_burst <= 5000000)
				if(lines < 10000)
					fprintf(fptr,"OSS : not used it entire quantum \n",lines++);
			if(lines < 10000)
				fprintf(fptr,"OSS : Putting process with PID %d into queue 0\n",p,lines++);
			push(&hq,p);
			}
			logclock -> nano +=  PCB[loc].last_burst;
			if(logclock -> nano > NANOSEC)
			{
				logclock -> sec += logclock -> nano/NANOSEC;
				logclock -> nano %= NANOSEC;
			}
			
			StartSec = logclock -> sec;
			StartNano = logclock -> nano;
			loc = -1;
			
		} 
		else if((p = pop(&lq)) > 0)
		{
			if(lines < 10000)
				fprintf(fptr,"OSS : Dispaching pid : %d from queue 1 at time %d:%ld\n",p,logclock -> sec,logclock -> nano,lines++);
			logclock -> pid = p;
			logclock -> quantum = Quantum;
			EndSec = logclock -> sec;
			EndNano = logclock -> nano;
			dSec = EndSec - StartSec;
			dNano = EndNano - StartNano;
			diff = dSec * NANOSEC + dNano;
			if(lines < 10000)
				fprintf(fptr,"OSS : Total time this dispatch was %ld\n",diff,lines++);
			sem_wait(sem);
			loc = get_PcbByPid(p);
			if(PCB[loc].status == 1)
			{
				if(lines < 10000)
					fprintf(fptr,"OSS : Reciving pid : %d executed %ld\n",p,PCB[loc].last_burst,lines++);
				waitpid(p, &status,0);
				if(lines < 10000)
					fprintf(fptr,"OSS : pid : %d completed execution CPU : %f  Total : %f\n",p,PCB[loc].CPU_time,PCB[loc].total_time,lines++);
				clear_PCblock(loc);
			}
			else
			{
			if(lines < 10000)
				fprintf(fptr,"OSS : Reciving pid : %d executed %ld\n",p,PCB[loc].last_burst,lines++);
			if(PCB[loc].last_burst <= 10000000)
				if(lines < 10000)
					fprintf(fptr,"OSS : not used it entire quantum \n",lines++);
			if(lines < 10000)	
				fprintf(fptr,"OSS : Putting process with PID %d into queue 1\n",p,lines++);
			push(&lq,p);
			}
			logclock -> nano +=  PCB[loc].last_burst;
			if(logclock -> nano > NANOSEC)
			{
				logclock -> sec += logclock -> nano/NANOSEC;
				logclock -> nano %= NANOSEC;
			}	
			StartSec = logclock -> sec;
			StartNano = logclock -> nano;
			loc = -1;
		}
		else
			advance_Clock();
		
	
	}
 
	shmdt(PCB);
	shmdt(logclock);
	shmctl(pcbid,IPC_RMID,NULL);
	shmctl(clockid,IPC_RMID,NULL);
	return 0;
}
	  

