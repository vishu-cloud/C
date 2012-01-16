/*
contato: c00f3r@gmail.com
autor: Antonio Costa aka Cooler_
coolerlab.wordpress.com

Simples detector de ladrão usando Arduino+sensor de movimento

apt-get install libhighgui-dev  libcv
gcc -o thiefget thiefget.c  -lcv -lhighgui -lcxcore; ./thief

usei o Opencv para tirar shoot da webcam pois não conheço outra API
que faça o mesmo...

só para enviar e-mail que usei programa de terceiro... parece gambi até xD

 */
#include <stdio.h>    
#include <stdlib.h> 
#include <string.h>   
#include <unistd.h>   
#include <termios.h>  
#include <fcntl.h> 
#include "opencv/cv.h"
#include "opencv/highgui.h"

// quantas vezes o sensor retornara 1 para mandar e-mail ?
#define MOVES 16

// numero que toleda erros
#define FAIL 16

// sempre concatene com 'B' a  taxa de transmissão ,"9600" padrão do AVR
#define BAUDRATE B9600

int serialboot(const char* serialport, int baud);
int serialread(int fd, char* buf, char until);
void webcam(const char* file);

void banner() 
{
 printf("\nFollow patern: ./PegaLadrao <SerialPort>\n"
  "Just Another geek Cool THing With ArduinO!!!\n"
  "Simple THief Get\n"
  "Coded By Cooler_\n"
  "coolerlab.wordpress.com\n"
  "\n");
}

int main(int argc, char *argv[]) 
{
 int baudrate = BAUDRATE,fd=0,counter=0,OFF=0; 
 char *buf=malloc(sizeof(char)*16);
 char *serialport=malloc(sizeof(char)*512);

  if(argc<2) 
  {
   banner();
   exit(EXIT_SUCCESS);
  }
    
  printf("Serial:%s\n",argv[1]);

//conection
  strncpy(serialport,argv[1],512);
  fd=serialboot(serialport, baudrate);

   if(fd<=0)
   {
    puts("veja se o dispositivo esta conectado!!");
    printf("%d\n",fd);
    exit(EXIT_SUCCESS);
   }
 
  while(1) 
  {
    
// tempo em milesegundos
   serialread(fd, buf, '\n');
   printf("====================\nread: %s\ncounter: %d\n===================\n",buf,counter);

     if(strcmp(buf,"1")>0)
     {
      usleep(500000);
      puts("MOVEMENT ONLINE");
      puts(buf);
      counter++;
      OFF=0;
     } 
//numero de vezes que achou movimento
     if(counter==MOVES)
     {
      puts("cof cof thief in HOME");
      webcam("test.jpg");
      system("echo 'LogLog!!!' | mutt -s 'THief Get LOg' -a /home/nick/c/arduino/test.jpg -- youremail@gmail.com");
      break;
      counter=0;
     } 
     if(!strlen(buf))
      OFF--;
     if(OFF==FAIL)
      break;
     
  }

 puts("Simple THief Get ,set to OFF");
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
     puts("serialboot: não foi possivel abrir a porta ");
     return -1;
    }
    
    if(tcgetattr(fd, &toptions) < 0) 
    {
     puts("serialboot: nao foi possivel pegar atributos do terminal");
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
     puts("serialboot: nao foi possivel adicionar atributos no term erro 1");
     return -1;
    }

 return fd;
}

void webcam(const char* file)
{
 CvCapture *capture = cvCreateCameraCapture(0);
   if(capture==NULL)
    puts( "nao he possivel pegar foto!\n");

  IplImage* frame = cvQueryFrame( capture );
   if(!frame)
    puts( "erro na foto!\n");

  int imageParams[3] = {CV_IMWRITE_JPEG_QUALITY, 100, 0};
  cvSaveImage(file , frame,imageParams);
  cvReleaseCapture( &capture );
}
