#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h> 
#define nano_secondsecond 1000000000
#define MILLIsecond 1000

typedef struct Block_struct 
{
        float C_time;
	float t_time;
	long int l_burst;
	int pid;
	int priority;
	int status;
	

	
}Block_struct;

typedef struct Clock 
{ 
        int second;
	long int nano_second;
        int pid;
	int quantum;
	int count;
	
	
	
} Clock;

Clock *logclock;
Block_struct *PCB;
int r,seed =24;

int get_PcbByPid(int pid)
{
	int i;

	for(i  = 0; i < 18; i++ )
		if(PCB[i].pid == pid)
			return i;
		
	return -1;
}

int Completed_check(int loc)
{
	if(PCB[loc].C_time > 0.05)
	{
		srand(seed++);
		if(rand()%100 > 30)
			return 1;
		else
			return 0;
	}
}

int Random_genration()
{

	r =rand()%100;
	if(r <= 50)
		return 1;			
	else if(r <= 60 )
		return 0;			
	else if(r <= 75)
		return 2;			
	else
		return 3;			

}


 int main(int argc,char *argv[])
 {
	 
	 
	 int shmid = atoi(argv[1]);
	 int shmid2 = atoi(argv[2]);
	 int i,pid;
	
	 char *c = argv[3];
	 sem_t *sem = sem_open(c, 0);
	 PCB = shmat(shmid, NULL, 0);
	 logclock = shmat(shmid2, NULL, 0);
	 logclock -> count +=1; 
	 pid = getpid(); 
	 srand(pid);
	 int ch;
	 int r=0,s=0,p = -1;
	 float w,t,q;
	 int Startsecond = 0,Endsecond = 0, dsecond = 0;
	 long int Startnano_second = 0, Endnano_second = 0, dnano_second =0;

	 Startsecond = logclock -> second;
	 Startnano_second = logclock -> nano_second;

	while(1)
	{
		if(pid == logclock -> pid)
		{
			w = 0,r=0,t=0.0,s=0, q = 0.0;
			ch = Random_genration();
			switch(ch)
			{
				case 0:
						break;
				case 1:
						w = (float)(logclock -> quantum) / MILLIsecond;
						
						//wait(w);
						break;
				case 2:	
						r = rand()%5;
						s = rand()%1000;
						t = (float)s / 1000;
						w = (float)r + t;
						//wait(w);
						break;
				case 3:

						r = rand()%100;
						t = (float)r / 100;
						q = (float)(logclock -> quantum) / MILLIsecond ;
						w = (float) q  * t;
						//wait(w);
						break;
				default:
						fprintf(stderr,"Error in User \n");
						exit(0);
					
			}
	
			if((p = get_PcbByPid(pid)) >= 0)
			{
			
				PCB[p].l_burst = w * nano_secondsecond;
				PCB[p].C_time += w;
				if(Completed_check(p) == 1)
				{
					
					break;
				}
			}
			else
			{
				fprintf(stderr,"Error in Child : %d -- Exiting\n",getpid());
				exit(0);
			}
			logclock -> pid = -1;
			
			sem_post(sem);
			
		}
	}
			Endsecond = logclock -> second;
			Endnano_second = logclock -> nano_second;
			dsecond = Endsecond - Startsecond;
			dnano_second = Endnano_second - Startnano_second;
			t = (float)dnano_second / nano_secondsecond;
			PCB[get_PcbByPid(pid)].t_time = (float) dsecond + t + w;
			PCB[p].status = 1;
			sem_post(sem);
			
	shmdt(PCB);
	shmdt(logclock);
	 return 0;
 }
