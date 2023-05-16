#include "ForkTest.h"

void ParentChildProcessTest()
{
    printf("Init in Parent Process...\n");

    string RetStr = "";
    pid_t ChildPID = -1;

    for (int i = 0; i < 2; i++)
    {
        ChildPID = fork();

        if (ChildPID == 0)
        {
            RetStr = "Child";
        }
        else if (ChildPID > 0)
        {
            RetStr = "Parent";
        }
        else
        {
            printf("Failed to create child process!!!\n");
            return;
        }

        printf("[%d] %s Do Task , PPID:%d, PID:%d, CPID:%d\n",
               i, RetStr.c_str(), getppid(), getpid(), ChildPID);
    }

    printf("Done in Parent Process, PPID:%d, PID:%d\n", getppid(), getpid());
}

void sig_child(int signo)
{
    printf("child process:%d terminal or stop! PID:%d\n", signo, getpid());
}

int SharedMem_Static_IntValue = 99;
void SharedMemTest()
{
    if (signal(SIGCHLD, sig_child) == SIG_ERR)
    {
        printf("signal error");
        return;
    }

    int SharedMem_Stack_IntValue = 1;

    pid_t PID = fork();
    if (PID == 0)
    {
        printf("[Child]  Before  PID:%d StaticPtr:%p StackPtr:%p\n", getpid(),
               &SharedMem_Static_IntValue, &SharedMem_Stack_IntValue);

        SharedMem_Static_IntValue++;
        SharedMem_Stack_IntValue++;

        printf("[Child]  After   PID:%d StaticPtr:%p StackPtr:%p\n", getpid(),
               &SharedMem_Static_IntValue, &SharedMem_Stack_IntValue);
    }
    else if (PID > 0)
    {
        printf("[Parent] Before  PID:%d StaticPtr:%p StackPtr:%p\n", getpid(),
               &SharedMem_Static_IntValue, &SharedMem_Stack_IntValue);

        SharedMem_Static_IntValue--;
        SharedMem_Stack_IntValue--;

        printf("[Parent] After   PID:%d StaticPtr:%p StackPtr:%p\n", getpid(),
               &SharedMem_Static_IntValue, &SharedMem_Stack_IntValue);
        sleep(2);
    }
    else
    {
        return;
    }

    printf("PID:%d Static_Value:%d Stack_Value:%d\n", getpid(), SharedMem_Static_IntValue, SharedMem_Stack_IntValue);
}


#define SIGPARENTCUSTOMSIGNAL SIGRTMIN+1
void Sig_ParentCustomSignal(int Signal, siginfo_t* Info, void* Context)
{
    if (Signal == SIGPARENTCUSTOMSIGNAL && Info)
    {
        printf("\nReceive signal:%d from PID:%d to PID:%d with info:%d\n\n",
               SIGPARENTCUSTOMSIGNAL, Info->si_pid, getpid(), Info->si_value.sival_int);
    }
}

void SignalQueueTest()
{
    pid_t PID = fork();
    if (PID == 0)           // child
    {
        printf("[Child] PID:%d Wait for parent's signal\n", getpid());

        {// Set to receive signal from parent
            struct sigaction Act;
            sigfillset(&Act.sa_mask);
            Act.sa_flags = SA_SIGINFO | SA_RESTART | SA_ONSTACK;
            Act.sa_sigaction = Sig_ParentCustomSignal;

            sigaction(SIGPARENTCUSTOMSIGNAL, &Act, nullptr);
        }

        // Wait for parent's signal
        sleep(2);

        {// Clear signal callback
            struct sigaction Act;
            sigemptyset(&Act.sa_mask);
            Act.sa_flags = 0;
            Act.sa_sigaction = nullptr;

            sigaction(SIGPARENTCUSTOMSIGNAL, &Act, nullptr);
        }

        printf("[Child] PID:%d Stop waiting for parent's signal\n", getpid());
    }
    else if (PID > 0)       // parent
    {
        // Wait for child executing
        sleep(1);

        union sigval sigInfo;
        sigInfo.sival_int = 520;

        printf("[Parent] PID:%d Sending signal to child\n", getpid());

        // Send signal to child
        sigqueue(PID, SIGPARENTCUSTOMSIGNAL, sigInfo);
    }
    else
    {
        printf("!!!Error Failed to fork a child process!\n");
        return;
    }
}

void GameServerSignalTest()
{
#define WAIT_AND_FORK_QUEUE_SIGNAL SIGRTMIN+1

    union sigval sigInfo;
    sigInfo.sival_int = 256;// 0000-0100

    do {
        pid_t GameServerPID = -1;
        printf("Input GameServer Parent PID: ");
        scanf("%d", &GameServerPID);

        while(getchar() != 'q')
        {
            sigqueue(GameServerPID, WAIT_AND_FORK_QUEUE_SIGNAL, sigInfo);
            printf("sigqueue to %d with signal:%d\n", GameServerPID, sigInfo.sival_int);

            sigInfo.sival_int++;
        }
    } while(getchar() != 'q');
}
