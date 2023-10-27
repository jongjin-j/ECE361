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

char *extractFiledata(char *buffer)
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

    // Receive first packet from client
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';
    char *totalfiledata = extractFiledata(buffer);

        // Extract number of total fragments, current fragment number, and filename
    char *token = strtok(buffer, ":");
    int total_frag = atoi(token);
    printf("Total frag %d\n", total_frag);
    token = strtok(NULL, ":");
    int frag_no = atoi(token);
    printf("Frag number %d\n", frag_no);
    token = strtok(NULL, ":");
    int frag_size = atoi(token);
    token = strtok(NULL, ":");
    char *filename =  malloc(1000);
    strcpy(filename, token);
    // char *filename_2 = "kyfrerqg";

    FILE *fp = fopen(filename, "a+");
            if(fp){
                fwrite(totalfiledata, strlen(totalfiledata), 1, fp);
                printf("appending %s\n", totalfiledata);
                fclose(fp);
            }



    printf("This is the filename %s\n", filename);

    printf("%s", filename);

    if (frag_no == 0)
    {
        printf("Packet number %d received out of %d\n", frag_no, total_frag - 1);
        printf("\nFile Data: %s\n", totalfiledata);
        sendto(sockfd, (const char *)ack, strlen(ack), 0,
               (const struct sockaddr *)&cliaddr, len);
    }
    else
    {
        sendto(sockfd, (const char *)nack, strlen(nack), 0,
               (const struct sockaddr *)&cliaddr, len);
    }

    while (frag_no != total_frag - 1)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';
        char *filedata = extractFiledata(buffer);

        token = strtok(buffer, ":");
        token = strtok(0, ":");
        int cur_frag_no = atoi(token);

        // Send ACK or NACK depending on whether the correct packet was delivered
        if (cur_frag_no == frag_no + 1)
        {
            printf("\n\nPacket number %d received out of %d\n", cur_frag_no, total_frag - 1);
            // printf("File data: %s\n", filedata);
            sendto(sockfd, (const char *)ack, strlen(ack), 0,
                   (const struct sockaddr *)&cliaddr, len);


            
            //reconstruct
            printf("this is the frag size %d", frag_size);
            FILE *fp = fopen(filename, "a+");
            if(fp){
                fwrite(filedata, strlen(filedata), 1, fp);
                //printf("appending %s\n", filedata);
                fclose(fp);
            }
            else{
                fclose(fp);
            }
            
            frag_no++;
        }
        else
        {
            printf("Something went wrong with packet number %d\n", cur_frag_no);
            sendto(sockfd, (const char *)nack, strlen(nack), 0,
                   (const struct sockaddr *)&cliaddr, len);
        }
    }

    return 0;
}