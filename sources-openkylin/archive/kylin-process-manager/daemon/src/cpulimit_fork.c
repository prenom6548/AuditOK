#include "loghelper.h"
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h> // for compatibility
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define PROC_FILENAME 64

//kernel time resolution (inverse of one jiffy interval) in Hertz
//i don't know how to detect it, then define to the default (not very clean!)
#define HZ 100

//some useful macro
#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//pid of the controlled process
pid_t pid = 0;
//lazy mode
int lazy = TRUE;  //--lazy 模式是 cpulimit 工具的一个选项，用于控制当没有找到目标进程或目标进程已经终止时，cpulimit 的行为。具体来说，当使用 --lazy 选项时，cpulimit 会在找不到目标进程或目标进程终止时自动退出，而不是继续等待或监控系统进程

// number of CPUs we detected
int NCPU;

int send_signal = SIGCONT;
//return ta-tb in microseconds (no overflow checks!)
long timediff(const struct timespec *ta,const struct timespec *tb) {
    unsigned long us = (ta->tv_sec-tb->tv_sec)*1000000 + (ta->tv_nsec/1000 - tb->tv_nsec/1000);
    return us;
}
int Check_Us(pid_t target_pid)
{
   pid_t this_pid;
   this_pid = getpid();
   if (this_pid == target_pid)
   {
      fprintf(stderr, "We cannot throttle ourselves.\n");
      exit(7);
   }
   return TRUE;
}

//作用是等待指定的进程（通过 pid 参数指定）出现，并尝试控制该进程。具体来说，该函数会不断检查 /proc 目录下的进程信息，直到找到指定的 pid 对应的进程。找到后，它会尝试向该进程发送 SIGSTOP 和 SIGCONT 信号，以确保该进程可以被控制。如果成功控制，则函数返回 0；否则，根据情况输出错误信息并返回 -1 或退出程序。
int waitforpid(int pid) {
	int i=0;

	while(1) {
		DIR *dip;
		struct dirent *dit;
		//open a directory stream to /proc directory
		if ((dip = opendir("/proc")) == NULL) {
			perror("opendir");
			return -1;
		}
		//read in from /proc and seek for process dirs
		while ((dit = readdir(dip)) != NULL) {
			//get pid
			if (pid==atoi(dit->d_name)) {
				//pid detected
                Check_Us(pid);
				if (kill(pid,SIGSTOP)==0 &&  kill(pid,SIGCONT)==0) {
					//process is ok!
                    if (closedir(dip) == -1) {
                        perror("closedir");
                        return -1;
                    }
					goto done;
				}
				else {
					fprintf(stderr,"Error: Process %d detected, but you don't have permission to control it\n",pid);
				}
			}
		}
		//close the dir stream and check for errors
		if (closedir(dip) == -1) {
			perror("closedir");
			return -1;
		}
		//no suitable target found
		if (i++==0) {
			if (lazy) {
				fprintf(stderr,"No process found\n");
				exit(2);
			}
			else {
				fprintf(stderr, "Warning: no target process found. Waiting for it...\n");
			}
		}
		//sleep for a while
		sleep(2);
	}
done:
    return 0;

}
//SIGINT and SIGTERM signal handler
void quit(int sig) {
	//let the process continue if we are stopped
	kill(pid, send_signal);
	exit(0);
}

