#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <syslog.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024


void daemonize() {
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
    
        exit(EXIT_SUCCESS);  
    }
  
    if (setsid() < 0) {
        perror("Setsid failed");
        exit(EXIT_FAILURE);
    }
    
    if (chdir("/") < 0) {
        perror("Chdir failed");
        exit(EXIT_FAILURE);
    }
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    openlog("image_server_daemon", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Daemon started successfully"); 
}

void reverse_string(char *str) {
    int length = strlen(str);
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}


char *hex_to_string(const char *hex, size_t *decoded_size) {
    int len = strlen(hex);
    char *result = (char *)malloc(len / 2 + 1);  
    for (int i = 0; i < len; i += 2) {
        sscanf(hex + i, "%2hhx", &result[i / 2]);
    }
    *decoded_size = len / 2; 
    result[len / 2] = '\0';  
    return result;
}


void save_decrypted_file(const char *data, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char filename[50];
    snprintf(filename, sizeof(filename), "database/%ld.jpeg", t);

    FILE *file = fopen(filename, "wb");
    if (file != NULL) {
        fwrite(data, 1, size, file);
        fclose(file);
        printf("Decryption successful! File saved as %s\n", filename);
    }
}
void decrypt_file(int socket, char *buffer) {

 
    reverse_string(buffer);
    
    size_t decoded_size = 0;
    char *decoded_data = hex_to_string(buffer, &decoded_size);

   
    save_decrypted_file(decoded_data, decoded_size);

  
    const char *response = "Decryption and save successful!";
    send(socket, response, strlen(response), 0);

    free(decoded_data);  
}

 void send_file(int client_socket, char *filename) {
    char path[100];
    snprintf(path, sizeof(path), "database/%s", filename);
     FILE *file = fopen(path, "rb");
     if (file == NULL) {
         const char *error_msg = "Error: File not found";
         send(client_socket, error_msg, strlen(error_msg), 0);
         return;
     }
 
     fseek(file, 0, SEEK_END);
     long file_size = ftell(file);
     fseek(file, 0, SEEK_SET);
 
     send(client_socket, &file_size, sizeof(file_size), 0);
 
     char buffer[MAX_BUFFER_SIZE];
     size_t bytes_read;
     while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
     send(client_socket, buffer, bytes_read, 0);
     }
 
     fclose(file);
     const char *response = "File sent successfully!";
     send(client_socket, response, strlen(response), 0);
 }

void handle_client(int client_socket) {
    while (1)
    {
        char buffer[MAX_BUFFER_SIZE * 10];
        int read_size;
    
       
        read_size = recv(client_socket, buffer, sizeof(buffer), 0);
        buffer[read_size] = '\0';  
        if (read_size > 0) {
            buffer[read_size] = '\0';  
            printf("Received file data for decryption: %s\n", buffer);
    
            if (strncmp(buffer, "decrypt", 7) == 0) {
                printf("Decrypting file...\n");
                char *filecontent = buffer + 8;
                decrypt_file(client_socket, filecontent);
                
            } else if (strncmp(buffer, "download", 8) == 0) {
                char *filename = buffer + 8;
                printf("Requesting file %s for download...\n", filename);
                send_file(client_socket, filename);
            } else {
                const char *error_msg = "Invalid command!";
                send(client_socket, error_msg, strlen(error_msg), 0);
            }
            
            close(client_socket);
        }
    }
    

   
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

   
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

  
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

  
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    daemonize(); 
    printf("Server listening on port %d...\n", PORT);

  
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        handle_client(client_socket);
    }

    close(server_fd);
    return 0;
}
