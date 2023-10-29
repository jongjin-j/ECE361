/*
Lab Title: File Transfer Lab

Author(s): Charlotte Fritz, JongJin Jung
Lab Date: October 1st, 2023
Course: ECE361
Practical Section: PRA0103

Usage:
- Compile this program using a C compiler (gcc server.c -o server).
- Run the compiled executable.

Based on code provided in 4-Ta- Socket Programming lecture slides and from
https://www.scaler.com/topics/udp-server-client-implementation-in-c/

Question: Can we use string functions on messages?
Answer: Yes, but be cautious of buffer overflows and security vulnerabilites.
*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 1024

typedef struct
{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char *filename;
    char filedata[1000];
} packet;

/*char *reconstructPacket(char *buffer, packet packet, int size)
{
    char *filedata = malloc(1500);

    size_t i = 0;
    int colonCount = 0;

    while (colonCount != 4)
    {
        if (buffer[i] == ':')
        {
            colonCount++;
        }
        i++;
    }

    int count = 0;

    while (buffer[i] != '\0')
    {
        memcpy(filedata + count, buffer + i, 2);
        count++;
        i++;
    }

    return filedata;
}*/

packet reconstructPacket(char *buffer, int n)
{
    packet p;
    // buffer[n] = '\0';
    printf("HERE");
    char *token = strtok(buffer, ":");
    int total_frag = atoi(token);
    p.total_frag = total_frag;
    printf("Total frag %d\n", total_frag);
    token = strtok(NULL, ":");
    int frag_no = atoi(token);
    p.frag_no = frag_no;
    printf("Frag number %d\n", frag_no);
    token = strtok(NULL, ":");
    int frag_size = atoi(token);
    p.size = frag_size;
    token = strtok(NULL, ":");
    char *filename = token;
    p.filename = filename;
    token = strtok(NULL, ":");
    printf("len %d\n", sizeof(token));
}

// Driver code
int main(int argc, char *argv[])
{
    int portNumber = atoi(argv[1]);

    int sockfd;
    char buffer[MAXLINE];

    char *yes = "yes";
    char *no = "no";
    char *ack = "ACK";
    char *nack = "NACK";
    struct sockaddr_in servaddr, cliaddr;

    // Accept execution command
    char server[100];
    unsigned short int PORT = (short)portNumber;
    bool isValid = false;

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Creating socket file descriptor
    if (sockfd < 0)
    {
        printf("socket creation failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        // printf("Socket creation successful");
    }

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == 0)
    {
        // printf("Bind successful");
    }
    else
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int len, n;
    len = sizeof(cliaddr); // len is value/result
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';
    // printf("Message from client : %s\n", buffer);

    if (strcmp(buffer, "ftp") == 0)
    {
        sendto(sockfd, (const char *)yes, strlen(yes), 0,
               (const struct sockaddr *)&cliaddr, len);
        // printf("Message 'yes' sent to client.\n");
    }
    else
    {
        sendto(sockfd, (const char *)no, strlen(no), 0,
               (const struct sockaddr *)&cliaddr, len);
        // printf("Message 'no' sent to client.\n");
    }

    for (int i = 0; i < 2; i++)
    {
        // Receive first packet from client
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&cliaddr, &len);

        printf("BUFFER SIZE %d\n", n);
        buffer[n] = '\0';

        packet p;
        char *token = strtok(buffer, ":");
        int total_frag = atoi(token);
        p.total_frag = total_frag;
        token = strtok(NULL, ":");
        int frag_no = atoi(token);
        p.frag_no = frag_no;
        // printf("Frag number %d\n", frag_no);
        token = strtok(NULL, ":");
        int frag_size = atoi(token);
        p.size = frag_size;
        token = strtok(NULL, ":");
        char *filename = token;
        p.filename = filename;
        token = strtok(NULL, ":");
        // printf("%s\n", token);
        printf("packet: %s\n", buffer);

        int serialSize = snprintf(NULL, 0, "%d:%d:%d:%s:", p.total_frag, p.frag_no, p.size, p.filename);
        printf("s size %d\n", serialSize);
        char *filedata = malloc(p.size * sizeof(char));
        memcpy(filedata, buffer + serialSize, p.size);
        // printf("len %d\n", sizeof(token));

        // packet p = reconstructPacket(buffer, n);
        printf("Total frag %d\n", p.total_frag);
        printf("Frag number %d\n", p.frag_no);
        printf("size %d\n", p.size);
        printf("%s\n", p.filename);

        FILE *fp = fopen("testtest.png", "a");
        if (fp)
        {
            fwrite(filedata, frag_size, 1, fp);
            fclose(fp);
        }

        if (frag_no == i)
        {
            printf("Packet number %d received out of %d\n", frag_no, total_frag - 1);
            sendto(sockfd, (const char *)ack, strlen(ack), 0,
                   (const struct sockaddr *)&cliaddr, len);
        }
        else
        {
            sendto(sockfd, (const char *)nack, strlen(nack), 0,
                   (const struct sockaddr *)&cliaddr, len);
        }
    }

    int count = 1;

    /*while (count < total_frag)
    {
        printf("count: %d\n", count);
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&cliaddr, &len);

        printf("BUFFER SIZE %d\n", n);

        packet p;
        char *token = strtok(buffer, ":");
        int total_frag = atoi(token);
        p.total_frag = total_frag;
        token = strtok(NULL, ":");
        int frag_no = atoi(token);
        p.frag_no = frag_no;
        token = strtok(NULL, ":");
        int frag_size = atoi(token);
        p.size = frag_size;
        token = strtok(NULL, ":");
        char *filename =  token;
        p.filename = filename;
        token = strtok(NULL, ":");

        int serialSize = snprintf(NULL, 0, "%d:%d:%d:%s:", p.total_frag, p.frag_no, p.size, p.filename);
        printf("s size %d\n", serialSize);
        char *filedata = malloc(p.size * sizeof(char));
        memcpy(filedata, buffer + serialSize, p.size);

        // Send ACK or NACK depending on whether the correct packet was delivered
        if (count == frag_no)
        {
            printf("\n\nPacket number %d received out of %d\n", count, total_frag - 1);
            sendto(sockfd, (const char *)ack, strlen(ack), 0,
                   (const struct sockaddr *)&cliaddr, len);
            FILE *fp = fopen("testtest.png", "a+");
            if(fp){
                fwrite(filedata, frag_size, 1, fp);
                fclose(fp);
            }
            else{
                fclose(fp);
            }

            count++;
        }
        else
        {
            printf("Something went wrong with packet number %d\n", count);
            sendto(sockfd, (const char *)nack, strlen(nack), 0,
                   (const struct sockaddr *)&cliaddr, len);
        }
    }*/

    return 0;
}