//get jiffies count from /proc filesystem
int getjiffies(int pid) {
    static char stat[256] = {0};
    static char buffer[1024] = {0};
    char *p = NULL;
    size_t buf_size = sizeof(stat);

    int ret = snprintf(stat, buf_size, "/proc/%d/stat", pid);
    if (ret < 0 || (size_t)ret >= buf_size) {
        return -1;
    }

    FILE *f = fopen(stat, "r");
    if (f == NULL) {
        perror("fopen /proc/pid/stat failed");
        return -1;
    }

    p = fgets(buffer, sizeof(buffer), f);
    fclose(f);
    if (p == NULL) {
        fprintf(stderr, "fgets failed for pid %d\n", pid);
        return -1;
    }

    size_t offset = p - buffer;
    char *end_proc = memchr(p + 1, ')', sizeof(buffer) - offset - 1);
    if (end_proc == NULL) {
        fprintf(stderr, "memchr: no ')' found in stat for pid %d\n", pid);
        return -1;
    }
    p = end_proc;

    int sp = 12;
    while (sp--) {
        offset = p - buffer;
        p = memchr(p + 1, ' ', sizeof(buffer) - offset - 1);
        if (p == NULL) {
            fprintf(stderr, "memchr: no enough ' ' found (sp=%d) for pid %d\n", sp, pid);
            return -1;
        }
    }

    offset = p - buffer;
    char *utime_ptr = memchr(p + 1, ' ', sizeof(buffer) - offset - 1);
    if (utime_ptr == NULL) {
        fprintf(stderr, "memchr: no ' ' for utime found for pid %d\n", pid);
        return -1;
    }
    int utime = atoi(p + 1);

    offset = utime_ptr - buffer;
    char *ktime_ptr = memchr(utime_ptr + 1, ' ', sizeof(buffer) - offset - 1);
    if (ktime_ptr == NULL) {
        fprintf(stderr, "memchr: no ' ' for ktime found for pid %d\n", pid);
        return -1;
    }
    int ktime = atoi(utime_ptr + 1);

    return utime + ktime;
}


//process instant photo
struct process_screenshot {
	struct timespec when;	//timestamp
	int jiffies;	//jiffies count of the process
	int cputime;	//microseconds of work from previous screenshot to current
};

//extracted process statistics
struct cpu_usage {
	float pcpu;
	float workingrate;
};
#define MEM_ORDER 10
//circular buffer containing last MEM_ORDER process screenshots
struct process_screenshot ps[MEM_ORDER];
//the last screenshot recorded in the buffer
int front=-1;
//the oldest screenshot recorded in the buffer
int tail=0;

//这段代码是一个用于估算特定进程CPU使用率的动态系统函数。它通过维护一个固定大小的循环缓冲区来记录进程在过去一段时间内的CPU时间快照，并利用这些快照来计算CPU使用率。
int compute_cpu_usage(int pid,int last_working_quantum,struct cpu_usage *pusage) {
	//let's advance front index and save the screenshot
	front=(front+1)%MEM_ORDER;
	int j=getjiffies(pid);
	if (j>=0) ps[front].jiffies=j;
	else return -1;	//error: pid does not exist

        // Linux and BSD can use real time
	clock_gettime(CLOCK_REALTIME,&(ps[front].when));
	ps[front].cputime=last_working_quantum;

	//buffer actual size is: (front-tail+MEM_ORDER)%MEM_ORDER+1
	int size=(front-tail+MEM_ORDER)%MEM_ORDER+1;

	if (size==1) {
		//not enough samples taken (it's the first one!), return -1
		pusage->pcpu=-1;
		pusage->workingrate=1;
		return 0;
	}
	else {
		//now we can calculate cpu usage, interval dt and dtwork are expressed in microseconds
		long dt=timediff(&(ps[front].when),&(ps[tail].when));
		long dtwork=0;
		int i=(tail+1)%MEM_ORDER;
		int max=(front+1)%MEM_ORDER;
		do {
			dtwork+=ps[i].cputime;
			i=(i+1)%MEM_ORDER;
		} while (i!=max);
		int used=ps[front].jiffies-ps[tail].jiffies;
		float usage=(used*1000000.0/HZ)/dtwork;
		pusage->workingrate=1.0*dtwork/dt;
		pusage->pcpu=usage*pusage->workingrate;
		if (size==MEM_ORDER)
			tail=(tail+1)%MEM_ORDER;
		return 0;
	}
	#undef MEM_ORDER
}

