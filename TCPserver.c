/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include "myfunctions.h"

int quit = 0, rc;
char buf[1000];

FILE * pFile;
sqlite3 *db;

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
_Bool raspunde(void *);
int callback(void *, int, char **, char **);
int callback_void(void *, int, char **, char **);
int callback_send_first_to_client(void *, int, char **, char **);
int callback_value_first_to_server(void *, int, char**, char **);
void readFromClient(int, char*, int);

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;	
    int nr;		//mesajul primit de trimis la client 
    int sd;		//descriptorul de socket 
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;
    
   //Start database
    //startDB(rc, db);

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};

static void *treat(void * arg)
{		
	while(quit == 0) {		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
    _Bool success;
		success = raspunde((struct thData*)arg);
    if(!success){
      break;
    }
	}
		/* am terminat cu acest client, inchidem conexiunea */
	close ((intptr_t)arg);
	return(NULL);
  		
};

_Bool raspunde(void *arg)
{
    int nr, i = 0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	if (read (tdL.cl, &nr,sizeof(int)) <= 0)
    {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");\
        return 0;
    }
	printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);
    
    char *err_msg = 0, *sql;
    
    rc = sqlite3_open("topmusic.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    
    /*pregatim mesajul de raspuns */
    switch(nr) {
        case 3:
            sql = "SELECT COUNT(*) FROM Songs";
            rc = sqlite3_exec(db, sql, callback_send_first_to_client, &tdL.cl, &err_msg); 
            
            //Select all songs sorted desc by their likes
            sql = "SELECT * FROM Songs ORDER BY Likes DESC";
            
            rc = sqlite3_exec(db, sql, callback, &tdL.cl, &err_msg);
            
            if (rc != SQLITE_OK ) {
                
                fprintf(stderr, "Failed to select data\n");
                fprintf(stderr, "SQL error: %s\n", err_msg);

                sqlite3_free(err_msg);
                sqlite3_close(db);
            }
            break;

        case 4:
            sql = "SELECT 'youtubelink' || ifnull(max(Song_ID) + 1, 1)  FROM Songs;";
            char songName[50], songDescription[200], youtubeLink[250], information[250], *token;
            sqlite3_exec(db, sql, callback_value_first_to_server, youtubeLink, &err_msg);
            read(tdL.cl, &information, sizeof(information));
            token = strtok(information,"\n");
            strcpy(songName, token);
            token = strtok(NULL,"\n");
            strcpy(songDescription,token);
            songDescription[strlen(songDescription)] = '\0';
            sql = (char *)malloc((1000+1)*sizeof(char));
            sprintf(sql, "INSERT INTO SONGS (NAME,LINK,DESCRIPTION,LIKES) VALUES (\'%s\',\'%s\',\'%s\',0);", songName, youtubeLink, songDescription);
            sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
            break;

        case 5:
            sql = (char *)malloc((1000+1)*sizeof(char));
            char Song_ID[100];
            read(tdL.cl, &Song_ID, sizeof(Song_ID));
            sprintf(sql, "DELETE FROM SONGS WHERE Song_ID = %s;", Song_ID);
            sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
            break;
        default: 
           printf("User did not insert a valid number\n");
    }
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, buf);
  return 1;
}

void readFromClient(int socketDescriptor, char *buf, int sizeBuffer){
    if (read (socketDescriptor, &buf, sizeBuffer) < 0)
    {
        perror ("[client]Eroare la read() de la client.\n");
        exit errno;
    }
}

int callback(void *sd, int argc, char **argv, char **azColName) {
    int socketDescriptor = *((int *)sd);
    for (int i = 0; i < argc; i++) {
        char buf[500];
        sprintf(buf ,"%s = %s\n", azColName[i], argv[i] ? argv[i] : " ");
        printf("Sent to %d: %s\n", socketDescriptor, buf);
        write(socketDescriptor, buf, sizeof(buf));
        read(socketDescriptor, buf, sizeof(buf));
        if(atoi(buf) != 1){
            perror("Error, client didn't get song info");
        }
    }    
    return 0;
}

int callback_value_first_to_server(void *value, int argc, char **argv, char **azColName) {
    strcpy(value, argv[0]);
    return 0;
}

int callback_void(void *value, int argc, char **argv, char **azColName) {
    return 0;
}

int callback_send_first_to_client(void *sd, int argc, char **argv, char **azColName) {
    int socketDescriptor = *((int *)sd);
    write(socketDescriptor, argv[0] ? argv[0] : "0", sizeof(argv[0]));    
    return 0;
}
