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

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0, iterations = 1000;
  char buf[1000];

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
      return errno;
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
		printf ("[client]Introduceti un numar: ");
		fflush (stdout);
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
                if (read (sd, &buf,sizeof(buf)) < 0)
                {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                printf("buffer are valoarea %s\n",buf);
                lines_count = atoi(buf) * 4;
                printf("lines_count = %d\n",lines_count);
                
                for(int i = 1; i <= lines_count; i++) {
                    if (read (sd, &buf,sizeof(buf)) < 0)
                    {
                        perror ("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    else{
                      write(sd, "1", 1);
                    }
                    /* afisam mesajul primit */
                    printf ("[client]Mesajul primit este: %s", buf);
                    if (i % 4 == 0) printf("\n");
                }
                break;
            case 2:
                if (read (sd, &buf,sizeof(buf)) < 0)
                {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                printf ("%s", buf);
                
                char name[50],description[200];
                
                read(0, name, sizeof(name));
                
                //Send name for song to server
                if (write (sd,&name,sizeof(name)) <= 0)
                {
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                
                if (read (sd, &buf,sizeof(buf)) < 0)
                {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                printf ("\n%s", buf);
                
                read(0, description, sizeof(description));
                
                if (write (sd,&description,sizeof(description)) <= 0)
                {
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                
                if (read (sd, &buf,sizeof(buf)) < 0)
                {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                printf ("\n%s", buf);
                
                break;
            default:
                if (read (sd, &buf,sizeof(buf)) < 0)
                {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                printf ("[client]Mesajul primit este: %s\n", buf);
        }
	}
  /* inchidem conexiunea, am terminat */
  close (sd);
}

