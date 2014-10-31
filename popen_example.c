/*
 * Exemplo simples de popen()
 *
 * Este programa abstrai informações de uma varredura do nmap,
 * e salva servidores com uma característica determinada na constante
 * "DORK", e então salva em um determinado log.
 *
 * */
#include <stdio.h>
#include <string.h>
#define PATH_MAX 500
#define LOG "log_nmap.txt"
#define DORK "open  ssh"

int
Write_File(char *file,char *str)
{
 FILE *arq;
 
 arq=fopen(file,"a");
 
  if(!arq)
  {
   fprintf(stdout,"error in Write_File() %s",file);
   return 0;
  }
 fprintf(arq,"%s\n",str);
 fclose(arq);
 
 return 1;
}


int
main()
{
 FILE *fp;
 char buffer[PATH_MAX+1],tmp[(PATH_MAX<<1)+3];

 puts("Simple nmap Bot :: by Cooler_");

 fp = popen("/usr/bin/nmap 192.168.0.1/24", "r");

  while(fgets(buffer,PATH_MAX,fp) != NULL)
  {
   
   if(strstr(buffer,"report"))
   {
    fprintf(stdout,"%s",buffer);
    bzero(tmp,(PATH_MAX<<1)+2);
    strncpy(tmp,buffer,PATH_MAX);
   }

   if(strstr(buffer,DORK))
   {
    fprintf(stdout,"DORK HERE hohoho \n %s\n", buffer);
    strncat(tmp,buffer,PATH_MAX);
    Write_File(LOG,tmp);
   }
   
  }

 pclose(fp);

 return 0;
}
