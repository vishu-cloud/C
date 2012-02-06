/*
contato: c00f3r@gmail.com
autor: Antonio Costa aka Cooler_
coolerlab.wordpress.com

Simple  Heartbeat detector with arduino

 */
#include <stdio.h>    
#include <stdlib.h> 
#include <string.h>   
#include <unistd.h>   
#include <termios.h>  
#include <fcntl.h> 
#include <time.h>

// sempre concatene com 'B' a  taxa de transmissão ,"9600" padrão do AVR
#define BAUDRATE B9600

// macros debug
#define BUGVIEW 1

#define DEBUG(x, s...) do { \
 if (!BUGVIEW) { break; } \
 time_t t = time(NULL); \
 char *d = ctime(&t); \
 fprintf(stderr, "%.*s %s[%d] %s(): ", \
 (int)strlen(d) - 1, d, __FILE__, \
 __LINE__, __FUNCTION__); \
 fprintf(stderr, x, ## s); \
} while (0);

// vai pegar os dados a cada 5 segundos
#define SECOND 5

int serialboot(const char* serialport, int baud);
int serialread(int fd, char* buf, char until);
int WriteFile(char *file,char *str);

void banner() 
{
 printf("\nFollow patern: ./heartbeat <SerialPort> <times 2 get>\n"
  "Just Another geek thing to get HeartBeat and plot!!!\n"
  "Coded By Cooler_\n"
  "coolerlab.wordpress.com\n"
  "\n");
}

int main(int argc, char *argv[]) 
{
 int baudrate = BAUDRATE,fd=0,counter=0,times=0; 
 char *buf=malloc(sizeof(char)*16);
 char *serialport=malloc(sizeof(char)*512);

// apagamos o último log
 unlink("logbeat.txt");

  if(argc<2) 
  {
   banner();
   exit(EXIT_SUCCESS);
  }
    
  printf("Serial:%s\n",argv[1]);

  strncpy(serialport,argv[1],512);
  fd=serialboot(serialport, baudrate);

   if(fd<=0)
   {
    DEBUG("veja se o dispositivo esta conectado!!");
    DEBUG("%d\n",fd);
    exit(EXIT_SUCCESS);
   }

//número de dados que seram pegos a cada 5 segundos
  times=atoi(argv[2]);
  
  while(1) 
  {
   serialread(fd, buf, '\n');

   if(strcmp(buf," ")>0)
   {
    fprintf(stdout,"%s",buf);
    sleep(SECOND);
    WriteFile("logbeat.txt",buf);
    times--;
   }
// se terminar número de vezes então pula
   if(!times)
    break;    
  }

// nosso amigão gnuplot nos ajudando :-)
 FILE *pipe = popen("gnuplot -persist","w");
  fprintf(pipe, "set grid'\n");
  fprintf(pipe, "set style data lines\n");
  fprintf(pipe, "set terminal png\n");
  fprintf(pipe, "set output 'heartbeat.png'\n");
  fprintf(pipe, "plot \"logbeat.txt\"\n");
 pclose(pipe);

// fechando a banca xD
 puts("end HeartBeat, look image heartbeat.png");
 close(fd);
 free(buf);
 free(serialport);

 exit(EXIT_SUCCESS);    
} 

int serialread(int fd, char* buf, char until)
{
 char b[1];
 int i=0;
 do { 
  int n=read(fd, b, 1);  
  if(n==-1) 
    return -1;    
  if(!n) 
  {
   usleep(16000); 
   continue;
  }
  buf[i] = b[0]; 
  i++;
 } while(b[0]!=until);

 buf[i]=0;  

 return 0;
}

int serialboot(const char* serialport, int baud)
{
 struct termios toptions;
 int fd;

    fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);

    if(fd == -1)  
    {
     DEBUG("serialboot: não foi possivel abrir a porta ");
     return -1;
    }
    
    if(tcgetattr(fd, &toptions) < 0) 
    {
     DEBUG("serialboot: nao foi possivel pegar atributos do terminal");
     return -1;
    }
    speed_t brate = baud; 
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);
   // para default recv com termios.h
    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;
    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // olhe http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;
    
    if(tcsetattr(fd, TCSANOW, &toptions) < 0) 
    {
     DEBUG("serialboot: nao foi possivel adicionar atributos no term erro 1");
     return -1;
    }

 return fd;
}


//escreve num txt
int WriteFile(char *file,char *str)
{
 FILE *arq;
 
 arq=fopen(file,"a"); 
  if(!arq) 
  {
   DEBUG("error in WriteFile() %s",file); 
   return 0;
  }
 fprintf(arq,"%s",str); 
 fclose(arq); 
 return 1;
}

