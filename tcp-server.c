// ud client driver program:q
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 4242
#define MAXLINE 1024
#define BUFFER_SIZE 512
#define SIZE 1024

int main()
{
    char buffer[BUFFER_SIZE];
    int sockfd, n, new_sock;
    struct sockaddr_in servaddr, newaddr;

    // create stream socket using socket()
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.30");
    
    // bind socket with bind() function
    int e;
    //while(1){
    	e = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    //}
    if(e < 0) {
        perror("[-]Error in bind");
        exit(1);
    }
    
    // listen for connection from client
    if(listen(sockfd, 10) == 0)
    {
		printf("[+]Listening..\n");
		printf("test listen\n");
    }
    else
    {
		perror("[-]Error in listening");
    		exit(1);
    }
    
    // accept connection from client with accept() function
    printf("going into accept loop...\n");
    while(1){
	printf("start accept loop\n");
    	int addr_size = sizeof(newaddr);
	printf("before accept call\n");
    	new_sock = accept(sockfd, (struct sockaddr*)&newaddr, &addr_size);
	printf("after accept call\n");
	if (new_sock<0){
		perror("accept failed\n");
		continue;
	}
	else break;
    	printf("Accepted connection from client successfully!\n");
    }

   // **NEW: Receive LLEXT file from client**
        // First receive filename length
        int filename_len;
        recv(new_sock, &filename_len, sizeof(filename_len), 0);
	printf("filename_len is %d\n", filename_len);
        // Receive filename
        char filename[256];
        recv(new_sock, filename, filename_len, 0);
        filename[filename_len] = '\0';
        printf("Filename is %s\n", filename);

        // Receive file size
        int file_size;
        recv(new_sock, &file_size, sizeof(file_size), 0);
        printf("File size is %d bytes\n", file_size);

        // Open file for writing
        FILE *fp = fopen(filename, "wb");
        if (fp == NULL) {
            perror("Error opening file for writing");
            close(new_sock);
            //continue;
        }

        // Receive file data
        long bytes_received = 0;
        while (bytes_received < file_size) {
            int chunk_size = recv(new_sock, buffer, BUFFER_SIZE, 0);
            if (chunk_size <= 0) break;
            fwrite(buffer, 1, chunk_size, fp);
            bytes_received += chunk_size;
        }

        //fclose(fp);
       //printf("[+]LLEXT file received successfully: %s (%ld bytes)\n", filename, bytes_received);

        close(new_sock);
   //}

    // Close the descriptor
    close(sockfd);
    return 0;
}
