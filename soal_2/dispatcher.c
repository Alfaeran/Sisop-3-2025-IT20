#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_ORDER 100

typedef struct {
char nama_penerima[100];
char alamat_tujuan[100];
char jenis_pengiriman[10];
int status;
char agen[50];
} DataPesanan;


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

