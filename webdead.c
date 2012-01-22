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
#define THREAD 10

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
char *substr(char *src, const int start, const int count);
char **split(char *src, const char *token, int *total);
char *readLine(char * NameFile);
char *StrRep(char *st,char *orig,char *repl,const int mim); 

// inicio do código
int main (int argc, char *argv[])
{
 int serversock=0,port=0,count=0;
 int **pack=(int**)alloca(1*sizeof(int));

 if(argc < 1)
  DEBUG("Precisa usar um argumento para porta");

 port=atoi(argv[1]);
 if(!(port<=65535&& port>0))
 {
  DEBUG("error in PORT\n");
  return 0;
 }
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
   if(pthread_create(&thread[count], NULL, request, (void *)pack) != 0)
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
 memset(&self, 0, sizeof(self));
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
 int clientsock,socklen,total;   
 struct sockaddr_in clientaddr;
 char *resposta=(char *)alloca(MAX*sizeof(char));
 char *pergunta=(char *)xmalloc(BUF*sizeof(char));
 char **parse,**URL,**POST;

  memset(&clientaddr, 0, sizeof(clientaddr));
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
//  puts(pergunta);
// split da head , para pegar URL outras coisas
  total=12;
  parse=split(pergunta,"\r",&total); //16);
//split para extrai endereço de html da URL em URL[1]
  total=4;
  URL=split(parse[0]," ",&total);

  while(parse[count] != NULL)
  {
   fprintf(stdout,"%s",parse[count]); 
   count++;
  }

// se encontrar algum POST, mostra e dá parser
  if(strstr(parse[0],"POST"))
  {
   total=6;
   fprintf(stdout,"\nLog of POST: \n");
   POST=split(parse[count-1],"&",&total);
    if(POST!=NULL)
    {
     fprintf(stdout,"\n %s  \n",POST[0]);
     while(total)
     {
      free(POST[total]);
      total--;
     }
    }
  }

  strncat(resposta,"HTTP/1.1 200 Ok\n\n",26);
  URL[1]=StrRep(URL[1],"/","",sizeof(URL[1]));  
  strncat(resposta,readLine(URL[1]),BUF);

  count=4;
  while(count)
  {
   free(URL[count]);
   count--;
  }
  count=12;
  while(count)
  {
   free(parse[count]);
   count--;
  }

// OUT para o cliente
  write(clientsock, resposta, strlen (resposta));

 free(URL);
 free(parse);

// unlock 
 pthread_mutex_unlock(&morfo);  

 }
 free(pergunta);

// fecha conexão 
 close(clientsock);

 pthread_exit(NULL);
}

char *substr(char *src, const int start, const int count)
{
 char *tmp,*tmp2;
 tmp = (char *)xmalloc(count+1);
 if (tmp == NULL) 
 {
  DEBUG("error");
  return NULL;
 }

 strncpy(tmp, src+start, count);
 tmp[count] = '\0';
 strncpy(tmp2,tmp,count);
 free(tmp);

 return tmp;
}

// split 
char **split(char *src, const char *token, int *total)
{ 
 char **str;
 register int i, j, count, item, start;
 int len;

 if(!src || !token) 
 {
  *total = 0;
  return NULL;
 }

 count = item = start = 0;
 j = 1;
 len = strlen(src);

 for(i = 0; i < len; i++) 
 {
  if(src[i] == *token)
   count++;
 }

 if(!count) 
 {
  *total = 0;
  return NULL;
 }

 str = (char **)xmalloc(count * sizeof(char *));
 if(str == NULL)
  DEBUG("error");

 for(i = 0; i < len; i++) 
 {
  if(src[i] != *token)
   j++;
  else {
   str[item] = (char *)xmalloc(j-start);
    if (str[item] == NULL) 
    {
     DEBUG("error");
     return NULL;
    }
   *(str+item) = substr(src, start, j-1);
   str[item][j-start-1] = '\0';
   item++;
   start = j++;
  }
 }

 *(str+count) = (char *)xmalloc(j);
 if(str[count] == NULL)
  DEBUG("error");

 str[count] = substr(src, start, j);
 str[count][j-start] = '\0';
 *total = ++count;

 free(str);

 return str;
}
        

// return lines from file, example:  const char *buff=readLine("log.txt"),printf("%s",buff);
char *readLine(char * NameFile)
{
 FILE * file;
 char *lineBuffer=calloc(1,1), line[128];

 file=fopen(NameFile, "r");

 if( !file || !lineBuffer )
  return "error in your URL";
 
 while(fgets(line,sizeof line,file))  
 {
  lineBuffer=realloc(lineBuffer,strlen(lineBuffer)+strlen(line)+1);
  if(!lineBuffer)
   DEBUG("error in readLine() at %s",NameFile);
  strncat(lineBuffer,line,MAX);
 }

 fclose(file);
 free(lineBuffer);
 return lineBuffer;
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