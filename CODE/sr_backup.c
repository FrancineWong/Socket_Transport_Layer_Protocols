#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int cksum = 0;
int sum;
int nleft;
int checksum(packet)
struct pkt packet;
{
  sum = 0;
  sum = (packet.acknum&0xffff)+(packet.acknum>>16) + (packet.seqnum&0xffff)+(packet.seqnum>>16);
  // printf("Sum of the header: %04x\n", sum);

  for(nleft=0; nleft<20; nleft+=2)
  {
    sum += packet.payload[nleft]+(packet.payload[nleft+1]<<8);
    // printf("Sum %d of buffer is: %04x\n",nleft,sum);
  }

  sum = (sum>>16)+(sum&0xffff);
  // printf("The result of sum is %04x\n",sum);
  cksum = ~sum;
  // printf("The result of checksum is %04x\n",cksum);
  return cksum;
}
/***********************************************************************************************/
//return 1: No corrupt of the packet
//reutrn 0: Corruption
int valiate_checksum(packet)
struct pkt packet;
{
  sum = 0;
  sum = (packet.acknum&0xffff)+(packet.acknum>>16) + (packet.seqnum&0xffff)+(packet.seqnum>>16) + (packet.checksum&0xffff)+(packet.checksum>>16);
  for(nleft = 0; nleft<20; nleft +=2)
  {
    sum += packet.payload[nleft]+(packet.payload[nleft+1]<<8);
    // printf("Sum %d of buffer is: %04x\n",nleft,sum);
  }
  sum = (sum>>16)+(sum&0xffff);
  sum = ~sum;
  sum = sum&0xffff;
  printf("valiate_checksum is: %04x\n",sum);
  return sum; 
}
/***********************************************************************************************/
//Multiple timer setting
#define TRUE 1
#define FALSE 0

#define MAX_TIMER_NUM 500

int timer_num;
struct timer
{
  int inuse; //flag for call the timer in use
  float counter; //timer out 
  int number; //corresponding to the send_base, every packet has a timer
}timers[MAX_TIMER_NUM];

void timers_init(){
  struct  timer *t;
  for(t=timers;t<&timers[MAX_TIMER_NUM];t++){
    t->inuse = FALSE;
    t->counter = 0;
  }
}

/***********************************************************************************************/
struct buf
{
  char BUFFER[20];
  int ACKed;
}BUFFER[1200];

void bufinit(){
  struct buf *b;
  for(b=BUFFER;b<&BUFFER[1200];b++){
    b->ACKed = 0;
  }
}

int next_seq_num;
int send_base;
int buffer_size;
int N;//window size

struct pkt packet;
float TIMEOUT = 1;
#define COUNTER 10

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  strcpy(BUFFER[buffer_size].BUFFER,message.data);
  printf("BUFFER content: %s\n", BUFFER[buffer_size].BUFFER);
  buffer_size++;
  if(next_seq_num<=buffer_size){
    if(next_seq_num < (send_base + N)){
      while(strcmp(BUFFER[next_seq_num].BUFFER,"")){//if equal, return 0
        packet.seqnum = next_seq_num;
        packet.acknum = 0;
        memset(packet.payload,0,20);
        // printf("Payload content: %s\n", packet.payload);
        memcpy(packet.payload, BUFFER[next_seq_num].BUFFER,20);
        printf("Send seqnum: %d\n",packet.seqnum);
        printf("Payload contentA_: %.*s\n",20, packet.payload);
        packet.checksum = checksum(packet);
        tolayer3(0,packet);
        //start timer for every send packet
        timer_num = next_seq_num%N;
        timers[timer_num].inuse = TRUE;
        timers[timer_num].counter = COUNTER;
        next_seq_num++;
      }    
    }
  }
}
/***********************************************************************************************/
/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if(valiate_checksum(packet)==1){//no corruption of ACK
    printf("packetnum: %d next_seq_num: %d\n", packet.seqnum, next_seq_num);
    if(packet.acknum>=send_base){
      BUFFER[packet.acknum].ACKed = 1;
      if(packet.acknum == send_base){
        while(BUFFER[send_base].ACKed ==1){
          timer_num = send_base%N;
          if(timers[timer_num].counter!=0){
            // timer_num = (next_seq_num-1+N)%N;
            timers[timer_num].counter = 0;//works like stop the simulate timer
            timers[timer_num].inuse = FALSE;
            send_base++;
            printf("send_base: %d next_seq_num: %d\n", send_base, next_seq_num);
          }  
        }
        
      }else{
        BUFFER[packet.acknum].ACKed = 1;
      }
    }

  }
}
/***********************************************************************************************/
/* called when A's timer goes off */
void A_timerinterrupt()
{
  // starttimer(0,TIMEOUT);
  struct timer *t;
  for(t=timers;t<&timers[MAX_TIMER_NUM];t++){
    if(t->inuse){
      if(t->counter!=0){//unexpired
       t->counter--;
      }
      if(t->counter <= 0){//expired
        // if(send_base != packet.seqnum){//no ACK received 
          //do retransmission
          packet.seqnum = send_base;
          memset(packet.payload,0,20);
          memcpy(packet.payload,BUFFER[send_base].BUFFER,20);
          // printf("BUFFER: %s\n", BUFFER[i]);
          printf("retransmission payload: %.*s\n",20, packet.payload);
          packet.checksum = checksum(packet);
          tolayer3(0,packet);
          t->counter = COUNTER; //restart the corresponding timer
          t->inuse = TRUE;
        // }
      }
    }
  }
  starttimer(0,TIMEOUT);
}  
/***********************************************************************************************/
/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  N = getwinsize();
  starttimer(0,TIMEOUT);
  timers_init();
  bufinit();
  next_seq_num = 0;
  send_base = 0;
  buffer_size = 0;
}
/***********************************************************************************************/
/* Note that with simplex transfer from a-to-B, there is no B_output() */
char BUFFER_B[1200][20];
int buffer_b_size;
int rcv_base;
int expect_seq_num;
/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  printf("Rcv seqnum: %d\n", packet.seqnum);
  if(valiate_checksum(packet)==1){//notcorrpt
    memcpy(BUFFER_B[packet.seqnum],packet.payload,20);
    printf("rcv payload: %.*s\n", 20,packet.payload);
    printf("rcv_base: %d\n", rcv_base);
    printf("BUFFER_B[rcv_base]: %.*s\n", 20,BUFFER_B[packet.seqnum]);
    if(packet.seqnum <= buffer_size){
      if(packet.seqnum>=rcv_base){
        if(packet.seqnum == rcv_base){
          while(strcmp(BUFFER_B[rcv_base],"")){//if gap filled, move on to deliver all avalibale payload
            //if buffer equal to null, return 0
            tolayer5(1,BUFFER_B[rcv_base]);
            packet.acknum = packet.seqnum;
            memset(packet.payload,0,20);
            packet.checksum = checksum(packet);
            tolayer3(1,packet);
            rcv_base++;
            printf("rcv_base: %d\n", rcv_base);
            // expect_seq_num++;
          }
        }
      }else{//duplicate ack, has been acked 
        packet.acknum = packet.seqnum;
        memset(packet.payload,0,20);
        packet.checksum = checksum(packet);
        tolayer3(1,packet);
      }
      
    }
    
  }//corrupt, do nothing

}
/***********************************************************************************************/
/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  N = getwinsize();
  buffer_b_size = 0;
  rcv_base = 0;
  expect_seq_num = 0;
}
