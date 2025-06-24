#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 4242
#define BUFFER_SIZE 1024

/*#define FS_O_CREATE 0x10
//#define FS_O_WRITE 0x02*/

#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#include <zephyr/net/ethernet.h>




void main() {
    // code for mounting lfs
    FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage_fs_data);
struct fs_mount_t littlefs_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &storage_fs_data,
    .storage_dev = (void *)FIXED_PARTITION_ID(storage_partition),
    .mnt_point = "/lfs"
};
    int rc = fs_mount(&littlefs_mnt);
    if (rc < 0) {
    printk("Failed to mount LittleFS: %d\n", rc);
    }

    // Open file for writing
    const char *file_name = "/lfs/test.bin";
    struct fs_file_t zfp;
    fs_file_t_init(&zfp);
    rc = fs_open(&zfp, file_name,FS_O_CREATE | FS_O_WRITE);

    if (rc < 0) {
        LOG_ERR("Failed to open file for writing [%d]", rc);
        return;
    }


    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation error");
        exit(1);
    }

    printf("Hello\n");
    // Fill server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // or INADDR_ANY
    servaddr.sin_port = htons(PORT);

    // Bind socket to server address
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    printf("UDP server listening on port %d...\n", PORT);
    
    //initalize file variable
    //struct fs_file_t *file;


    // Keep listening for packets
    // Receive packets and write to file
    int n;
    while (1) {
        cliaddr_len = sizeof(cliaddr);
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                     (struct sockaddr*)&cliaddr, &cliaddr_len);
        if (n < 0) {
            LOG_ERR("recvfrom error: %d", errno);
            continue;
        }

        // Write received data to file
	printf("n=%d\n",n);
        printf("Received: %.*s\n", n, buffer);  // Safe length-limited print
        ssize_t written = fs_write(&zfp, buffer, n);
        if (written < 0) {
            LOG_ERR("File write error: %d", written);
            break;
        }
	printf("wriiten = %d\n", written);
        printf("Received and wrote %d bytes", n);
	break;
    }
    void *data_buffer=malloc(n);
    printf("return value of fs_read is %d", fs_read(&zfp,data_buffer,n ));
    printf("%p", data_buffer);
    free(data_buffer);
    data_buffer=NULL;
    fs_close(&zfp);
    close(sockfd);
}

