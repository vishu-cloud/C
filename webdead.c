// example how too do http server in C , by Cooler_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <alloca.h>

// log de conexões
#define LOG "log.txt"

// Buffer strings
#define BUF 512

// buffer maximo
#define MAX  26512

// maximo de threads para conexões multiplas 
#define THREAD 20

// macros para depuração , 0 para desativar
#define BUGVIEW 1

#define DEBUG(x, s...) do { \
 if (!BUGVIEW) { break; } \
 time_t t = time(NULL); \
 char *d = ctime(&t); \
 fprintf(stderr, "%.*s %s[%d] %s(): \n", \
 (int)strlen(d) - 1, d, __FILE__, \
 __LINE__, __FUNCTION__); \
 fprintf(stderr, x, ## s); \
} while (0);

//para evitar reentrancia  
pthread_mutex_t morfo = PTHREAD_MUTEX_INITIALIZER;

// funções diversas
void *xmalloc(unsigned int len);
int init_sock(int serversock,int port);
void *request(void * sock);
char **split(char *string, char separator); 
int WriteFile(char *file,char *str);
char *readLine(char * NameFile);
char *TimeNow();
char *StrFlip(char *str);
char *StrRep(char *st,char *orig,char *repl,const int mim); 

// inicio do código
int main (int argc, char *argv[])
{
 int serversock=0,port=0,count=0;
 int **pack=(int**)alloca(1*sizeof(int));

 if(argc < 1)
  DEBUG("Precisa usar um argumento para porta");

 port=atoi(argv[1]);
 serversock=init_sock(serversock,port);

 pack[0]=(int *)port;
 pack[1]=(int *)serversock; 

 pthread_t *thread;   
 thread=(pthread_t *)xmalloc(THREAD*sizeof(pthread_t));

 while(1)
 {
  if(count!=THREAD)
  {
//accept e passa o fd q ele retornou como argumento, no create
   if(pthread_create (&thread[count], NULL, request, (void *)pack) != 0)
    DEBUG("problema no pthread_create"); 
   count++;
  } else {
   count=0;
   while(count!=THREAD) 
   {
    if(pthread_join (thread[count], NULL))
     DEBUG("problema no pthread_join");
    count++;
   }
   count=0;
   free(thread);
   thread=(pthread_t *)xmalloc(THREAD*sizeof(pthread_t));
  }
 }
 
 free(thread);
 close(serversock);
 
 return 0;
}

// alinhamento da memória HEAP :-X
void *xmalloc(unsigned int len)
{
 void *ptr;
 ptr=malloc(len);
 if(ptr==NULL) 
  DEBUG("fail malloc");
 return ptr;
}

// boot to start socket
int init_sock(int serversock,int port)
{
 struct sockaddr_in self;
 
   /* cria o socket */
 serversock = socket(AF_INET, SOCK_STREAM, 0) ;
  if(serversock < 0)
   DEBUG("Erro no socket");
 
   /* monta o endereço */
 bzero(&self, sizeof(self));
 self.sin_family=AF_INET;
 self.sin_port=htons(port);
 self.sin_addr.s_addr=INADDR_ANY; /* uso o endereço do host */
 
   /* associa o endereço ao socket */
  if(bind(serversock, (struct sockaddr*)&self, sizeof(self)) )
   DEBUG("Erro no bind");
 
   /* coloca o socket em escuta */
  if(listen(serversock, 20) )
   DEBUG("Erro no listen");
 
 return serversock;
}

