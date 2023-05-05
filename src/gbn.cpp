#include "../include/simulator.h"
#include <iostream>
#include "../include/simulator.h"
#include <queue>
#include <cstring>
#include <cstdio>   


std::vector<struct pkt> packetList;
std::vector<struct msg> messageList;
int winSize;
int baseSeqNum;
int nextSeqNum;
int seqNumB;



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
/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    seqNumB = 1;
}

bool isValidPackAtB(pkt pack){
    if(pack.seqnum == seqNumB){
        return true;
    }
    return false;
}

bool validateChecksum(pkt packet, int checksum){

    if(packet.checksum == checksum){
        return true;
    }
    return false;
    
}


int calcChecksum(int seq, int ack, char *message, int n){
    
    int sum = 0;
    for(int i =0; i<n; i++){
        sum+= (int)message[i];
    }
    sum += seq + ack;
    return sum;
}
void A_output(struct msg message)
{
    messageList.push_back(message);
    int max_seq_num = baseSeqNum + winSize;
    while(nextSeqNum < max_seq_num && nextSeqNum < messageList.size()) {
    pkt next_pkt;
    next_pkt.seqnum= nextSeqNum;
    next_pkt.acknum= 0;
    strcpy(next_pkt.payload, message.data);
    next_pkt.checksum= calcChecksum(next_pkt.seqnum,next_pkt.acknum, next_pkt.payload, strlen(next_pkt.payload));

    packetList.push_back(next_pkt);
    tolayer3(0, next_pkt);
		
		if(baseSeqNum == nextSeqNum){
			//cout<<"starting timeout in send data for : "<<next_seq_num<<endl;
			starttimer(0, 20);
		}
		
		nextSeqNum++;
    }
}


/* called from layer 3, when a packet arrives for layer 4 */
void A_input(pkt packet)
{
    int packChecksum= calcChecksum(packet.seqnum, packet.acknum, packet.payload,strlen(packet.payload));
    if(validateChecksum(packet, packChecksum) == true && packet.acknum >= baseSeqNum && packet.acknum < nextSeqNum){
        baseSeqNum = packet.acknum + 1;
		
		if(baseSeqNum == nextSeqNum)
			stoptimer(0);
		else{
			starttimer(0, 20);
		}
    }
    
}


/* called when A's timer goes off */
void A_timerinterrupt()
{
    int max_seq_num = baseSeqNum + winSize;
   // pkt packA= packetList[nextSeqNum];
    //tolayer3(0,packA);
    nextSeqNum= baseSeqNum;
    while(nextSeqNum < max_seq_num && nextSeqNum < packetList.size()) {
    

        tolayer3(0, packetList[nextSeqNum]);
		
		if(baseSeqNum == nextSeqNum){
			//cout<<"starting timeout in send data for : "<<next_seq_num<<endl;
			starttimer(0, 20);
		}
		
		nextSeqNum++;
    }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    winSize= getwinsize();
    baseSeqNum=0;
    nextSeqNum=0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    bool isPktValid= validateChecksum(packet, calcChecksum(packet.seqnum, packet.acknum, packet.payload, strlen(packet.payload)));
    bool expValueAtB= isValidPackAtB(packet);

    if(isPktValid && expValueAtB){
        tolayer5(1, packet.payload);
        pkt pktAck;
        pktAck.seqnum= seqNumB;
        pktAck.acknum= 0;
        pktAck.checksum= calcChecksum(pktAck.seqnum,pktAck.acknum, pktAck.payload, strlen(pktAck.payload));
        tolayer3(1, pktAck);
        seqNumB++;
    }
    
}




