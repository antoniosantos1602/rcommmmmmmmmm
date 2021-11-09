/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_UA 0x07
#define C_SET 0x03

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


//    while (STOP==FALSE) {       /* loop for input */
//      res = read(fd,buf,1);   /* returns after 5 chars have been input */
//      buf[res]=0;               /* so we can printf... */
//      printf(":%s:%d\n", buf, res);
//      if (buf[res-1]==0) STOP=TRUE;
//    }

    int count = 0;
    unsigned char set[5];
    while (STOP==FALSE) {
        res = read(fd, buf, 1);
        printf("a");
        buf[res] = 0;
        set[count] = res;
        printf("Received %d byte: %02X\n", res, buf[0]);
        set[count] = buf[0];
        count++;
        if (count == 5) STOP=TRUE;
    }
    
    unsigned char ua[5];
    ua[0] = FLAG;
    ua[1] = A_RECEIVER;
    ua[2] = C_UA;
    unsigned char bcc_ua = C_UA ^ A_RECEIVER;
    ua[3] = bcc_ua;
    ua[4] = FLAG;

    if (set[3] == set[1] ^ set[2]) {
        res = write(fd, ua, 5);
        printf("Sent UA");
    }



  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */

    
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