//pega a pergunta e da a resposta
void *request(void * sock)
{
 int **unpack = (int **)sock;
 int serversock=(int)unpack[1],port=(int)unpack[0],count=0;
 int clientsock,socklen;   
 struct sockaddr_in clientaddr;
 char *resposta=(char *)alloca(MAX*sizeof(char));
 char *pergunta=(char *)xmalloc(BUF*sizeof(char));
 char **parse=(char **)xmalloc(16*sizeof(char *));
 char **URL=(char **)xmalloc(8*sizeof(char *));
 char **POST=(char **)xmalloc(4*sizeof(char *));

  bzero(&clientaddr, sizeof(clientaddr));
  clientaddr.sin_family=AF_INET;
  clientaddr.sin_port=htons(port);
  clientaddr.sin_addr.s_addr=INADDR_ANY; 

// aguarda e aceita uma conexão externa 
 socklen=sizeof(clientaddr);
 clientsock = accept(serversock, (struct sockaddr *)&clientaddr,(socklen_t  *)&socklen); 

 if(clientsock>0)
  DEBUG("\nLog: \n");
 fprintf(stdout,"\nClient %s:%d connected\n", inet_ntoa (clientaddr.sin_addr),ntohs (clientaddr.sin_port));

// lock
  pthread_mutex_lock(&morfo); 

// pega useragent ,POST e GET etc...
 if(recv(clientsock,pergunta,BUF,0)!=-1)
 {
// split da head , para pegar URL outras coisas
  parse=split(pergunta,'\r'); //16);
//split para extrai endereço de html da URL em URL[1]
  URL=split(parse[0],' ');

  while(count != 10)
  {
   fprintf(stdout,"%s",parse[count]); 
   count++;
  }

// se encontrar algum POST, mostra e dá parser
  if(strstr(parse[0],"POST"))
  {
   fprintf(stdout,"\nLog of POST: %s \n",parse[12]);
   POST=split(parse[12],'&');
   fprintf(stdout,"\n %s  :  %s \n",POST[0],POST[1]);
   
  }

  strcat(resposta,"HTTP/1.1 200 Ok\n\n");
  URL[1]=StrRep(URL[1],"/","",sizeof(URL[1]));  
  strcat(resposta,readLine(URL[1]));
     
// OUT para o cliente
  write(clientsock, resposta, strlen (resposta));

// unlock 
 pthread_mutex_unlock(&morfo);  

 }

// fecha conexão 
 close(clientsock);

 free(POST);
 free(pergunta);
 free(URL);
 free(parse);

 pthread_exit(NULL);
}


// split igual do Perl e do PHP  x-)
char **split(char *string, char separator) 
{
 int inicio=0,count=2,i=0,x=0;
 char **newarray;

 while(string[i] != '\0')
 {
// numero de elementos que tera nosso array 
  if(string[i] == separator)
   count++;
  i++;              
 }

 newarray=calloc(count,sizeof(char*));
 i=0;

 while(*string!='\0') 
 {
  if(*string==separator) 
  {
   newarray[i]=calloc(x-inicio+2,sizeof(char));
   strncpy(newarray[i],string-x+inicio,x-inicio);
   newarray[i][x-inicio+1]='\0'; 
   inicio=x;
   inicio++;
   i++;
  }
  string++;
  x++;
 }
        
 newarray[i] = calloc(x - inicio + 1,sizeof(char));
 strncpy(newarray[i],string-x+inicio,x-inicio);
 newarray[++i] = NULL;
 
 return newarray;
}

// escreve em um arquivo
int WriteFile(char *file,char *str)
{
 FILE *arq;
 
 arq=fopen(file,"a"); 
 if(!arq) 
  DEBUG("error in WriteFile() %s",file);    
 fprintf(arq,"%s\n",str); 
 fclose(arq); 

 return 1;
}

// return lines from file, example:  const char *buff=readLine("log.txt"),printf("%s",buff);
char *readLine(char * NameFile)
{
 FILE * file;
 char *lineBuffer=calloc(1,1), line[128];

 file=fopen(NameFile, "r");

 if( !file || !lineBuffer )
  return "error in your URL";

//  DEBUG("error in readLine() at %s",NameFile);
 
 while(fgets(line,sizeof line,file))  
 {
  lineBuffer=realloc(lineBuffer,strlen(lineBuffer)+strlen(line)+1);
  if(!lineBuffer)
   DEBUG("error in readLine() at %s",NameFile);
  strcat(lineBuffer,line);
 }

 fclose(file);
 return lineBuffer;
}

char *TimeNow()
{
 struct tm *local;
 time_t t;
 t = time(NULL);
 local = localtime(&t);
 local = gmtime(&t);
 return asctime(local);
}

// return reverse string
char *StrFlip(char *str)
{
 char *p1, *p2;

 if(! str || ! *str)
  return str;
 for(p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
 {
  *p1 ^= *p2;
  *p2 ^= *p1;
  *p1 ^= *p2;
 }
 return str;
}


//string replace
char *StrRep(char *st,char *orig,char *repl,const int mim) 
{
 char bufer[mim];
 char *ch;

  if(!(ch = strstr(st, orig)))
   return st;
 strncpy(bufer, st, ch-st);  
 bufer[ch-st] = 0;
 sprintf(bufer+(ch-st),"%s%s",repl,ch+strlen(orig));

 char *out=bufer;

 return out;
}
