#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

typedef struct com{
	long mtype;
	int  mvalue;
} com;

int filozofID;
int queueID;
int shmID;
int* mem;

void CreateMsg(key_t key);
void CreateShm(key_t key);
void MsgError(int nr);
void PrintForks();
 
int main(int argc, char **argv)
{
    key_t key1 = ftok(".", 'a');
    key_t key2 = ftok(".", 'b');
    CreateMsg(key1);
    CreateShm(key2);


    filozofID = atoi(argv[1]);
    int left, right;
    left = filozofID;
    if(filozofID == 1)
    {
        right = 5;
    }
    else
    {
        right = filozofID-1;
    }
    while(1)
    {
        printf("Filozof %d mysli.\n", filozofID);
        sleep(getpid()%2+2);


        com receive;
        int test = msgrcv(queueID, &receive, sizeof(receive), left, 0); //CZEKAMY NA LEWY
        MsgError(test);
        test = msgrcv(queueID, &receive, sizeof(receive), right, 0); //CZEKAMY NA PRAWY
        MsgError(test);

	    mem[left-1] = filozofID;
	    mem[right-1] = filozofID;
	    
        printf("Filozof %d je. Zajete widelce: %d, %d.\n", filozofID, left, right);
        PrintForks();
        sleep(getpid()%4+2);

        mem[left-1] = -1;
	    mem[right-1] = -1;

        com send;
        send.mtype = left;
        test = msgsnd(queueID, &send, sizeof(send.mvalue), 0);
        MsgError(test);
        send.mtype = right;
        test = msgsnd(queueID, &send, sizeof(send.mvalue), 0);
        MsgError(test);
        printf("Filozof %d zjadl.\n", filozofID);
        PrintForks();
    }

    exit(0);
}

void CreateMsg(key_t key)
{
    queueID=msgget(key, 0600);
    if(queueID == -1)
    {
        if(errno == ENOENT)
        {
            perror("Blad, serwer nie wlaczony\n");
            exit(1);
        }
        perror("Blad przy dolaczaniu do kolejki\n");
        exit(1);
    }
}

void CreateShm(key_t key)
{
    shmID = shmget(key, 5*sizeof(int), IPC_CREAT | 0600);
    if(shmID == -1)
    {
        perror("Blad przy tworzeniu pamieci filozofa");
        exit(1);
    }

    mem = (int*)shmat(shmID, NULL, 0);
    if(!mem)
    {
        perror("Blad przy dolaczaniu pamieci filozofa");
        exit(1);
    }
}

void MsgError(int test)
{
    if(test == -1)
    {
        if(errno == EIDRM || errno == EINVAL)
        {
            perror("Blad, brak kolejki w filozofie.\n");
            exit(1);
        }
        perror("Blad przy pobieraniu komunikatu.\n");
        exit(1);
    }
}

void PrintForks()
{
    for(int i=0; i<5; i++)
    {
        printf("%d\t", mem[i]);
    }
    printf("\n");
}
