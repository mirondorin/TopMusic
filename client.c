/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

int quit = 0, lines_count = 0;

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

void printMenu();
void readFromServer(int socketDescriptor, char * buf, int sizeBuffer);
void writeToServer(int socketDescriptor, char * buf, int sizeBuffer);
void listSongs(int sd);

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0, iterations = 1000;
  char buf[1000], user_name[50], user_pass[50], user_ID[10], temp_user[300], Song_ID[100], information[300];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      exit errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  while(quit == 0) {
		/* citirea mesajului */
		printMenu();
		read (0, buf, sizeof(buf));
		nr=atoi(buf);
		//scanf("%d",&nr);
		
		printf("[client] Am citit %d\n",nr);

		/* trimiterea mesajului la server */
		if (write (sd,&nr,sizeof(int)) <= 0)
		  {
		    perror ("[client]Eroare la write() spre server.\n");
		    return errno;
		  }

		/* citirea raspunsului dat de server 
		   (apel blocant pina cind serverul raspunde) */
        switch(nr) {
            case 1:
                printf("username:");
                fgets(user_name, sizeof(user_name), stdin);
                printf("password:");
                fgets(user_pass, sizeof(user_pass), stdin);
                strcpy(temp_user, user_name);
                strncpy(temp_user + strlen(temp_user), user_pass, strlen(user_pass) + 1);
                write(sd, &temp_user, sizeof(temp_user));
                break;

            case 2:
                printf("Enter a username:");
                fgets(user_name, sizeof(user_name), stdin);
                printf("Enter a password:");
                fgets(user_pass, sizeof(user_pass), stdin);
                strcpy(temp_user, user_name);
                strncpy(temp_user + strlen(temp_user), user_pass, strlen(user_pass) + 1);
                write(sd, &temp_user, sizeof(temp_user));
                break;

            case 3:
                listSongs(sd);
                break;

            case 4:
                printf("What genre of songs would you like to see: ");
                char genreName[50],tmp_buf[1000];
                fgets(genreName, sizeof(genreName), stdin);
                write(sd, &genreName, sizeof(genreName));
                int lines_count;
                read(sd, &tmp_buf, sizeof(tmp_buf));
                lines_count = atoi(tmp_buf) * 6;
                printf("lines_count este = %d\n", lines_count);
                for(int i = 1; i <= lines_count; i++) {
                    read(sd, &tmp_buf, sizeof(tmp_buf));
                    write(sd, "1", 1);
                    /* afisam mesajul primit */
                    printf ("[client]Mesajul primit este: %s", tmp_buf);
                    if (i % 6 == 0) printf("\n");
                }
                break;

            case 5:
                printf("Insert a name for the song: ");
                char name[50], genre[50], description[200], *token;
                fgets(name, sizeof(name), stdin);
                printf("Insert a description for the song: ");
                fgets(description, sizeof(description), stdin);
                printf("\nInsert a genre for the song: ");
                fgets(genre, sizeof(genre), stdin);
                strcpy(information, name);
                strncpy(information + strlen(information), description, strlen(description) + 1);
                strncpy(information + strlen(information), genre, strlen(genre) + 1);
                write(sd, &information, sizeof(information));   
                break;

            case 6:
                printf("Insert the ID of the song you would like to delete: ");
                fgets(Song_ID, sizeof(Song_ID), stdin);
                write(sd, &Song_ID, sizeof(Song_ID));
                break;

            case 7:
                printf("Insert the ID of the song you would like to comment on: ");
                fgets(Song_ID, sizeof(Song_ID), stdin);
                printf("Comment: ");
                char comment[200];
                fgets(comment, sizeof(comment), stdin);
                strcpy(information, Song_ID);
                strncpy(information + strlen(information), comment, strlen(comment) + 1);
                write(sd, &information, sizeof(information));
                break;

            case 8:
                printf("Insert the ID of the song you would like to see the comments: ");
                fgets(Song_ID, sizeof(Song_ID), stdin);
                write(sd, &Song_ID, sizeof(Song_ID));
                int comments_count;
                read(sd, &buf, sizeof(buf));
                comments_count = atoi(buf) * 2;
                for(int i = 1; i <= comments_count; i++) {
                    read(sd, &buf,sizeof(buf));
                    write(sd, "1", 1);
                    /* afisam mesajul primit */
                    printf ("[client]Mesajul primit este: %s", buf);
                    if (i % 2 == 0) printf("\n");
                }
                break;

            case 9:
                printf("Insert the ID of the song you would like to upvote: ");
                fgets(Song_ID, sizeof(Song_ID), stdin);
                write(sd, &Song_ID, sizeof(Song_ID));
                break;

            case 10:
                printf("Insert the ID of the user you would like to restrict from upvoting: ");
                fgets(user_ID, sizeof(user_ID), stdin);
                write(sd, &user_ID, sizeof(user_ID));
                break;

            case 11:
                printf("Insert the ID of the user you would like to grant admin privileges: ");
                fgets(user_ID, sizeof(Song_ID), stdin);
                write(sd, &user_ID, sizeof(user_ID));
                break;
        
            default:
                printf("Invalid number\n");
        }
	}
  /* inchidem conexiunea, am terminat */
  close (sd);
}

void listSongs(int sd)
{
    char buf[1000];
    int lines_count;
    read(sd, &buf, sizeof(buf));
    printf("buffer are valoarea %s\n",buf);
    lines_count = atoi(buf) * 6;
    printf("lines_count = %d\n",lines_count);
    
    for(int i = 1; i <= lines_count; i++) {
        read(sd, &buf,sizeof(buf));
        write(sd, "1", 1);
        /* afisam mesajul primit */
        printf ("[client]Mesajul primit este: %s", buf);
        if (i % 6 == 0) printf("\n");
    }
}

void readFromServer(int socketDescriptor, char *buf, int sizeBuffer){
    if (read (socketDescriptor, &buf, sizeBuffer) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        exit errno;
    }
}

void writeToServer(int socketDescriptor,char *buf, int sizeBuffer){
    if (write (socketDescriptor, &buf, sizeBuffer) <= 0)
    {
        perror ("[client]Eroare la write() spre server.\n");
        exit errno;
    }    
}

void printMenu()
{
    printf("Here's a list of the available commands: \n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("3. List Songs\n");
    printf("4. List Songs from a Genre\n");
    printf("5. Add Song\n");
    printf("6. Delete Song\n");
    printf("7. Comment on a song\n");
    printf("8. Show comments from a song\n");
    printf("9. Like a song\n");
    printf("10. Remove/Grant user's right to vote\n");
    printf("11. Make admin\n");
    printf("\nEnter the number for your desired command: ");
    fflush(stdout);
}