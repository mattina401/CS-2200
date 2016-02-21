#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rtp.h"

/* GIVEN Function:
 * Handles creating the client's socket and determining the correct
 * information to communicate to the remote server
 */
CONN_INFO* setup_socket(char* ip, char* port){
	struct addrinfo *connections, *conn = NULL;
	struct addrinfo info;
	memset(&info, 0, sizeof(struct addrinfo));
	int sock = 0;

	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_DGRAM;
	info.ai_protocol = IPPROTO_UDP;
	getaddrinfo(ip, port, &info, &connections);

	/*for loop to determine corr addr info*/
	for(conn = connections; conn != NULL; conn = conn->ai_next){
		sock = socket(conn->ai_family, conn->ai_socktype, conn->ai_protocol);
		if(sock <0){
			if(DEBUG)
				perror("Failed to create socket\n");
			continue;
		}
		if(DEBUG)
			printf("Created a socket to use.\n");
		break;
	}
	if(conn == NULL){
		perror("Failed to find and bind a socket\n");
		return NULL;
	}
	CONN_INFO* conn_info = malloc(sizeof(CONN_INFO));
	conn_info->socket = sock;
	conn_info->remote_addr = conn->ai_addr;
	conn_info->addrlen = conn->ai_addrlen;
	return conn_info;
}

void shutdown_socket(CONN_INFO *connection){
	if(connection)
		close(connection->socket);
}

/* 
 * ===========================================================================
 *
 *			STUDENT CODE STARTS HERE. PLEASE COMPLETE ALL FIXMES
 *
 * ===========================================================================
 */


/*
 *  Returns a number computed based on the data in the buffer.
 */
static int checksum(char *buffer, int length){


	int i = 0;
	int sum = 0;

	while(i < length) {
		sum = sum + (int) buffer[i];
		i++;
	}

	return sum;

	/*  ----  FIXME  ----
	 *
	 *  The goal is to return a number that is determined by the contents
	 *  of the buffer passed in as a parameter.  There a multitude of ways
	 *  to implement this function.  For simplicity, simply sum the ascii
	 *  values of all the characters in the buffer, and return the total.
	 */ 
}

/*
 *  Converts the given buffer into an array of PACKETs and returns
 *  the array.  The value of (*count) should be updated so that it 
 *  contains the length of the array created.
 */
static PACKET* packetize(char *buffer, int length, int *count){

	PACKET* packets;
	
   	int num_of_packets = 0;
   	num_of_packets = length / MAX_PAYLOAD_LENGTH;
   	
	if (length % MAX_PAYLOAD_LENGTH != 0) 
	{
        	num_of_packets++;
    	}
    	packets = malloc(sizeof(PACKET) * num_of_packets);
    	*count = num_of_packets;


	int i = 0;

	while (i < num_of_packets) {

		if(i != num_of_packets - 1) {
			memcpy(packets[i].payload, &buffer[i * MAX_PAYLOAD_LENGTH],MAX_PAYLOAD_LENGTH);
			packets[i].type = DATA;
			packets[i].payload_length = MAX_PAYLOAD_LENGTH;
        		packets[i].checksum = checksum(packets[i].payload, packets[i].payload_length);
		} else {
			memcpy(packets[i].payload, &buffer[i * MAX_PAYLOAD_LENGTH],length % MAX_PAYLOAD_LENGTH);
			packets[i].type = LAST_DATA;
			packets[i].payload_length = length % MAX_PAYLOAD_LENGTH;
        		packets[i].checksum = checksum(packets[i].payload, packets[i].payload_length);
		}

		i++;

	}


    	return packets;

	
	
		



	/*  ----  FIXME  ----
	 *  The goal is to turn the buffer into an array of packets.
	 *  You should allocate the space for an array of packets and
	 *  return a pointer to the first element in that array.  Each 
	 *  packet's type should be set to DATA except the last, as it 
	 *  should be LAST_DATA type. The integer pointed to by 'count' 
	 *  should be updated to indicate the number of packets in the 
	 *  array.
	 */ 
}

/*
 * Send a message via RTP using the connection information
 * given on UDP socket functions sendto() and recvfrom()
 */
int rtp_send_message(CONN_INFO *connection, MESSAGE*msg){


	int count = 0;

	PACKET* packets = packetize(msg -> buffer, msg -> length, &count);
	

	PACKET* buffer;
	buffer = malloc(sizeof(PACKET));

	int i =0;
	
	while (i < count) {
		sendto(connection -> socket, &packets[i], sizeof(PACKET), 0, connection -> remote_addr, connection -> addrlen);
	 	recvfrom(connection -> socket, buffer, sizeof(PACKET), 0, NULL, NULL);
	 	PACKET *p = buffer;
	 	if (p -> type == NACK) {
			i--;
		}
	
		i++;
	}


	 return 1;


	/* ---- FIXME ----
	 * The goal of this function is to turn the message buffer
	 * into packets and then, using stop-n-wait RTP protocol,
	 * send the packets and re-send if the response is a NACK.
	 * If the response is an ACK, then you may send the next one
	 */
}

/*
 * Receive a message via RTP using the connection information
 * given on UDP socket functions sendto() and recvfrom()
 */
MESSAGE* rtp_receive_message(CONN_INFO *connection){

	int chksum;
	MESSAGE* message;
	message = malloc(sizeof(MESSAGE));
	PACKET* buffer;
	buffer = malloc(sizeof(PACKET));
	
	do {
		recvfrom(connection -> socket, buffer, sizeof(PACKET), 0, NULL, NULL);
		if(buffer -> type == DATA || buffer -> type == LAST_DATA) {
			PACKET* p = (PACKET*)buffer;
			PACKET* response = malloc(sizeof(PACKET));
			chksum = checksum(p -> payload, p -> payload_length);
			if(p->checksum == chksum) {
				char* new_buffer = malloc((sizeof(char)*p -> payload_length) + sizeof(char) * message -> length);
				memcpy(new_buffer, message -> buffer, message -> length);
				memcpy(new_buffer + message -> length, p -> payload, p -> payload_length);
				message -> length = message -> length + p -> payload_length;
				message -> buffer = new_buffer;
				response -> type = ACK;
			} else {
				response -> type = NACK;
				if(p -> type == LAST_DATA) {
					p -> type = DATA;	
				}	
					
			}
		sendto(connection -> socket, response, sizeof(PACKET), 0, connection -> remote_addr, (socklen_t)connection -> addrlen);
		}
	} while(buffer -> type != LAST_DATA);

	return message;




	/* ---- FIXME ----
	 * The goal of this function is to handle 
	 * receiving a message from the remote server using
	 * recvfrom and the connection info given. You must 
	 * dynamically resize a buffer as you receive a packet
	 * and only add it to the message if the data is considered
	 * valid. The function should return the full message, so it
	 * must continue receiving packets and sending response 
	 * ACK/NACK packets until a LAST_DATA packet is successfully 
	 * received.
	 */
}