// Get the number of CPU cores on this machine.
int get_ncpu()
{
        int ncpu = 1;
        #ifdef _SC_NPROCESSORS_ONLN
        ncpu = sysconf(_SC_NPROCESSORS_ONLN);
        #endif
        return ncpu;
}
int main(int argc, char **argv) {
	int perclimit=0;

    NCPU = get_ncpu();
	if(argc < 3)
	{
		exit(-1);
	}
	//std::string process_name = "/opt/kingsoft/wps-office/office6/wpp";
    //pid_t pid = find_pid_by_name(process_name);
	pid = atoi(argv[1]);
	perclimit = atoi(argv[2]);
    PRINT_NOTICE("cpulimit_fork,mypid = %d,pid = %d,perclimit = %d\n", getpid(), pid, perclimit);
    float limit = perclimit / 100.0;
    if ((limit <= 0.00) || (limit > NCPU)) {
        fprintf(stderr, "Error: limit must be in the range of 1 to %d00\n", NCPU);
        exit(1);
    }

    // parameters are all ok!
    signal(SIGINT, quit); // 必须要存在，不然的话，无法正常恢复被控制进程的运行   ??
    signal(SIGTERM, quit);
    // time quantum in microseconds. it's splitted in a working period and a sleeping one
    int period = 100000;
    struct timespec twork, tsleep; // working and sleeping intervals    ?????
    memset(&twork, 0, sizeof(struct timespec));
    memset(&tsleep, 0, sizeof(struct timespec));

wait_for_process:
    waitforpid(pid); //作用是等待指定的进程（通过 pid 参数指定）出现
	struct timespec startwork,endwork;
	long workingtime=0;		//last working time in microseconds
	//here we should already have high priority, for time precision  //wangshuai 注意 高优先级
 	while(1) {

		//estimate how much the controlled process is using the cpu in its working interval
		struct cpu_usage cu;
		if (compute_cpu_usage(pid,workingtime,&cu)==-1) {
		    if (lazy) 
                exit(2);
		    //wait until our process appears
	             goto wait_for_process;		
		}

		//cpu actual usage of process (range 0-1)
		float pcpu=cu.pcpu;
		//rate at which we are keeping active the process (range 0-1)
		float workingrate=cu.workingrate;

		//adjust work and sleep time slices
		if (pcpu>0) {
			twork.tv_nsec=min(period*limit*1000/pcpu*workingrate,period*1000);
		}
		else if (pcpu==0) {
			twork.tv_nsec=period*1000;
		}
		else if (pcpu==-1) {
			//not yet a valid idea of cpu usage
			pcpu=limit;
			workingrate=limit;
			twork.tv_nsec=min(period*limit*1000,period*1000);
		}
		tsleep.tv_nsec=period*1000-twork.tv_nsec;

		//update average usage
        if (pcpu < limit)
        {
			//resume process
			if (kill(pid,SIGCONT)!=0) {
	            if (lazy) exit(2);
				//wait until our process appears
				goto wait_for_process;
			}
		}
		clock_gettime(CLOCK_REALTIME,&startwork); //用于获取指定时钟的当前时间,CLOCK_REALTIME: 系统实时时间,struct timespec *tp 是一个指向 struct timespec 类型的指针，用于存储获取到的时间值

		nanosleep(&twork,NULL);		//now process is working高精度睡眠的函数。它允许程序以纳秒级的分辨率暂停当前线程的执行，从而实现更精确的定时操作
		clock_gettime(CLOCK_REALTIME,&endwork);
		workingtime=timediff(&endwork,&startwork);

        if (pcpu > limit)
        {
			//stop process, it has worked enough
			if (kill(pid,SIGSTOP)!=0) {
		        if (lazy) exit(2);
				//wait until our process appears
				goto wait_for_process;
			}
			nanosleep(&tsleep,NULL);	//now process is sleeping
		}         
	}
   return 0;
}


