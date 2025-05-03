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
