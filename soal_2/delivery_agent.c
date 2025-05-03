#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
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

DataPesanan *orders;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


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
