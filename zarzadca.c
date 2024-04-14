#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

int queueID;
int shmID;
int* mem;

void CreateMsg(key_t key);
void CreateShm(key_t key);
void MsgError(int test);
void Close();

typedef struct com{
	long mtype;
	int mvalue;
} com;

int main()
{
    key_t key1 = ftok(".", 'a');
    key_t key2 = ftok(".", 'b');

    signal(SIGINT, Close);
    signal(SIGSEGV, Close);
    CreateMsg(key1);
    CreateShm(key2);
    com message;
    

    for(int i=1;i<=5;i++)
    {
        message.mtype = i;
        int test = 0;
        test = msgsnd(queueID, &message, sizeof(message), 0);
        MsgError(test);
    }

    for(int i=1;i<=5;i++)
    {
        char filozofID[20];
        sprintf(filozofID, "%d", i);
        switch(fork())
        {
            case 0:
                if(execl("./filozof", "filozof", filozofID, NULL) == -1)
                {
                    perror("Blad przy execl.\n");
                    exit(1);
                }
                break;
            case -1:
                perror("Blad przy forku.\n");
                exit(1);
                break;
            default:
                break;
        }
    }

    while(1)
    {

    }
}

void CreateMsg(key_t key)
{
    queueID=msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if(queueID == -1)
    {
        perror("Blad przy tworzeniu kolejki.\n");
        Close();
    }
    else
    {
        printf("Message Queue - %d\n", queueID);
    }
}

void CreateShm(key_t key)
{
    shmID = shmget(key, 5*sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    if(shmID == -1)
    {
        perror("Blad przy tworzeniu pamieci zarzadcy.");
        exit(1);
    }
    else
    {
        printf("Shared Memory - %d\n", shmID);
    }
    
    mem = (int*)shmat(shmID, NULL, 0);
    if(!mem)
    {
        perror("Blad przy dolaczaniu zarzadcy.");
        exit(1);
    }

    for(int i=0;i<5;i++)
    {
        mem[i] = -1;
    }
}

void MsgError(int test)
{
    if(test == -1)
    {
        if(errno == EIDRM || errno == EINVAL)
        {
            perror("Blad, brak kolejki.\n");
            exit(1);
        }
        perror("Blad przy pobieraniu komunikatu.\n");
        exit(1);
    }
}

void Close()
{
    printf("\nZamknieto serwer\n");
    int koniec1 = msgctl(queueID, IPC_RMID, 0);
    int koniec2 = shmctl(shmID, IPC_RMID, 0);
    if(koniec1 == -1)
    {
        perror("Blad przy usuwaniu kolejki.");
        exit(1);
    }
    else if (koniec2 == -1)
    {
        perror("Blad przy usuwaniu pamieci.");
        exit(1);
    }
    else
    {
        printf("Usunieto pamiec i kolejke.\n");  
        exit(0);
    }

}
