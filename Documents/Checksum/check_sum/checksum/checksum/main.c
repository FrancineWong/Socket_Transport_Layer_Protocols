//
//  main.c
//  Checksum
//
//  Created by 王敏凡 on 3/24/16.
//  Copyright © 2016 王敏凡. All rights reserved.
//

#include <stdio.h>
#include <string.h>

int main(int argc, const char * argv[]) {
    struct msg{
        char data[20];
    }message;
    
    struct pkt{
        int seqnum;
        int acknum;
        int checksum;
        char payload[20];
    }packet;
    
    int sum = 0;
//    int payloadsize = 19;
    int nleft;
//    = payloadsize;
    int checksum = 0;
    //    int valiate = 10;
    
    packet.acknum = 1;
    packet.seqnum = 1;
    //    packet.checksum = 0;
    memcpy(message.data, "ccccccccccccccccccccHʾ?", strlen("ccccccccccccccccccccHʾ?"));
    memcpy(packet.payload, message.data, strlen(message.data));
    printf("The content of message is: %s\n",message.data);
    printf("The content of message is: %s\n",packet.payload);
    
    sum = (packet.acknum&0xffff)+(packet.acknum>>16) + (packet.seqnum&0xffff)+(packet.seqnum>>16);
    printf("Sum of the header: %04x\n", sum);
    
    for (nleft=0; nleft<strlen("ccccccccccccccccccccHʾ?");nleft+=2 )
    {
        sum += packet.payload[nleft]+(packet.payload[nleft-1]<<8);
        printf("Sum %d of buffer is: %04x\n",nleft,sum);
        nleft-=2;
        //        printf("Sum %d of buffer is: %04x\n",nleft,sum);
    }
    printf("The result of sum is %04x\n",sum);
    
    sum = (sum>>16)+(sum&0xffff);
    sum += (sum>>16);
    
    printf("The result of sum is %04x\n",sum);
    checksum = ~sum;
    printf("The result of checksum is %04x\n",checksum);
    return 0;
}
