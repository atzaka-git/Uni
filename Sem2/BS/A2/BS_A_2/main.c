#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MESSZEIT 5
#define FNAME "testfile.txt"
#define N_GRANDKIDDIES 2

// Prototypen
int kiddyscode(void);
int grandkiddyscode(int nr);

// globale Variablen
int pids[2];
int n_grandkiddies;
int countSIGUSR1;
int countSIGUSR2;
int stop = 0;

// Signalhandler
// SIGUSR1 = 10
// SIGUSR2 = 12
// SIGCHLD = 17
// SIGUALRM = 14
// einrichten
void initSignalhandler(int sig, struct sigaction sa , void (*handler))
{
    printf("in init für sig%i\n", sig);
    memset (&sa, 0,sizeof(sa));
    sa.sa_handler = handler;
    sigaction(sig, &sa, NULL);
}

// SIGCHLD Handler
void handle_sigchld(int sig)
{
    printf("in SIGCHLD\n");
    n_grandkiddies--;

    if (n_grandkiddies == 0)
    {
    // schreibt in testfile.txt
    FILE *fp;
    fp = fopen(FNAME, "w");
    char * name1 = "PID-1";
    char * name2 = "PID-2";
    fprintf(fp, "Keine Enkel mehr\n%s hat %i Auslöser\n%s hat %i Auslöser\n", name1, countSIGUSR1, name2, countSIGUSR2);
    fclose(fp);
    }
}

// SIGUSR(1 und 2) Handler
void handle_sigusr(int sig, int sigNr)
{
    if (sig == SIGUSR1) countSIGUSR1++;
    if (sig == SIGUSR2) countSIGUSR2++;
}

// SIGALARM Handler
void handle_sigalrm(int sig)
{
    printf("in SIGALRM\n");
    stop = 1;
}


// Prozess Startpunkt
int main()
{
printf("IN MAIN\n");
    // Erstellt testfile.txt
    FILE *fp;
    fp = fopen(FNAME, "w");
    fprintf(fp ,"Test-Text");
    fclose(fp);

     //Erstellt Kindprozess
    switch(fork())
    {
        case -1:
            // Fehler
            perror("fork");
            exit(1);
            break;
        case 0:
            // Kindprozess
            kiddyscode();
            return 0;
        default:
            // Elternprozess
            // Wartet auf Kindprozess
            wait(NULL);
            printf("KIDDY ANGEKOMMEN\n");
    }

    // Datei auslesen
    fp = fopen(FNAME, "r");
    char c;
    while((c = fgetc(fp)) != EOF)
        printf("%c", c);
    fclose(fp);

    // Exit Prozess
    exit(0);
}

int kiddyscode(void)
{
    printf("IN KID\n");

    struct sigaction sa1;
    initSignalhandler(SIGCHLD, sa1, handle_sigchld);
    initSignalhandler(SIGUSR1, sa1, handle_sigusr);
    initSignalhandler(SIGUSR2, sa1, handle_sigusr);

    for(int i = 1; i <= N_GRANDKIDDIES; i++)
    {
        pids[i]= fork();
        printf("PID:%i       Durch: %i  PIDS1: %i und PIDS2: %i\n", getpid(), i, pids[1], pids[2]);
        if(pids[i] == -1) exit(-1);
        else if(pids[i]==0)
        {
            grandkiddyscode(i);
            printf("after grandkiddies\n");
            exit(0);
        }
        else
        {
            n_grandkiddies++;
        }
    }

    for(int i = 1; i <= N_GRANDKIDDIES; i++)
    {
        printf("Nummer %i start!\n", i);
        int pid = wait(NULL);
        printf("Nummer %i mit pid %i fertig!\n", i, pid);
    }
    printf("nach for!!\n");

    return 0;
}

int grandkiddyscode(int nr)
{
    printf("IN GRANDKIDDIES\n");

    struct sigaction sa1;
    initSignalhandler(SIGALRM, sa1, handle_sigalrm);

    alarm(MESSZEIT);

    while(stop != 1)
    {
        if(nr == 1) {
        kill(getppid(), SIGUSR1);
        }
        if(nr == 2) {
        kill(getppid(), SIGUSR2);
        }
    }
    printf("after while in grandkiddies\n");
    return 0;
}


