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

/* called from layer 5, passed the data to be sent to other side */

/*where message is a structure of type msg, containing data to be sent to the B-side. 
This routine will be called whenever the upper layer at the sending side (A) has a 
message to send. It is the job of your protocol to insure that the data in such a 
message is delivered in-order, and correctly, to the receiving side upper layer.
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// int cksum = 0;
// int sum;
// int nleft;
// int checksum(packet)
// struct pkt packet;
// {
//   sum = 0;
//   sum = (packet.acknum&0xffff)+(packet.acknum>>16) + (packet.seqnum&0xffff)+(packet.seqnum>>16);
//   for(nleft=0; nleft<20; nleft+=2)
//   {
//     sum += packet.payload[nleft]+(packet.payload[nleft+1]<<8);
//   }
//   sum = (sum>>16)+(sum&0xffff);
//   cksum = ~sum;
//   return cksum;
// }
// int valiate_checksum(packet)
// struct pkt packet;
// {
//   sum = 0;
//   sum = (packet.acknum&0xffff)+(packet.acknum>>16) + (packet.seqnum&0xffff)+(packet.seqnum>>16) + (packet.checksum&0xffff)+(packet.checksum>>16);
//   for(nleft = 0; nleft<20; nleft +=2)
//   {
//     sum += packet.payload[nleft]+(packet.payload[nleft+1]<<8);
//   }
//   sum = (sum>>16)+(sum&0xffff);
//   sum = ~sum;
//   sum = sum&0xffff;
//   printf("valiate_checksum is: %04x\n",sum);
//   return sum; 
// }
/***************************************************************************************************/
int checksum(struct pkt packet);
int valiate_checksum(struct pkt packet);
/**************************************************************************************************/
char BUFFER[1200][20];
int buffer_size;
int next_seq_num;//smallest unused sequence number
int send_base;//sequence number of the oldest unackknolewdged packet
int N;

float TIMEOUT = 15;
struct pkt packet;
void A_output(message)
  struct msg message;
{
  memcpy(BUFFER[buffer_size],message.data,20);
  buffer_size++;
  printf("buffer_size: %d\n", buffer_size);
  if(next_seq_num<=buffer_size){//the content has to be valid
    if(next_seq_num<send_base+N){
      while(strcmp(BUFFER[next_seq_num],"")){
        packet.seqnum = next_seq_num;
        packet.acknum = 0;
        memcpy(packet.payload, BUFFER[next_seq_num],20);
        packet.checksum = checksum(packet);
        tolayer3(0,packet);
        if(send_base == next_seq_num){
          starttimer(0,TIMEOUT);
        }
        next_seq_num++;
      }
    }
    
  }
  

}

/* called from layer 3, when a packet arrives for layer 4 */
/*where packet is a structure of type pkt. This routine will be called whenever a 
packet sent from the B-side (as a result of a tolayer3() being called by a 
B-side procedure) arrives at the A-side. packet is the (possibly corrupted) packet 
sent from the B-side.*/
int j;
void A_input(packet)
  struct pkt packet;
{
  printf("Recv packetnum: %d\n",packet.acknum);
  if(valiate_checksum(packet)==1){
    if(packet.seqnum>=send_base){
      if(packet.seqnum == send_base){
        send_base = packet.acknum+1;
        
        printf("send_base: %d\n", send_base);
        printf("next_seq_num: %d\n", next_seq_num);
      }
      if(send_base == next_seq_num){
        stoptimer(0);
      }else{
        // starttimer(0,TIMEOUT);
        if(next_seq_num<buffer_size){
          if(buffer_size<=send_base+N-1){
            if(next_seq_num>=send_base){
              starttimer(0,TIMEOUT);
              printf("send_base: %d\n", send_base);
              for(j=send_base;j<buffer_size;j++){
                packet.acknum =0;
                packet.seqnum = j;
                memset(packet.payload,0,20);
                memcpy(packet.payload,BUFFER[j],20);
                packet.checksum = checksum(packet);
                tolayer3(0,packet);
              }
              // if(send_base == next_seq_num){
              //   starttimer(0,TIMEOUT);
              // }
              next_seq_num = buffer_size;
              printf("next_seq_num: %d\n", next_seq_num);
            }
          }
          if(buffer_size>send_base+N-1){
            if(next_seq_num>=send_base){
              printf("send_base: %d\n", send_base);
              starttimer(0,TIMEOUT);
              for(j=send_base;j<send_base+N;j++){
                packet.acknum = 0;
                packet.seqnum = j;
                memset(packet.payload,0,20);
                memcpy(packet.payload,BUFFER[j],20);
                packet.checksum = checksum(packet);
                tolayer3(0,packet);
              }
              // if(send_base == next_seq_num){
              //   starttimer(0,TIMEOUT);
              // }
              next_seq_num = send_base+N;
              printf("next_seq_num: %d\n", next_seq_num);
            }
          }
        }
      }

    }
    

/****************************************************************************/ 
//manage transmission of  
/****************************************************************************/
    // if(next_seq_num<=buffer_size){
    //   if(next_seq_num>=send_base){
    //     if(next_seq_num<send_base+N){
    //       packet.seqnum = next_seq_num-1;
    //       packet.acknum = 0;
    //       memcpy(packet.payload, BUFFER[next_seq_num-1],20);
    //       packet.checksum = checksum(packet);
    //       tolayer3(0,packet);
    //       starttimer(0,TIMEOUT);
    //       next_seq_num++;
    //     }
    //     // if(send_base == next_seq_num){
    //     //   starttimer(0,TIMEOUT);
    //     //   next_seq_num++;
    //     // }
    //   }
    // }
/**************************************************************************/    
    // if(next_seq_num<send_base+N){
    //   for(i = send_base; i < next_seq_num; i++){
    //     packet.seqnum = i;
    //     memset(packet.payload,0,20);
    //     memcpy(packet.payload,BUFFER[i],20);
    //     packet.checksum = checksum(packet);
    //     tolayer3(0,packet);
    //   }
    // }
    // next_seq_num = next_seq_num + //发送上线更新seq_num
 /**************************************************************************/ 
    
    // if(next_seq_num<buffer_size){
    //   if(buffer_size<=send_base+N-1){
    //     if(next_seq_num>=send_base){
    //       // starttimer(0,TIMEOUT);
    //       printf("send_base: %d\n", send_base);
    //       for(j=send_base;j<buffer_size;j++){
    //         packet.acknum =0;
    //         packet.seqnum = j;
    //         memset(packet.payload,0,20);
    //         memcpy(packet.payload,BUFFER[j],20);
    //         packet.checksum = checksum(packet);
    //         tolayer3(0,packet);
    //       }
    //       if(send_base == next_seq_num){
    //         starttimer(0,TIMEOUT);
    //       }
    //       next_seq_num = buffer_size;
    //       printf("next_seq_num: %d\n", next_seq_num);
    //     }
    //   }
    //   if(buffer_size>send_base+N-1){
    //     if(next_seq_num>=send_base){
    //       printf("send_base: %d\n", send_base);
    //       // starttimer(0,TIMEOUT);
    //       for(j=send_base;j<send_base+N;j++){
    //         packet.acknum = 0;
    //         packet.seqnum = j;
    //         memset(packet.payload,0,20);
    //         memcpy(packet.payload,BUFFER[j],20);
    //         packet.checksum = checksum(packet);
    //         tolayer3(0,packet);
    //       }
    //       if(send_base == next_seq_num){
    //         starttimer(0,TIMEOUT);
    //       }
    //       next_seq_num = send_base+N;
    //       printf("next_seq_num: %d\n", next_seq_num);
    //     }
    //   }
    // }
    
  }//valiate checksum
  
}

