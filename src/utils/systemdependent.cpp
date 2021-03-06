// Copyright (c) 2014-2017 Coin Sciences Ltd
// MultiChain code distributed under the GPLv3 license, see COPYING file.

#include "multichain/multichain.h"

#ifndef WIN32

#ifndef _AIX
#include <pthread.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <semaphore.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <syslog.h>

#include <glob.h>
#include <pwd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
//#include "us_define.h"
#include <string.h>
#include <sys/stat.h>


#include <dlfcn.h>  // We need if of dlopen
#include <unistd.h> // We need it for sleep

#include <pwd.h>

#include "utils/define.h"
#include "utils/declare.h"

#define SHM_FAILED        (void *)-1L
#define WAIT_FAILED ((cs_int32)0xFFFFFFFF)
#define WAIT_OBJECT_0       0


void __US_Dummy()
{
    
}

void __US_Sleep (int dwMilliseconds)
{
    timespec ts;

    ts.tv_sec = (time_t)(dwMilliseconds/1000);
    ts.tv_nsec = (int) ((dwMilliseconds % 1000) * 1000000);

    nanosleep(&ts, NULL);
}

int __US_Fork(char *lpCommandLine,int WinState,int *lpChildPID)
{
    int res;
    int statloc;

    statloc=0;
    res=fork();
    if(res==0)
    {
        res=fork();
        if(res)
        {
            *lpChildPID=res;
            exit(0); //Child exits
        }
    }
    else
    {
        if(res>0)
        {
            while(*lpChildPID==0)
            {
                 __US_Sleep(1);
            }
            waitpid(res, &statloc, 0);
            res=*lpChildPID;
        }
    }

    return res;//Parent exits with PID of grandchild , GrandChild exits with 0
}
    
int __US_BecomeDaemon()
{
    int ChildPID;
    int res;
    int i,fd;

    res=__US_Fork(NULL,0,&ChildPID);

    if(res)
         exit(0);

    for (i = getdtablesize()-1; i > 2; --i)
          close(i);

    fd = open("/dev/tty", O_RDWR);
    ioctl(fd, TIOCNOTTY, 0);
    close (fd);

    return res;
}

void* __US_SemCreate()
{
    sem_t *lpsem;
    
    lpsem=NULL;
    
    lpsem=new sem_t;
    if(sem_init(lpsem,0666,1))
    {
        delete lpsem;
        return NULL;
    }

    return (void*)lpsem;
}

void __US_SemWait(void* sem)
{
    if(sem)
    {
        sem_wait((sem_t *)sem);
    }
}

void __US_SemPost(void* sem)
{
    if(sem)
    {
        sem_post((sem_t*)sem);
    }
}

void __US_SemDestroy(void* sem)
{
    sem_t *lpsem;
    if(sem)
    {
        sem_close((sem_t*)sem);
        lpsem=(sem_t*)sem;
        delete lpsem;
    }
}

uint64_t __US_ThreadID()
{
    return (uint64_t)pthread_self();
}

const char* __US_UserHomeDir()
{
    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL) 
    {
        homedir = getpwuid(getuid())->pw_dir;
    }    
    
    return homedir;
}

#else

#include "windows.h"

void __US_Dummy()
{
    
}

int __US_Fork(char *lpCommandLine,int WinState,int *lpChildPID)
{
    return 0;
}

int __US_BecomeDaemon()
{
    return 0;
}

void __US_Sleep (int dwMilliseconds)
{
    Sleep(dwMilliseconds);
}


void* __US_SemCreate()
{
    return (void*)CreateSemaphore(NULL,1,1,NULL);
}

void __US_SemWait(void* sem)
{
    if(sem)
    {
	WaitForSingleObject(sem, 0x7FFFFFFF);
    }
}

void __US_SemPost(void* sem)
{
    if(sem)
    {
        ReleaseSemaphore(sem,1,NULL);
    }
}

void __US_SemDestroy(void* sem)
{
    if(sem)
    {
	CloseHandle(sem);
    }
}

uint64_t __US_ThreadID()
{
    return (uint64_t)GetCurrentThreadId ();
}

const char* __US_UserHomeDir()
{
    return NULL;
}

#endif
