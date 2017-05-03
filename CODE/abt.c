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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int cksum = 0;
int sum;
int nleft;

struct pkt packet;

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
  printf("The result of checksum is %04x\n",cksum);
  return cksum;
}

//return 1: No corrupt of the packet
//reutrn 0: Corruption
int valiate_checksum(packet)
struct pkt packet;
{
  sum = 0;
  // printf("Check seqnum: %d\n", packet.seqnum);
  // printf("Check acknum: %d\n", packet.acknum);
  // printf("Check payload: %s\n", packet.payload);
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
/* called from layer 5, passed the data to be sent to other side */

/*where message is a structure of type msg, containing data to be sent to the B-side. 
This routine will be called whenever the upper layer at the sending side (A) has a 
message to send. It is the job of your protocol to insure that the data in such a 
message is delivered in-order, and correctly, to the receiving side upper layer.
*/

int pkt_num=0;
float TIMEOUT=10;
int rcv_state = 0;

void A_output(message)
  struct msg message;
{
  if(rcv_state == 1)//if get message during the wait time, do nothing
  {
    //make packet
      strcpy(packet.payload, message.data); //payload
      printf("Content of payload: %s\n", packet.payload);
      // printf("checksum of packet make : %d\n", packet.checksum);
      packet.seqnum = pkt_num;
      packet.acknum = 0;
      packet.checksum=checksum(packet);
      tolayer3(0,packet);
      starttimer(0,TIMEOUT);
      rcv_state = 0;
      // if(TIMEOUT==0&&(rcv_state == 0))
      //   A_timerinterrupt();
  }else{}
  
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if(TIMEOUT!=0){
    printf("Check seqnum: %d\n", packet.seqnum);
    printf("Check acknum: %d\n", packet.acknum);
    printf("Check payload: %s\n", packet.payload);
    if(valiate_checksum(packet)==1){
      if(packet.acknum == pkt_num){
        pkt_num = (pkt_num+1)%2;
        rcv_state = 1;
        stoptimer(0);
      }
    }
  }
  
}

/* called when A's timer goes off */
/*This routine will be called when A's timer expires (thus generating a timer interrupt). 
You'll probably want to use this routine to control the retransmission of packets. 
See starttimer() and stoptimer() below for how the timer is started and stopped.*/

void A_timerinterrupt()
{
  printf("Content of retransmit payload: %s\n", packet.payload);
  printf("Content of retransmit seq: %d\n", packet.seqnum);
  printf("Content of retransmit checksum: %d\n", packet.checksum);
  tolayer3(0,packet);
  starttimer(0,TIMEOUT);
  // if(TIMEOUT==0&&(rcv_state == 0))
  //   A_timerinterrupt();
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  //window size, sequence number
  packet.seqnum = 0;
  packet.acknum = 0;
  packet.checksum = 0;
  pkt_num = 0;
  rcv_state = 1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
/*where packet is a structure of type pkt. This routine will be called whenever a packet 
sent from the A-side (as a result of a tolayer3() being called by a A-side procedure) 
arrives at the B-side. packet is the (possibly corrupted) packet sent from the A-side.*/

int expect_pkt_num=0;

void B_input(packet)
  struct pkt packet;
{
  printf("Content of recv packet: %s\n", packet.payload);
  // checksum(packet);
  if(valiate_checksum(packet)==1)
  {
    printf("Recsequence number: %d expect_pkt_num: %d\n", packet.seqnum, expect_pkt_num);
    if(packet.seqnum == expect_pkt_num){
      printf("Content of recv packet: %s\n", packet.payload);

      tolayer5(1,packet.payload);
      // expect_pkt_num++;
      packet.acknum = packet.seqnum;
      memset(packet.payload,0,20);
      
      packet.checksum = checksum(packet);
      tolayer3(1,packet);
      expect_pkt_num = (expect_pkt_num+1)%2;
    }
    else{
      packet.acknum = packet.seqnum;
      memset(packet.payload,0,20);
      packet.checksum = checksum(packet);
      tolayer3(1,packet);
    }
  }else{
    packet.acknum = packet.seqnum;
    memset(packet.payload,0,20);
    packet.checksum = checksum(packet);
    tolayer3(1,packet);
  }
    
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  // expect_pkt_num = 0;
}
