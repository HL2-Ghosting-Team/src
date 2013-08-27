// Includes necessary for socket programming
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string>

// Globals
int sock;
struct sockaddr_in server_addr;
char connected = 0;

// Setup function, should be run first. Sets up socket.
void SetupClient(char* ip, int port)
{
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
	
}

// Quick hand for sending structures
void SendPacket(char * to_send, int size)
{
	sendto(sock, to_send, size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
}

// Make string packet. Since a lot of packets are just strings
char* MakeString(char info_bit, char * string)
{
	char * tosend = (char *)malloc(sizeof(string))+1;
	*tosend = info_bit;
	int i;
	for (i = 0; i < strlen(string); i++)
	{
		*(tosend+1+i) = *(string+i);
	}
	return tosend;
}

// Sends connect packet
void Connect(char * password)
{
	SendPacket(MakeString('\0', password), strlen(password)+1);
}

// Sends a map change
void MapChange(char * map)
{
	SendPacket(MakeString(1, map), strlen(map)+1);
}

// Sends a model and name name change
void ModelAndNameChange(char * model, char * name)
{
	char * first = MakeString(2, model);
	char * second = MakeString('\n', name);
	
	char * tosend = (char *)malloc(strlen(first)+strlen(second));
	int i;
	for (i = 0; i < strlen(first); i++)
	{
		*(tosend+i) = *(first+i);
	}
	for (i = 0; i < strlen(second); i++)
	{
		*(tosend+strlen(first)+i) = *(second+i);
	}

	SendPacket(tosend, strlen(model)+strlen(name)+2);
}

// Sends position
void SendPos(float x, float y, float z)
{
	char * tosend = (char *)malloc(1+(sizeof(float)*3));
	char * bytesX = (char *)malloc(sizeof(float));
	char * bytesY = (char *)malloc(sizeof(float));
	char * bytesZ = (char *)malloc(sizeof(float));
	memcpy(bytesX, &x, sizeof (float));
	memcpy(bytesY, &y, sizeof (float));
	memcpy(bytesZ, &z, sizeof (float));
	*tosend = 128;

	int i;
	for (i = 0; i < 4; i++)
	{
		*(tosend+1+i) = *(bytesX+i);
	}
	for (i = 0; i < 4; i++)
	{
		*(tosend+5+i) = *(bytesY+i);
	}
	for (i = 0; i < 4; i++)
	{
		*(tosend+9+i) = *(bytesZ+i);
	}

	SendPacket(tosend, 1+(sizeof(float)*3));
}

// Testbed
int main()
{
	SetupClient((char *)"127.0.0.1", 6969);
	Connect("godiebarney");
	MapChange("d1_coast_04");
	ModelAndNameChange("whatever", "DrKabob");
	SendPos(128.4, -34.23, -64.32);
	return 0;
}