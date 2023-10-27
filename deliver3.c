/*
Lab Title: File Transfer Lab

Author(s): Charlotte Fritz, JongJin Jung
Lab Date: October 1st, 2023
Course: ECE361
Practical Section: PRA0103

Usage:
- Compile this program using a C compiler (gcc client.c -o client).
- Run the compiled executable.

Based on code provided in 4-Ta- Socket Programming lecture slides and from
https://www.scaler.com/topics/udp-server-client-implementation-in-c/
*/

//
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLINE 1024

struct packet
{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char *filename;
    char filedata[1000];
};

struct r_t
{
    uint8_t payload[1000];
    int len;
};

size_t customstrlen(char* string) {
    size_t count = 0;
    for (char ch = *string; ch != '\0'; ch = *++string) {
        count++;
    }

    return count;
}

void append(uint8_t* s, uint8_t c) {
        int len = customstrlen(s);
        s[len] = c;
        s[len+1] = '\0';
};

char *generatePacketStr(struct packet curPacket)
{
    char *packetStr = malloc(1500);
    char total_frag_str[5];
    snprintf(total_frag_str, "%d", curPacket.total_frag);
    char frag_no_str[5];
    snprintf(frag_no_str, "%d", curPacket.frag_no);
    char size_str[5];
    snprintf(size_str, "%d", curPacket.size);

    memcpy(packetStr, total_frag_str, customstrlen(total_frag_str) + 1);
    memcpy(packetStr + customstrlen(packetStr), ":", 2);
    memcpy(packetStr + customstrlen(packetStr), frag_no_str, customstrlen(frag_no_str) + 1);
    memcpy(packetStr + customstrlen(packetStr), ":", 2);
    memcpy(packetStr + customstrlen(packetStr), size_str, customstrlen(size_str) + 1);
    memcpy(packetStr + customstrlen(packetStr), ":", 2);
    memcpy(packetStr + customstrlen(packetStr), curPacket.filename, customstrlen(curPacket.filename) + 1);
    memcpy(packetStr + customstrlen(packetStr), ":", 2);
    memcpy(packetStr + customstrlen(packetStr), curPacket.filedata, customstrlen(curPacket.filedata) + 1);

    //printf("PCK STR: %s\n", packetStr);

    return packetStr;
}

int main(int argc, char *argv[])
{
    int portNumber = atoi(argv[2]);

    int sockfd;
    char buffer[MAXLINE];
    char *ftp_string = "ftp";
    struct sockaddr_in servaddr;

    // Accept execution command
    short unsigned int PORT = (short unsigned int)portNumber;
    char deliver[32];
    bool valid_resp = false;

    bool isValid = false;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(8080);

    // Accept ftp input arguments from user
    char file_name[32];
    char ftp[32];
    bool valid_ftp = false;

    while (valid_ftp == false)
    {
        printf("Please enter command 'ftp <file_name>' \n");
        if (scanf("%19s", ftp) != 1)
        {
            // Input does not match the desired format
            printf("Invalid input. Please use the format 'ftp <file_name>'.\n");
            while (getchar() != '\n')
                ; // Clear the input buffer
            continue;
        }

        if (strcmp(ftp, "ftp") != 0)
        {
            // The command is not 'server'
            printf("Invalid command. Please use 'ftp'.\n");
            while (getchar() != '\n')
                ; // Clear the input buffer

            continue;
        }

        // Check for the presence of a newline character
        int c;
        if ((c = getchar()) == '\n')
        {
            // Newline character encountered without UDP listen port
            printf("File name is missing.\n");
            continue;
        }
        else if (c == EOF)
        {
            // End of input reached
            printf("Input ended without providing a file name.\n");
            break;
        }

        if (scanf("%19s", file_name) != 1)
        {
            // Input does not match the desired format
            printf("Invalid input. Please use the format 'ftp <file_name>'.\n");
            while (getchar() != '\n')
                ; // Clear the input buffer
            continue;
        }

        valid_ftp = true;
    }

    if (access(file_name, F_OK) == 0)
    {
        // send message
        sendto(sockfd, (const char *)ftp_string, customstrlen(ftp_string), MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }
    else
    {
        exit(0);
    }

    int n, len;
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';

    if (strcmp(buffer, "yes") == 0)
    {
        printf("A file transfer can start.\n");
    }
    else
    {
        exit(0);
    }

    // Getting size of file and calculating number of fragments needed
    struct stat st;
    stat(file_name, &st);
    off_t filesize = st.st_size;
    unsigned int total_frag = filesize / 1000;
    unsigned int frag_no = 0;

    // Initializing an array of packets
    struct packet packet;

    // Reading from given file and copying in value of filedata into packets
    struct r_t r;
    FILE *f = fopen(file_name, "rb");
    fseek(f, 0L, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    int packetCount = 0;

    packet.total_frag = total_frag;
    packet.filename = file_name;

    //FILE *fp = fopen("testtest.png", "a+");

    for (int i = 0; i < total_frag; i++) {
        r.len = fread(r.payload, 1, sizeof(r.payload), f);
        
        packet.frag_no = i;
        packet.size = r.len;
        memcpy(packet.filedata, r.payload, r.len);

        int serialSize = snprintf(NULL, 0, "%d:%d:%d:%s:", packet.total_frag, packet.frag_no, packet.size, packet.filename);
        char *packetStr = malloc((serialSize + r.len) * sizeof(char));
        sprintf(packetStr, "%d:%d:%d:%s:", packet.total_frag, packet.frag_no, packet.size, packet.filename);
        printf("%s\n", packet.filedata);
        memcpy(packetStr + serialSize, packet.filedata, packet.size);
        //printf("%s\n", packetStr);

        sendto(sockfd, (const char *)packetStr, r.len, MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        if (strcmp(buffer, "ACK") == 0)
        {
            printf("ACK Received.\n");
        }
        else
        {
            printf("NACK Received.\n");
        }

        //printf("%s\n", packetStr);
        free(packetStr);

        /*if(fp){
            fwrite(packet.filedata, r.len, 1, fp);
        }*/
        // serialize with snprintf and send
    }

    //fclose(fp);

    fclose(f);

    close(sockfd);
    return 0;
}