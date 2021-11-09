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
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_UA 0x07
#define C_SET 0x03

volatile int STOP = FALSE;
//int fd;

int llopen(char *port_name)
{
  int fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0)
  {
    perror(port_name);
    exit(-1);
  }
  return fd;
}

int llread(int fd, unsigned char flag, int timeout_secs, unsigned char *packet)
{
  int flag_count = 0;
  int res = 0;
  int count = 0;
  unsigned char buf[1];
  while (flag_count < 2)
  {
    while (res < 1 || (timeout_secs > 0))
      res = read(fd, buf, 1);
    packet[count] = buf[0];
    printf("Received %d byte: %02X\n", res, buf[0]);
    if (packet[count] == flag)
      flag_count++;
    count++;
  }
}

int write_set(int fd)
{
  unsigned char set[5];
  set[0] = FLAG;
  set[1] = A_SENDER;
  set[2] = C_SET;
  unsigned char bcc_set = C_SET ^ A_SENDER;
  set[3] = bcc_set;
  set[4] = FLAG;

  int res = write(fd, set, 5);
  printf("%d bytes written\n", res);

  return 0;
}

int llwrite(int fd, unsigned char *data, int idle_secs, int retries)
{
  int count = 0, read_count = 0, stop = 0, res = 0;
  unsigned char buf[1];
  unsigned char packet[5];
  while (count < retries || stop)
  {
    write_set(fd);
    alarm(idle_secs);
    read_count = 0;
    while (!stop)
    {
      res = read(fd, buf, 1);
      if(res){
        packet[read_count] = buf[0];
        printf("Received %d byte: %02X\n", res, buf[0]);
      }
      read_count += res;
      if (read_count == 5)
      {
        read_count = 0;
        if (packet[3] == packet[1] ^ packet[2])
          stop = 1;
      }
    }
    count++;
  }

  

  

  return 0;
}

//char bcc_calculator()

int main(int argc, char **argv)
{
  int c, res;
  struct termios oldtio, newtio;
  unsigned char buf[255];
  int i, sum = 0, speed = 0;

  (void)signal(SIGALRM, );

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0)))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  /*
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }
    */
  int fd = llopen(argv[1]);

  if (tcgetattr(fd, &oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  //sleep(1);
  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  /*
    for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }
    */

  /*testing*/
  //buf[25] = '\n';

  /*
    buf[0] = 'O';
    buf[1] = 'l';
    buf[2] = 'a';
    buf[3] = '\0';*/

  /* MESSAGE
    if(fgets(buf, 255, stdin) == NULL){
        printf("Gets Error!\n");
        return 1;
    }
    
    
    res = write(fd,buf, strlen(buf) + 1);   
    printf("%d bytes written\n", res);
    */

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

  //void * set = malloc(5);
  unsigned char set[5];
  set[0] = FLAG;
  set[1] = A_SENDER;
  set[2] = C_SET;
  unsigned char bcc_set = C_SET ^ A_SENDER;
  set[3] = bcc_set;
  set[4] = FLAG;
  printf("BCC SET: %02X\n", bcc_set);

  unsigned char ua[5];
  ua[0] = FLAG;
  ua[1] = A_RECEIVER;
  ua[2] = C_UA;
  unsigned char bcc_ua = C_UA ^ A_RECEIVER;
  ua[3] = bcc_ua;
  ua[4] = FLAG;
  printf("BCC UA: %02X\n", bcc_ua);

  //unsigned int * sett = (unsigned int *) malloc(5);
  //*sett = FLAG | (bcc << 8) | (C_SET << 16) | (A_SENDER << 24) | (FLAG << 32);
  //*sett = FLAG | (bcc << 8) | (C_SET << 16) | (A_SENDER << 24);
  //printf("SETT: %0X\n", *sett);
  //free(sett);

  printf("SET: ");
  for (int i = 0; i < 5; i++)
  {
    printf("%02X", set[i]);
  }
  printf("\n");

  res = write(fd, set, 5);
  printf("%d bytes written\n", res);

  int count = 0;
  unsigned char packet[5];
  STOP = FALSE;
  while (STOP == FALSE)
  {
    res = read(fd, buf, 1);
    buf[res] = 0;
    packet[count] = buf[0];
    printf("Received %d byte: %02X\n", res, buf[0]);
    count++;
    if (count == 5)
      STOP = TRUE;
  }

  printf("Received %d bytes.\n", count);

  if (packet[3] == packet[1] ^ packet[2])
  {
    printf("Correct BCC for UA\n");
  }
  else
  {
    printf("Wrong BCC!!!\n");
  }

  sleep(1);
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
