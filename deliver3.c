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
    sprintf(total_frag_str, "%d", curPacket.total_frag);
    char frag_no_str[5];
    sprintf(frag_no_str, "%d", curPacket.frag_no);
    char size_str[5];
    sprintf(size_str, "%d", curPacket.size);

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

    /*struct r_t r2;
    FILE *f2 = fopen(file_name, "r");
    fseek(f2, 0L, SEEK_END);
    size_t size2 = ftell(f2);
    fseek(f2, 0L, SEEK_SET);
    printf("%d\n", size2);
    int pckCount = 0;
    do
    {
        r2.len = fread(r2.payload, 1, sizeof(r2.payload), f2);
        printf("%s\n", r2.payload);
        printf("%d\n", pckCount);
        pckCount++;
    } while (r2.len > 0);
    fclose(f2);*/


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
    
    /*FILE *fp;
    unsigned char* buffer2;
    unsigned long fileLen2;

    fp = fopen(file_name, "rb");
    fseek(fp, 0, SEEK_END);
    fileLen2 = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer2 = (char*) malloc(fileLen2);
    fread(buffer2, fileLen2, sizeof(unsigned char), fp);
    
    int curLen = 0;
    while(curLen < fileLen2) {
        printf("%02X", ((unsigned char)buffer2[curLen]));
        curLen++;
    }

    printf("\n\n\n\n");

    fclose(fp);*/

    // Getting size of file and calculating number of fragments needed
    struct stat st;
    stat(file_name, &st);
    off_t filesize = st.st_size;
    unsigned int total_frag = filesize / 1000 + 1;
    unsigned int frag_no = 0;

    // Initializing an array of packets
    struct packet packets[total_frag];

    while (filesize != 0)
    {
        // Filling in packet array with packets and assigning values to members in the packet struct
        struct packet curPacket;
        curPacket.total_frag = total_frag;
        curPacket.frag_no = frag_no;

        if (filesize > 1000)
        {
            curPacket.size = 1000;
            filesize -= 1000;
        }
        else
        {
            curPacket.size = filesize;
            filesize = 0;
        }

        curPacket.filename = file_name;
        packets[frag_no] = curPacket;
        frag_no++;
    }

    // Reading from given file and copying in value of filedata into packets
    struct r_t r;
    FILE *f = fopen(file_name, "rb");
    // fseek(f, 0L, SEEK_END);
    // size_t size = ftell(f);
    // fseek(f, 0L, SEEK_SET);

    char buf [60];

    while (fread(buf, sizeof(buf), 1, f) == 1) {
        printf("%s", buf);
        fwrite(buf, sizeof(buf), 1, stdout);
    }

    //uint8_t* byteArr;
    /*uint8_t byteArr[size];

    if (f != NULL) {
        int counter = 0;
        do {
            byteArr[counter] = fgetc(f);
            counter++;
        } while (counter <= size);
        fclose(f);
    }

    for (size_t i = 0; i < size; i++) {
        append(packets[i / 1000].filedata, byteArr[i]);
        printf("%02X", byteArr[i]);
    }*/

    int packetCount = 0;
    do
    {
        r.len = fread(&r.payload, 1, sizeof(r.payload), f);        
        int curLen = 0;

        while(curLen < r.len) {
            //printf("%c", r.payload[curLen]);
            append(packets[packetCount].filedata, r.payload[curLen]);
            curLen++;
        }

        //memcpy(packets[packetCount].filedata, r.payload, r.len);
        //printf("%s\n", packets[packetCount].filedata);
        packetCount++;
    } while (r.len > 0);
    fclose(f);

    // Sending packets to server
    for (int i = 0; i < total_frag; i++)
    {
        char *packetStr = generatePacketStr(packets[i]);
        //printf("%s\n", packetStr);
        // Send packet to server
        sendto(sockfd, (const char *)packetStr, customstrlen(packetStr), MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));

        // Receive ACK or NACK from server
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
    }

    close(sockfd);
    return 0;
}