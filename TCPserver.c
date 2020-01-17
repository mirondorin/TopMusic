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
    int nr, i = 0, success;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	if (read (tdL.cl, &nr,sizeof(int)) <= 0)
    {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");\
        return 0;
    }
	printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);
    
    char user_name[50], user_pass[50], user_ID[10], user_type[50], user_vote[2], information[300], Song_ID[100], *token, *err_msg = 0, *sql;
    char tmp_user_name[50], tmp_user_pass[50], tmp_user_ID[10];
    rc = sqlite3_open("topmusic.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    
    /*pregatim mesajul de raspuns */
    switch(nr) {
        case 1:
            read(tdL.cl, &information, sizeof(information));
            char userFound[2];
            token = strtok(information, "\n");
            if(token != NULL) {
                strcpy(tmp_user_name, token);
            }
            else {
                break;
            }

            token = strtok(NULL, "\n");
            if(token != NULL) {
                strcpy(tmp_user_pass, token);
            }
            else {
                break;
            }

            sql = (char *)malloc((1000+1)*sizeof(char));
            sprintf(sql, "SELECT COUNT(*) FROM USERS WHERE User_Name = \'%s\' AND User_Pass = \'%s\';", tmp_user_name, tmp_user_pass);
            sqlite3_exec(db, sql, callback_value_first_to_server, userFound, &err_msg);
            if(atoi(userFound) == 1) {
                strcpy(user_name, tmp_user_name);
                strcpy(user_pass, tmp_user_pass);
                sprintf(sql, "SELECT User_ID from Users WHERE User_Name = \'%s\';", user_name);
                sqlite3_exec(db, sql, callback_value_first_to_server, user_ID, &err_msg);
                sprintf(sql, "SELECT User_Type FROM Users WHERE User_ID = %s;", user_ID);
                sqlite3_exec(db, sql, callback_value_first_to_server, user_type, &err_msg);
                sprintf(sql, "SELECT User_Vote FROM Users WHERE User_ID = %s;", user_ID);
                sqlite3_exec(db, sql, callback_value_first_to_server, user_vote, &err_msg);
            }
            break;

        case 2:
            read(tdL.cl, &information, sizeof(information));
            token = strtok(information, "\n");
            if(token != NULL) {
                strcpy(tmp_user_name, token);
            }
            else {
                break;
            }

            token = strtok(NULL, "\n");
            if(token != NULL) {
                strcpy(tmp_user_pass, token);
            }
            else {
                break;
            }
            sql = (char *)malloc((1000+1)*sizeof(char));
            sprintf(sql, "INSERT INTO Users (User_Name, User_Pass) VALUES (\'%s\', \'%s\');", tmp_user_name, tmp_user_pass);
            sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
            break;

        case 3:
            if(strcmp(user_type, "normal") == 0 || strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
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
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;

        case 4:
            if(strcmp(user_type, "normal") == 0 || strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
                char genreName[50];
                read(tdL.cl, &genreName, sizeof(genreName));
                genreName[strlen(genreName) - 1] = '\0';
                sql = (char *)malloc((1000+1)*sizeof(char));
                sprintf(sql, "SELECT COUNT(Song_ID) FROM SONGS WHERE Genre LIKE \'%%%s%%\';", genreName);
                sqlite3_exec(db, sql, callback_send_first_to_client, &tdL.cl, &err_msg);
                sprintf(sql, "SELECT * from SONGS where Genre LIKE \'%%%s%%\';", genreName);
                sqlite3_exec(db, sql, callback, &tdL.cl, &err_msg);
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;

        case 5:
            if(strcmp(user_type, "normal") == 0 || strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
                sql = "SELECT 'youtubelink' || ifnull(max(Song_ID) + 1, 1)  FROM Songs;";
                char songName[50], songGenre[50], songDescription[200], youtubeLink[250];
                sqlite3_exec(db, sql, callback_value_first_to_server, youtubeLink, &err_msg);
                read(tdL.cl, &information, sizeof(information));
                token = strtok(information,"\n");
                if(token != NULL) {
                    strcpy(songName, token);
                }
                else {
                    break;
                }

                token = strtok(NULL, "\n");
                if(token != NULL) {
                    strcpy(songDescription,token);
                }
                else {
                    break;
                }

                token = strtok(NULL, "\n");
                if(token != NULL) {
                    strcpy(songGenre, token);
                }
                else {
                    break;
                }
                sql = (char *)malloc((1000+1)*sizeof(char));
                sprintf(sql, "INSERT INTO SONGS (NAME,LINK,DESCRIPTION,LIKES,GENRE) VALUES (\'%s\',\'%s\',\'%s\',0,\'%s\');", songName, youtubeLink, songDescription, songGenre);
                sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;

        case 6:
            if(strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
                sql = (char *)malloc((1000+1)*sizeof(char));
                read(tdL.cl, &Song_ID, sizeof(Song_ID));
                Song_ID[strlen(Song_ID) -1] = '\0';
                sprintf(sql, "DELETE FROM SONGS WHERE Song_ID = %s;", Song_ID);
                sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;

        case 7:
            if(strcmp(user_type, "normal") == 0 || strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
                sql = (char *)malloc((1000+1)*sizeof(char));
                char comment[200], songFound[2];
                read(tdL.cl, &information, sizeof(information));
                token = strtok(information, "\n");
                if(token != NULL) {
                    strcpy(Song_ID, token);
                }
                else {
                    break;
                }

                token = strtok(NULL, "\n");
                if(token != NULL) {
                    strcpy(comment, token);
                }
                else {
                    break;
                }

                sprintf(sql, "SELECT COUNT(*) FROM Songs WHERE Song_ID = %s;", Song_ID);
                sqlite3_exec(db, sql, callback_value_first_to_server, songFound, &err_msg);
                if(strcmp(songFound, "1") == 0) {
                    sprintf(sql, "INSERT INTO COMMENTS (User_Comment, User_ID, Song_ID) VALUES (\'%s\',%d,\'%s\');", comment, atoi(user_ID), Song_ID);
                    sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
                }
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;

        case 8:
            if(strcmp(user_type, "normal") == 0 || strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
                sql = (char *)malloc((1000+1)*sizeof(char));
                read(tdL.cl, &Song_ID, sizeof(Song_ID));
                Song_ID[strlen(Song_ID) -1] = '\0';
                if(strlen(Song_ID)) {
                    sprintf(sql, "SELECT COUNT(*) from Comments where Song_ID = %s;", Song_ID);
                    sqlite3_exec(db, sql, callback_send_first_to_client, &tdL.cl, &err_msg);
                    sprintf(sql, "SELECT User_Name,User_Comment from Users U JOIN Comments C ON U.User_ID=C.User_ID WHERE Song_ID = %s;", Song_ID);
                    sqlite3_exec(db, sql, callback, &tdL.cl, &err_msg);
                }
                else {
                    write(tdL.cl, "0", 1000);
                }
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;

        case 9:
            if(strcmp(user_type, "normal") == 0 || strcmp(user_type, "admin") == 0) {
                success = 1;
                char userLiked[2];
                write(tdL.cl, &success, sizeof(success));
                sql = (char *)malloc((1000+1)*sizeof(char));
                read(tdL.cl, &Song_ID, sizeof(Song_ID));
                Song_ID[strlen(Song_ID) -1] = '\0';
                sprintf(sql, "SELECT COUNT(*) FROM User_Likes WHERE User_ID = %s AND Song_ID = %s;", user_ID, Song_ID);
                sqlite3_exec(db, sql, callback_value_first_to_server, userLiked, &err_msg);
                if(strcmp(userLiked, "0") == 0) {
                    sprintf(sql, "UPDATE Songs SET Likes = Likes + 1 WHERE Song_ID = %s;", Song_ID);
                    sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
                    sprintf(sql, "INSERT INTO User_Likes (User_ID, Song_ID) VALUES (%d, %s);", atoi(user_ID), Song_ID);
                    sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
                }
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;

        case 10:
            if(strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
                sql = (char *)malloc((1000+1)*sizeof(char));
                read(tdL.cl, &tmp_user_ID, sizeof(tmp_user_ID));
                tmp_user_ID[strlen(tmp_user_ID) - 1] = '\0';
                sprintf(sql, "UPDATE Users SET User_Vote = (User_Vote + 1)%%2 WHERE User_ID = %s;", tmp_user_ID);
                sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
            break;
        
        case 11:
            if(strcmp(user_type, "admin") == 0) {
                success = 1;
                write(tdL.cl, &success, sizeof(success));
                sql = (char *)malloc((1000+1)*sizeof(char));
                read(tdL.cl, &tmp_user_ID, sizeof(tmp_user_ID));
                tmp_user_ID[strlen(tmp_user_ID) - 1] = '\0';
                sprintf(sql, "UPDATE Users SET User_Type = 'admin' WHERE User_ID = %s;", tmp_user_ID);
                sqlite3_exec(db, sql, callback_void, &tdL.cl, &err_msg);
            }
            else {
                success = 0;
                write(tdL.cl, &success, sizeof(success));
            }
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
        char buf[1000];
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
    write(socketDescriptor, argv[0] ? argv[0] : "0", 1000);    
    return 0;
}
