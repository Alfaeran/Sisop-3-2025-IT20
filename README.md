# Sisop-3-2025-IT20

## Modul 3 Sistem Operasi 2025
- **Mey Rosalina NRP 5027241004**
- **Rizqi Akbar Sukirman Putra NRP 5027241044**
- **M. Alfaeran Auriga Ruswandi NRP 5027241115**

## Laporan Resmi Modul 3

### Soal 1
image_server.c
```c

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
```

- image_client.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

void clear_screen() {
    printf("\033[H\033[J");
}

void set_text_color(const char *color) {
    printf("%s", color);
}

void reset_text_color() {
    printf("\033[0m");
}

void display_menu() {
    set_text_color("\033[1;35m");
    printf("══════════════════════════════════════════\n");
    printf("          |  Image Decoder Client  |         \n");
    printf("══════════════════════════════════════════\n");
    reset_text_color();

    printf("╔════════════════════════════════════════╗\n");
    printf("║        Choose an option below:         ║\n");
    printf("╚════════════════════════════════════════╝\n");

    set_text_color("\033[1;32m");
    printf("┌────────────────────────────────────────┐\n");
    printf("│  1. Send input file to server          │\n");
    printf("│  2. Download file from server         │\n");
    printf("│  3. Exit                              │\n");
    printf("└────────────────────────────────────────┘\n");
    reset_text_color();

    printf("\nPlease enter your command: ");
}
void decrypt_file(int socket, const char *filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "secrets/%s", filename);

    struct stat st;

    if (stat(filepath, &st) != 0) {
        set_text_color("\033[1;31m");  
        printf("File %s not found!\n", filepath);
        reset_text_color();
        return;
    }

    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("File not found");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_data = (char *)malloc(file_size);
    fread(file_data, 1, file_size, file);
    fclose(file);

    set_text_color("\033[1;33m");  
    printf("Sending file %s for decryption...\n",  filename);
    reset_text_color();

    send(socket, "decrypt ", 8, 0);
    send(socket, file_data, file_size, 0); 

    char buffer[MAX_BUFFER_SIZE];
    int read_size = recv(socket, buffer, sizeof(buffer), 0);
    buffer[read_size] = '\0';
    set_text_color("\033[1;32m");
    printf("Server: %s\n", buffer);
    reset_text_color();

    free(file_data);
}

void download_file(int socket, const char *filename) {
    set_text_color("\033[1;33m");
    printf("Requesting file %s for download...\n", filename);
    reset_text_color();

    send(socket, "download", 8, 0);
    send(socket, filename, strlen(filename), 0);

    long file_size;
    recv(socket, &file_size, sizeof(file_size), 0);

    char *file_data = (char *)malloc(file_size);
    recv(socket, file_data, file_size, 0);

    FILE *file = fopen(filename, "wb");
    fwrite(file_data, 1, file_size, file);
    fclose(file);

    set_text_color("\033[1;32m");
    printf("Success! Image saved as %s\n", filename);
    reset_text_color();

    free(file_data);
}
void show_menu(int socket) {
    int choice;
    char filename[100];

    while (1) {
        display_menu();

        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1:
                printf("Enter the file name: ");
                scanf("%s", filename);
                decrypt_file(socket, filename);
                break;
            case 2:
                printf("Enter the file name: ");
                scanf("%s", filename);
                download_file(socket, filename);
                break;
            case 3:
                printf("Exiting...\n");
                close(socket);
                return;
            default:
                set_text_color("\033[1;31m");
                printf("Invalid choice! Please select again.\n");
                reset_text_color();
        }
    }
}

void print_error_message(char *message) {
    printf("\033[1;31m[ERROR] %s\033[0m\n", message);
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char filename[100];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    set_text_color("\033[1;36m");
    printf("Connected to address 127.0.0.1:8080\n");
    reset_text_color();

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        print_error_message("Gagal connect ke server");
        return -1;
    }
    
    show_menu(sock);

    return 0;
}
```
- Pada image_server.c, membuat funtion yang berjalan secara daemon di background dan terhubung dengan image_client.c melalui socket RPC.
-- function daemon program
```c
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
    
    if (chdir("/home/auriga/Documents/Sisop-3-2025-IT20/soal_1/server") < 0) {
        perror("Chdir failed");
        exit(EXIT_FAILURE);
    }
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    openlog("image_server_daemon", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Daemon started successfully"); 
```
-- function penghubung server ke client  melalui socket RPC
```c
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
```
- image_client.c yang sudah berhasil terhubung mengirimkan text file ke image_Server.c
```c
void decrypt_file(int socket, const char *filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "secrets/%s", filename);

    struct stat st;

    if (stat(filepath, &st) != 0) {
        set_text_color("\033[1;31m");  
        printf("File %s not found!\n", filepath);
        reset_text_color();
        return;
    }

    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("File not found");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_data = (char *)malloc(file_size);
    fread(file_data, 1, file_size, file);
    fclose(file);

    set_text_color("\033[1;33m");  
    printf("Sending file %s for decryption...\n",  filename);
    reset_text_color();

    send(socket, "decrypt ", 8, 0);
    send(socket, file_data, file_size, 0); 

    char buffer[MAX_BUFFER_SIZE];
    int read_size = recv(socket, buffer, sizeof(buffer), 0);
    buffer[read_size] = '\0';
    set_text_color("\033[1;32m");
    printf("Server: %s\n", buffer);
    reset_text_color();

    free(file_data);
}
```

- Function decrypt text file yang dimasukkan dengan cara Reverse Text lalu Decode from Hex, untuk disimpan dalam folder database server dengan nama file berupa timestamp dalam bentuk angka, misalnya: database/1744401282.jpeg.
```c
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
```
- function Request download dari database server sesuai filename yang dimasukkan, misalnya: 1744401282.jpeg

-- image_client.c
```c
void download_file(int socket, const char *filename) {
    set_text_color("\033[1;33m");
    printf("Requesting file %s for download...\n", filename);
    reset_text_color();

    send(socket, "download", 8, 0);
    send(socket, filename, strlen(filename), 0);

    long file_size;
    recv(socket, &file_size, sizeof(file_size), 0);

    char *file_data = (char *)malloc(file_size);
    recv(socket, file_data, file_size, 0);

    FILE *file = fopen(filename, "wb");
    fwrite(file_data, 1, file_size, file);
    fclose(file);

    set_text_color("\033[1;32m");
    printf("Success! Image saved as %s\n", filename);
    reset_text_color();

    free(file_data);
}
```
-- image_server.c
```c
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
```
- function untuk Program image_client.c yang disajikan dalam bentuk menu kreatif yang memperbolehkan pengguna untuk memasukkan perintah berkali-kali.
-- function tampilan menu
```c
void clear_screen() {
    printf("\033[H\033[J");
}