/* called when A's timer goes off */
/*This routine will be called when A's timer expires (thus generating a timer interrupt). 
You'll probably want to use this routine to control the retransmission of packets. 
See starttimer() and stoptimer() below for how the timer is started and stopped.*/
void A_timerinterrupt()
{
  int i;
  /*********when time out, retransmit all unacked packets**********/
  // if(next_seq_num<=buffer_size){
    // if(next_seq_num<send_base+N){
      starttimer(0,TIMEOUT);
      for(i = send_base; i < next_seq_num; i++){
        packet.seqnum = i;
        memset(packet.payload,0,20);
        memcpy(packet.payload,BUFFER[i],20);
        // printf("BUFFER: %s\n", BUFFER[i]);
        printf("retransmission payload: %.*s\n",20, packet.payload);
        packet.checksum = checksum(packet);
        tolayer3(0,packet);
      }
    // }
  // }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  N = getwinsize();
  next_seq_num = 0;
  send_base = 0;
  buffer_size = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
int expect_seq_num;
void B_input(packet)
  struct pkt packet;
{
  printf("Recv Seq: %d\n", packet.seqnum);
  if(valiate_checksum(packet)==1){
    
    printf("expect_seq_num: %d\n", expect_seq_num);
    if(packet.seqnum>=expect_seq_num){//make sure seqnum inorder
      if(packet.seqnum == expect_seq_num){//make sure the recv window can move
        printf("Recv payload: %.*s\n", 20,packet.payload);
        tolayer5(1,packet.payload);
        memset(packet.payload,0,20);
        packet.acknum = expect_seq_num;
        packet.checksum = checksum(packet);
        printf("ACK send back: %d\n", expect_seq_num);
        tolayer3(1,packet);
        expect_seq_num++;
      }else{
        // struct pkt packet;
        packet.acknum = expect_seq_num-1;
        packet.seqnum = expect_seq_num-1;
        memset(packet.payload,0,20);
        packet.checksum = checksum(packet);
        tolayer3(1,packet);
      }
    }else{
      // struct pkt packet;
      // packet.acknum = expect_seq_num-1;
      // packet.seqnum = expect_seq_num-1;
      packet.acknum = packet.seqnum;
      memset(packet.payload,0,20);
      packet.checksum = checksum(packet);
      tolayer3(1,packet);
    }
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  expect_seq_num = 0;
}

int checksum(packet)
struct pkt packet;
{
  int nleft;
    int checksum;
    checksum = packet.acknum + packet.seqnum;
    for(nleft=0; nleft<20; nleft+=2)
    {
        checksum += packet.payload[nleft]+(packet.payload[nleft+1]<<8);
    }
    return checksum;
}
int valiate_checksum(packet)
struct pkt packet;
{
    return (packet.checksum == checksum(packet));
}
