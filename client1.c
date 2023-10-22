#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define BUFF_SIZE 255
char buff[BUFF_SIZE];
unsigned int len;
void inputUserName()
{
	printf("Enter to exit\n");
	while (true)
	{

		printf("Insert username: ");
		fgets(buff, BUFF_SIZE, stdin);

		if (buff[0] == '\n')
		{
			printf("Exit programming\n");
			exit(1);
		}
		buff[strlen(buff) - 1] = '\0';

		bool containsSpace = false;
		for (int i = 0; i < strlen(buff); i++)
		{
			if (buff[i] == ' ')
			{
				containsSpace = true;
				break;
			}
		}

		if (containsSpace)
		{
			printf("Username cannot contain spaces. Please try again.\n");
		}
		else
		{
			printf("Username is valid: %s\n", buff);
			break;
		}
	}
}

void inputPassword()
{
	printf("Enter to exit\n");
	while (true)
	{
		printf("Insert password: ");
		fgets(buff, BUFF_SIZE, stdin);

		if (buff[0] == '\n')
		{
			printf("Exit programming\n");
			exit(1);
		}
		buff[strlen(buff) - 1] = '\0';

		bool containsSpace = false;
		for (int i = 0; i < strlen(buff); i++)
		{
			if (buff[i] == ' ')
			{
				containsSpace = true;
				break;
			}
		}

		if (containsSpace)
		{
			printf("Password cannot contain spaces. Please try again.\n");
		}
		else
		{
			printf("Password is valid: %s\n", buff);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int sockfd, rcvBytes, sendBytes;

	struct sockaddr_in servaddr;
	char *serv_IP;
	short serv_PORT;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <ServerIP> [<EchoPort>]\n", argv[0]);
		exit(1);
	}
	serv_IP = argv[1];
	serv_PORT = atoi(argv[2]);

	// Step 1: Construct socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		perror("Error: ");
		return 0;
	}
	// Step 2: Define the address of the server
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(serv_IP);
	servaddr.sin_port = htons(serv_PORT);
	// Step 3: Communicate with server

	int isLogin = 0;
	do
	{
		if (isLogin == 0)
		{
			inputUserName();
			if (buff[0] == '\n')
				exit(1);
			/*	printf("Insert username: ");
					fgets(buff, BUFF_SIZE, stdin);

				buff[strlen(buff) - 1] = '\0';*/
			len = sizeof(servaddr);
			sendBytes = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&servaddr, len);
			rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&servaddr, &len);
			printf("Reply from server: %s\n", buff);
			if (buff[0] == 'C')
				continue;
			if (buff[0] == 'A')
				continue;
			if (sendBytes < 0)
			{
				perror("Error 1");
				return 0;
			}
		}

		while (isLogin == 0)
		{

			inputPassword();

			/*	printf("Insert password: ");
					fgets(buff, BUFF_SIZE, stdin);

					buff[strlen(buff) - 1] = '\0';*/

			len = sizeof(servaddr);
			sendBytes = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&servaddr, len); // send message to server
			if (sendBytes < 0)
			{
				perror("Error 1");
				return 0;
			}
			rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&servaddr, &len);
			printf("Reply from server: %s\n", buff);
			if (strcmp(buff, "OK") == 0)
			{
				isLogin = 1;
			}
			if (buff[0] == 'P')
				break;
			if (buff[0] == 'C')
				break;
			if (buff[0] == 'A')
				break;
		}
		if (isLogin == 1)
		{
			printf("Send new password to server: ");
			fgets(buff, BUFF_SIZE, stdin);
			if (buff[0] == '\n')
				exit(1);
			buff[strlen(buff) - 1] = '\0';
			len = sizeof(servaddr);
			sendBytes = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&servaddr, len); // send message to server
			if (sendBytes < 0)
			{
				perror("Error 1");
				return 0;
			}
			for (;;)
			{
				rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&servaddr, &len); // receive message from server
				if (rcvBytes < 0)
				{
					perror("Error 2");
					return 0;
				}
				buff[rcvBytes] = '\0';
				printf("Reply from server: %s\n", buff);
				if (strcmp(buff, "ok") == 0)
				{
					printf("--------------------\n");
					break;
				}
				if (buff[0] == 'G')
				{
					isLogin = 0;
					break;
				}
			}
		}

	} while (1);

	close(sockfd);
	return 0;
}
