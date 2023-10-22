#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#define FILENAME "nguoidung.txt"
#define BUFF_SIZE 255
int sockfd, rcvSize;
int flag = 0;
unsigned int len;
char buff[BUFF_SIZE];
char recvBuff[BUFF_SIZE];
char done[BUFF_SIZE];
struct sockaddr_in servaddr, cliaddr;
short serv_PORT;
typedef struct User
{
	char username[50];
	char password[50];
	int status;
	int attempt;
	struct User *next;
} User;
User *head = NULL;
User *currentUser = NULL;
int isLoggedIn = 0;
char loggedInUsername[50];
char CurrentHomepage[50];
char correctActivationCode[] = "20210216";
char number[BUFF_SIZE];
char alphabet[BUFF_SIZE];
int error = 0;
void processRecvBuff(const char *receive)
{
	int countNumber = 0, countAlphabet = 0;
	for (size_t i = 0; i < strlen(receive); i++)
	{
		char currentChar = receive[i];

		if (isdigit(currentChar))
		{
			if (countNumber < BUFF_SIZE - 1)
			{
				number[countNumber++] = currentChar;
			}
			else
			{
			}
		}
		else if (isalpha(currentChar))
		{
			if (countAlphabet < BUFF_SIZE - 1)
			{
				alphabet[countAlphabet++] = currentChar;
			}
			else
			{
			}
		}
		else
		{
			error = 1;
			break;
		}
	}
	number[countNumber] = '\0';
	alphabet[countAlphabet] = '\0';
}
void loadUsersFromFile()
{
	FILE *file = fopen(FILENAME, "r");
	if (file == NULL)
	{
		perror("Khong the mo file");
		exit(1);
	}
	char line[150];
	while (fgets(line, sizeof(line), file))
	{
		User *newUser = (User *)malloc(sizeof(User));
		sscanf(line, "%s %s %d", newUser->username, newUser->password, &newUser->status);
		newUser->attempt = 0;
		newUser->next = head;
		head = newUser;
	}
	fclose(file);
}
/*ghi thong tin ra file*/
void saveUsersToFile()
{
	FILE *file = fopen(FILENAME, "w");
	if (file == NULL)
	{
		perror("Khong the mo file");
		exit(1);
	}
	User *current = head;
	while (current != NULL)
	{
		fprintf(file, "%s %s %d  \n", current->username, current->password, current->status);
		current = current->next;
	}
	fclose(file);
}
/*Chuc nang kiem tra mat khau*/
void checkWrongAttempts(User *user, char *enteredPassword)
{
	if (strcmp(enteredPassword, user->password) == 0)
	{

		sendto(sockfd, "OK", BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
		isLoggedIn = 1;
		flag = 2;
		currentUser = user;
		return;
	}
	user->attempt++;
	if (user->attempt >= 3)
	{
		user->status = 0;
		flag = 0;
		saveUsersToFile();
		sendto(sockfd, "Password is incorrect. Account is blocked\n", BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
	}
	else
	{
		sendto(sockfd, "NOT OK", BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
	}
}
/*Chuc nang tim kiem tai khoan*/
User *searchUser(char *username)
{
	User *current = head;
	while (current != NULL)
	{
		if (strcmp(current->username, username) == 0)
		{

			return current;
		}
		current = current->next;
	}
	sendto(sockfd, "Cannot find account\n", BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
	return NULL;
}
int main(int argc, char *argv[])
{
	loadUsersFromFile();
	len = sizeof(cliaddr);
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}
	serv_PORT = atoi(argv[1]);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		perror("Error: ");
		return 0;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(serv_PORT);
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("Error: ");
		return 0;
	}
	printf("Server started!\n");
	char username[50], password[50];

	for (;;)
	{
		memset(buff, '\0', sizeof(buff));
		memset(done, '\0', sizeof(done));

		rcvSize = recvfrom(sockfd, recvBuff, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, &len); // receive message from client
		if (rcvSize < 0)
		{
			perror("Error: ");
			return 0;
		}
		recvBuff[rcvSize] = '\0';
		if (flag == 0)
		{
			strcpy(username, recvBuff);
			currentUser = searchUser(username);
			if (searchUser(username) != NULL && currentUser->status == 1)
			{
				sendto(sockfd, "USER FOUND", BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
				flag = 1;
			}
			else if (currentUser->status == 2)
			{
				flag = 0;
				sendto(sockfd, "Account is not activated\n", BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
			}
			else if (currentUser->status == 0)
			{
				flag = 0;
				sendto(sockfd, "Account is blocked\n", BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
			}
			continue;
		}
		else if (flag == 1)
		{
			strcpy(password, recvBuff);
			checkWrongAttempts(currentUser, password);
		}
		else
		{
			if (rcvSize < 0)
			{
				perror("Error: ");
				return 0;
			}
			recvBuff[rcvSize] = '\0';
			if (strcmp(recvBuff, "bye") == 0)
			{
				char str1[100] = "Goodbye";
				char str2[100];
				strcpy(str2, currentUser->username);
				currentUser->attempt = 0;
				char str3[100];
				strcpy(str3, str1);
				strcat(str3, str2);
				sendto(sockfd, str3, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, len);
				flag = 0;
				continue;
			}
			printf("Receive from client: %s\n", recvBuff);
			processRecvBuff(recvBuff);
			if (error == 1)
			{
				strcpy(buff, "error");
				sendto(sockfd, buff, rcvSize, 0, (struct sockaddr *)&cliaddr, len); 
				strcpy(done, "ok");
				sendto(sockfd, done, rcvSize, 0, (struct sockaddr *)&cliaddr, len); 
				error = 0;
			}
			else
			{
				if (number[0] != '\0')
					sendto(sockfd, number, rcvSize, 0, (struct sockaddr *)&cliaddr, len);
				if (alphabet[0] != '\0')
					sendto(sockfd, alphabet, rcvSize, 0, (struct sockaddr *)&cliaddr, len);
				strcpy(done, "ok");
				sendto(sockfd, done, rcvSize, 0, (struct sockaddr *)&cliaddr, len); 
			}
		}
	}
	saveUsersToFile();
	return 0;
}
