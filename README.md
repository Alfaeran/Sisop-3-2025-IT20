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
**dispatcher.c**
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
```
library c, 

```<sys/ipc.h>``` Untuk menggunakan key_t dan fungsi ftok() yang menghasilkan kunci unik shared memory.

```<sys/shm.h>``` Untuk menggunakan fungsi shared memory seperti shmget(), shmat(), shmdt().

```<time.h>``` Untuk fungsi waktu seperti time(), localtime(), dan strftime() untuk membuat timestamp pada log.
```
#define MAX_ORDER 100

typedef struct {
char nama_penerima[100];
char alamat_tujuan[100];
char jenis_pengiriman[10];
int status;
char agen[50];
} DataPesanan;
```
Menyimpan data satu pesanan yang berisi: nama, alamat, jenis (Reguler/Express), status (0 = pending, 1 = delivered), dan agen pengantar.

```
void catat_ke_log(const char *agen, const char *tipe, const char *nama, const char *alamat) {
FILE *log = fopen("delivery.log", "a");

if (!log) {
perror("Gagal membuka delivery.log");
return;
}

time_t now = time(NULL);
struct tm *t = localtime(&now);
fprintf(log, "[%02d/%02d/%d %02d:%02d:%02d] [%s] %s package delivered to %s in %s\n",
t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
t->tm_hour, t->tm_min, t->tm_sec,
agen, tipe, nama, alamat);
fclose(log);
}
```
Menulis log ke file delivery.log .

```
int main(int argc, char *argv[]) {
key_t key = ftok("delivery_order.csv", 65);
int shmid = shmget(key, sizeof(DataPesanan) * MAX_ORDER, IPC_CREAT | 0666);

if (shmid == -1) {
perror("Gagal membuat shared memory");
exit(1);
}

DataPesanan *orders = (DataPesanan *) shmat(shmid, NULL, 0);
if (orders == (void *)-1) {
perror("Kesalahan saat menautkan shared memory");
exit(1);
}
```
Program ini akan menghasilkan key shared memory dari file delivery_order.csv, mengalokasikan shared memory untuk 100 pesanan dan ```shmat()``` menempelkan shared memory ke pointer orders.



**a. Mengunduh File Order dan Menyimpannya ke Shared Memory**
```
if (argc == 1) {
FILE *file = fopen("delivery_order.csv", "r");
if (!file) {
perror("Gagal membuka file CSV");
exit(1);
}

int i = 0;
char header[256];
fgets(header, sizeof(header), file);

while (fscanf(file, "%[^,],%[^,],%s\n",
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
Program ini akan Membuka isi file CSV ```FILE *file = fopen("delivery_order.csv", "r");```

``` while (fscanf(file, "%[^,],%[^,],%[^\n]\n", (...) ``` Membaca file CSV dan memasukkan setiap pesanan ke dalam orders[i].

```status = 0``` Menandai bahwa pesanan belum dikirim dan agen dengan tanda ```"-"```karena belum ada agen yang ditugaskan untuk mengantar pesanan itu.

**c. Pengiriman Bertipe Reguler**
```
else if (argc == 3 && strcmp(argv[1], "-deliver") == 0) {
char *target = argv[2];
int ditemukan = 0;

for (int i = 0; i < MAX_ORDER; i++) {
if (strcmp(orders[i].nama_penerima, target) == 0 &&
strcmp(orders[i].jenis_pengiriman, "Reguler") == 0 &&
orders[i].status == 0) {

orders[i].status = 1;
snprintf(orders[i].agen, sizeof(orders[i].agen), "AGENT %s", target);
catat_ke_log(orders[i].agen, "Reguler", orders[i].nama_penerima, orders[i].alamat_tujuan);
printf("Pesanan %s berhasil dikirim oleh %s.\n", orders[i].nama_penerima, orders[i].agen);
ditemukan = 1;
break;
}
}

if (!ditemukan) {
printf("Pesanan Reguler untuk %s tidak ditemukan atau sudah dikirim.\n", target);
}
}
```
Program akan mencari pesanan Reguler dengan nama yang cocok dan belum dikirim. Jika ditemukan, pesanan ditandai sebagai terkirim, nama agen dicatat, dan log dicetak ke delivery.log.
Jika tidak ditemukan, ditampilkan pesan bahwa pesanan tidak ada atau sudah dikirim.

**d. Mengecek Status Pesanan**
```
else if (argc == 3 && strcmp(argv[1], "-status") == 0) {
char *target = argv[2];
int ditemukan = 0;

for (int i = 0; i < MAX_ORDER; i++) {
if (strcmp(orders[i].nama_penerima, target) == 0) {
ditemukan = 1;

if (orders[i].status == 0) {
printf("Status untuk %s: Pending\n", orders[i].nama_penerima);
} else {
printf("Status untuk %s: Dikirim oleh %s\n", orders[i].nama_penerima, orders[i].agen);
}
break;
}
}

if (!ditemukan) {
printf("Pesanan untuk %s tidak ditemukan.\n", target);
}
}
```
Kode ini dijalankan dengan mengetik ./dispatcher -status <nama>.

Program akan mencari pesanan dengan nama tersebut dan menampilkan statusnya:

- Jika belum dikirim: Pending
  
- Jika sudah dikirim: ditampilkan siapa agennya
  
- Jika pesanan tidak ditemukan, akan muncul pesan kesalahan

**e. Melihat Daftar Semua Pesanan**
```
else if (argc == 2 && strcmp(argv[1], "-list") == 0) {
for (int i = 0; i < MAX_ORDER; i++) {
if (strlen(orders[i].nama_penerima) == 0) continue;

printf("[%s] %s %s %s %s\n",
orders[i].jenis_pengiriman,
orders[i].status == 0 ? "\033[1;31mPending\033[0m" : "\033[1;32mDelivered\033[0m",
orders[i].nama_penerima,
orders[i].alamat_tujuan,
orders[i].agen);
}
}

shmdt(orders);
return 0;
}
```
Kode ini dijalankan dengan mengetik ./dispatcher -list.

Program akan mencetak semua pesanan yang ada di shared memory seperti: Jenis pengiriman, Status (Pending atau Delivered), Nama penerima, Alamat tujuan dan Agen pengantar.

**delivery_agent.c**

Program ini akan membaca pesanan dari shared memory, menjalankan 3 thread agen pengantar Express (AGENT A, B, dan C).

Tiap agen otomatis:

- Mencari pesanan Express yang belum dikirim

- Mengirim pesanan

- Mencatat ke delivery.log
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_ORDER 100
```
Menggunakan library untuk I/O seperti threading (pthread), shared memory (sys/shm.h), dan waktu (time.h).

```
typedef struct {
char nama_penerima[100];
char alamat_tujuan[100];
char jenis_pengiriman[10];
int status;
char agen[50];
} DataPesanan;
```
Menyimpan data satu pesanan yang berisi: nama, alamat, jenis (Reguler/Express), dan agen pengantar.

```
DataPesanan *orders;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
```
- orders menunjuk ke shared memory berisi array pesanan.

- lock digunakan agar akses antar thread tidak saling tumpang tindih (race condition)

```
void catat_ke_log(const char *agent, const char *nama, const char *alamat) {
FILE *log = fopen("delivery.log", "a");

if (!log) {
perror("Gagal membuka delivery.log");
return;
}

time_t now = time(NULL);
struct tm *t = localtime(&now);
fprintf(log, "[%02d/%02d/%d %02d:%02d:%02d] [%s] Express package delivered to %s in %s\n",
t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
t->tm_hour, t->tm_min, t->tm_sec,
agent, nama, alamat);
fclose(log);
}
```

Menulis log pengiriman Express ke file delivery.log .

```
void *agent_express(void *arg) {
char *agent_name = (char *)arg;

while (1) {
pthread_mutex_lock(&lock);
for (int i = 0; i < MAX_ORDER; i++) {
if (strcmp(orders[i].jenis_pengiriman, "Express") == 0 && orders[i].status == 0) {
orders[i].status = 1;
snprintf(orders[i].agen, sizeof(orders[i].agen), "%s", agent_name);
catat_ke_log(agent_name, orders[i].nama_penerima, orders[i].alamat_tujuan);
printf("[%s] Mengirim pesanan: %s (%s)\n", agent_name, orders[i].nama_penerima, orders[i].alamat_tujuan);
}
}

pthread_mutex_unlock(&lock);
sleep(1);
}
return NULL;
}
```

Program akan mencari order dengan ```jenis_pengiriman == "Express"``` dan ```status == 0```, kemudian tandai sudah dikirim. Lalu mengisi nama agen dan menulisnya kedalam log kemudian akan tampilkan di terminal. Loop terus setiap detik.

```
void mulai_thread_agen() {
pthread_t agents[3];
char *names[3] = { "AGENT A", "AGENT B", "AGENT C" };

for (int i = 0; i < 3; i++) {
pthread_create(&agents[i], NULL, agent_express, names[i]);
}

for (int i = 0; i < 3; i++) {
pthread_join(agents[i], NULL);
}
}
```
Program akan Membuat 3 thread untuk AGENT A, B, dan C dan semua menjalankan agent_express.

```
int main() {
key_t key = ftok("delivery_order.csv", 65);
int shmid = shmget(key, sizeof(DataPesanan) * MAX_ORDER, 0666);
if (shmid == -1) {
perror("Gagal mendapatkan shared memory");
exit(1);
}

orders = (DataPesanan *)shmat(shmid, NULL, 0);
if (orders == (void *)-1) {
perror("Kesalahan saat menautkan shared memory");
exit(1);
}

mulai_thread_agen();
shmdt(orders);
return 0;
}
```
Mengakses shared memory berisi pesanan kemudian menjalankan 3 thread agen.
```

# Soal 3
## Laporan Proyek "The Lost Dungeon"


Game ini dibangun dalam bahasa C dan terdiri dari 4 file utama:
shop.h, shop.c, dungeon.c, player.c

**1. shop.h**

Isi

Header file ini mendefinisikan semua struktur data dan deklarasi fungsi yang digunakan oleh shop.c, player.c, dan dungeon.c.

Struktur yang Didefinisikan

- Weapon – menyimpan info nama, damage, harga, passive effect
- WeaponList – kumpulan senjata di toko
- Player – status pemain
- BuyRequest – struktur pembelian senjata
- BattleResult – hasil pertempuran
- Fungsi Dideklarasikan
- void init_shop();
- WeaponList* get_shop_weapon_list();
- int buy_weapon(Player*, int);
- Weapon default_weapon();

Penggunaan

Semua file .c yang menggunakan struktur dan fungsi ini harus #include "shop.h"

**2. shop.c**

Isi

Mengimplementasikan semua fungsi yang berkaitan dengan senjata dan toko.

Fungsi Utama

- init_shop() – mengisi daftar senjata di toko (5 jenis senjata awal)
- get_shop_weapon_list() – mengembalikan daftar senjata
- buy_weapon(Player*, int) – memproses pembelian senjata oleh pemain
- default_weapon() – senjata awal ("Fist")


**3. dungeon.c**

Isi

Berperan sebagai "server statis" yang menyimpan daftar pemain dan logika pertempuran.

Fungsi Utama

- register_player() – mendaftarkan pemain baru
- get_player_stats() – mengembalikan status pemain
- get_shop_weapons() – memanggil daftar senjata dari shop
- buy_weapon_for_player() – membeli senjata berdasarkan request
- battle() – memproses logika pertempuran

Penggunaan

Dipanggil oleh player.c melalui extern agar pemain bisa interaksi dengan server.


**4. player.c**

Isi

Program utama yang dijalankan user. Mewakili client.

Fungsi

- main() – menerima nama pemain dari argumen terminal
- show_menu() – menampilkan menu utama
- show_stats() – menampilkan status pemain
- open_shop() – menampilkan daftar senjata, menerima input, memproses pembelian
- view_inventory() – menampilkan senjata yang sedang dipakai
- battle_mode() – menampilkan hasil pertarungan


Jalankan:
```./game Rik```

Output Contoh
```
=== THE LOST DUNGEON ===
1. Show Player Stats
2. Weapon Shop
3. View Inventory
4. Battle Mode
5. Exit
Choose: 2

--- Weapon Shop ---
[0] Rusty Sword - 100 gold, 5 dmg
[1] Steel Blade - 200 gold, 10 dmg
[2] Flame Dagger - 300 gold, 8 dmg [Passive: +5 dmg chance on hit]
[3] Thunder Axe - 400 gold, 12 dmg [Passive: Chance to stun enemy]
[4] Shadow Katana - 500 gold, 15 dmg
[-1] Cancel
Enter weapon ID to buy:
```

# Soal 4
## Laporan Proyek Sistem Hunter - SISOP 2025

Fitur utama:
•	Registrasi dan login Hunter
•	Manajemen Dungeon (generate, tampilkan, raid)
•	PVP antar Hunter
•	Fitur admin: melihat info Hunter dan Dungeon, ban Hunter, reset status
•	Fitur notifikasi dungeon berkala


Penggunaan


**1. Jalankan Program System (Admin)**
```./system```
Ini akan memulai sistem dan membuat Shared Memory yang dibutuhkan.


**2. Jalankan Program Hunter**

```./hunter```
Hunter dapat mendaftar, login, dan berinteraksi dengan dungeon maupun hunter lain.


Fitur System.c
•	Menampilkan semua data Hunter
•	Menampilkan semua Dungeon
•	Generate dungeon secara acak
•	Ban/unban Hunter tertentu
•	Reset stat Hunter ke awal
•	Hapus Shared Memory saat keluar


Fitur Hunter.c
•	Registrasi akun baru
•	Login akun
•	Melihat dungeon yang tersedia sesuai level
•	Melakukan raid dungeon dan mendapat reward
•	Melakukan PVP dan mengambil semua stat lawan
•	Notifikasi dungeon muncul setiap 3 detik (jika diaktifkan)


Contoh Output
```
=== SYSTEM MENU ===
1. Generate Dungeon
2. View Dungeons
3. Reset Dungeons
4. Ban Hunter
5. Exit
Choice: 1
25 dungeons generated.

=== SYSTEM MENU ===
1. Generate Dungeon
2. View Dungeons
3. Reset Dungeons
4. Ban Hunter
5. Exit
Choice: 2

=== DUNGEONS ===
[1] Red Gate Dungeon (Level 1+) - EXP: 86, ATK: 10, HP: 45, DEF: 5
[2] Blue Gate Dungeon (Level 1+) - EXP: 66, ATK: 20, HP: 35, DEF: 10
[3] Black Gate Dungeon (Level 1+) - EXP: 76, ATK: 14, HP: 32, DEF: 9
[4] Black Gate Dungeon (Level 3+) - EXP: 108, ATK: 80, HP: 176, DEF: 21
[5] Black Gate Dungeon (Level 9+) - EXP: 150, ATK: 46, HP: 112, DEF: 16
[6] Black Gate Dungeon (Level 10+) - EXP: 115, ATK: 33, HP: 174, DEF: 23
[7] Black Gate Dungeon (Level 2+) - EXP: 149, ATK: 57, HP: 192, DEF: 11
[8] Black Gate Dungeon (Level 4+) - EXP: 82, ATK: 113, HP: 158, DEF: 26
[9] Blue Gate Dungeon (Level 1+) - EXP: 106, ATK: 20, HP: 114, DEF: 14
[10] Red Gate Dungeon (Level 2+) - EXP: 100, ATK: 49, HP: 57, DEF: 23
[11] Blue Gate Dungeon (Level 7+) - EXP: 121, ATK: 85, HP: 114, DEF: 23
[12] Blue Gate Dungeon (Level 4+) - EXP: 112, ATK: 46, HP: 110, DEF: 22
[13] Blue Gate Dungeon (Level 8+) - EXP: 113, ATK: 101, HP: 100, DEF: 21
[14] Black Gate Dungeon (Level 4+) - EXP: 69, ATK: 79, HP: 69, DEF: 18
[15] Blue Gate Dungeon (Level 5+) - EXP: 56, ATK: 89, HP: 144, DEF: 18
[16] Black Gate Dungeon (Level 10+) - EXP: 132, ATK: 105, HP: 114, DEF: 30
[17] Blue Gate Dungeon (Level 8+) - EXP: 51, ATK: 71, HP: 187, DEF: 17
[18] Blue Gate Dungeon (Level 10+) - EXP: 114, ATK: 69, HP: 56, DEF: 16
[19] Red Gate Dungeon (Level 4+) - EXP: 140, ATK: 94, HP: 193, DEF: 16
[20] Black Gate Dungeon (Level 8+) - EXP: 139, ATK: 91, HP: 53, DEF: 18
[21] Blue Gate Dungeon (Level 1+) - EXP: 126, ATK: 120, HP: 172, DEF: 23
[22] Black Gate Dungeon (Level 4+) - EXP: 137, ATK: 47, HP: 145, DEF: 30
[23] Black Gate Dungeon (Level 9+) - EXP: 109, ATK: 91, HP: 151, DEF: 22
[24] Blue Gate Dungeon (Level 3+) - EXP: 92, ATK: 51, HP: 79, DEF: 14
[25] Red Gate Dungeon (Level 6+) - EXP: 81, ATK: 114, HP: 141, DEF: 23

1. Register
2. Login
3. Exit
Choice: 1
Enter name: Rok
Registered hunter Rok!

1. Register
2. Login
3. Exit
Choice: 2
Enter name: Rok
Welcome back, Rok!

=== HUNTER MENU ===
1. View Available Dungeons
2. Raid Dungeon
3. Logout
Choice: 1

=== AVAILABLE DUNGEONS ===
Red Gate Dungeon | EXP: 86 | ATK: 10 | HP: 45 | DEF: 5
Blue Gate Dungeon | EXP: 66 | ATK: 20 | HP: 35 | DEF: 10
Black Gate Dungeon | EXP: 76 | ATK: 14 | HP: 32 | DEF: 9
Blue Gate Dungeon | EXP: 106 | ATK: 20 | HP: 114 | DEF: 14
Blue Gate Dungeon | EXP: 126 | ATK: 120 | HP: 172 | DEF: 23
```