void set_text_color(const char *color) {
    printf("%s", color);
}

void reset_text_color() {
    printf("\033[0m");
}

void display_menu() {
    set_text_color("\033[1;35m");
    printf("══════════════════════════════════════════\n");
    printf("          |  Image Decoder Client  |         \n");
    printf("══════════════════════════════════════════\n");
    reset_text_color();

    printf("╔════════════════════════════════════════╗\n");
    printf("║        Choose an option below:         ║\n");
    printf("╚════════════════════════════════════════╝\n");

    set_text_color("\033[1;32m");
    printf("┌────────────────────────────────────────┐\n");
    printf("│  1. Send input file to server          │\n");
    printf("│  2. Download file from server          │\n");
    printf("│  3. Exit                               │\n");
    printf("└────────────────────────────────────────┘\n");
    reset_text_color();

    printf("\nPlease enter your command: ");
}
```
-- function agar user bisa input many command
```c
void show_menu(int socket) {
    int choice;
    char filename[100];

    while (1) {
        display_menu();

        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1:
                printf("Enter the file name: ");
                scanf("%s", filename);
                decrypt_file(socket, filename);
                break;
            case 2:
                printf("Enter the file name: ");
                scanf("%s", filename);
                download_file(socket, filename);
                break;
            case 3:
                printf("Exiting...\n");
                close(socket);
                return;
            default:
                set_text_color("\033[1;31m");
                printf("Invalid choice! Please select again.\n");
                reset_text_color();
        }
    }
}
```
- Program image_server.c diharuskan untuk tidak keluar/terminate saat terjadi error dan client akan menerima error message sebagai response
 Dari Client:
- Gagal connect ke server
```c
 if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        print_error_message("Gagal connect ke server");
        return -1;
    }
```

- Salah nama text file input
```c
   if (stat(filepath, &st) != 0) {
        set_text_color("\033[1;31m");  
        printf("File %s not found!\n", filepath);
        reset_text_color();
        return;
    }
```
Dari Server:
- Gagal menemukan file untuk dikirim ke client
```c

 if (file == NULL) {
         const char *error_msg = "Error: File not found";
         send(client_socket, error_msg, strlen(error_msg), 0);
         return;
```


## Revisi

Kekurangan pada Program, Tidak ada function untuk menyimpan server_log atau percakapan yang berlangsung antara server dengan client.
-Menambahkan function Server_log pada image_server.c:
```c
void server_Log(const char *source, const char *action, const char *info) {
    FILE *log_file = fopen("server.log", "a");
    if (log_file != NULL) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        
        char timestamp[20];
        snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec);

        fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
        fclose(log_file);
    } else {
        perror("Failed to open log file");
    }
}
```
Final code image_server.c setelah revisi:
```c
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
    
    if (chdir("/home/auriga/Documents/Sisop-3-2025-IT20/soal_1/server") < 0) {
        perror("Chdir failed");
        exit(EXIT_FAILURE);
    }
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    openlog("image_server_daemon", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Daemon started successfully"); 
}
void server_Log(const char *source, const char *action, const char *info) {
    FILE *log_file = fopen("server.log", "a");
    if (log_file != NULL) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        
        char timestamp[20];
        snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec);

        fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
        fclose(log_file);
    } else {
        perror("Failed to open log file");
    }
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
    server_Log("Client", "DECRYPT", "Text data");
    server_Log("Server", "SAVE", "File saved as [filename].jpeg");

    free(decoded_data);  
}

 void send_file(int client_socket, char *filename) {
    char path[100];
    snprintf(path, sizeof(path), "database/%s", filename);
     FILE *file = fopen(path, "rb");
     if (file == NULL) {
         const char *error_msg = "Error: File not found";
         send(client_socket, error_msg, strlen(error_msg), 0);
         server_Log("Client", "DOWNLOAD", "File not found");
         server_Log("Server", "UPLOAD", "File not found");
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
     server_Log("Client", "DOWNLOAD", filename);
     server_Log("Server", "UPLOAD", filename);
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
                server_Log("Client", "ERROR", "Invalid command received");
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
```

# Soal 2
a. Mengunduh File Order dan Menyimpannya ke Shared Memory
```
if (argc == 1) {
    FILE *file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Gagal membuka file CSV");
        exit(1);
    }

    int i = 0;
    while (fscanf(file, "%[^,],%[^,],%[^\n]\n",
                  orders[i].nama_penerima,
                  orders[i].alamat_tujuan,
                  orders[i].jenis_pengiriman) != EOF) {
        orders[i].status = 0;
        strcpy(orders[i].agen, "-");
        i++;
    }

    fclose(file);
    printf("Pesanan berhasil dimuat ke shared memory.\n");
}
```





# Soal 3

# Soal 4
