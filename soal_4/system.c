#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_HUNTERS 10
#define MAX_DUNGEONS 25

typedef struct {
    char name[50];
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int banned;
} Hunter;

typedef struct {
    char name[50];
    int min_level;
    int reward_exp;
    int atk;
    int hp;
    int def;
    long key;
} Dungeon;

typedef struct {
    Hunter hunters[MAX_HUNTERS];
    int hunter_count;
    Dungeon dungeons[MAX_DUNGEONS];
    int dungeon_count;
} SharedData;

void generate_dungeon(SharedData *data) {
    data->dungeon_count = 0;

    const char *base_names[] = {"Red Gate Dungeon", "Blue Gate Dungeon", "Black Gate Dungeon"};
    int base_name_count = sizeof(base_names) / sizeof(base_names[0]);

    // Tambahkan 3 dungeon awal level 1 untuk leveling
    for (int i = 0; i < 3 && data->dungeon_count < MAX_DUNGEONS; i++) {
        Dungeon *d = &data->dungeons[data->dungeon_count++];
        snprintf(d->name, sizeof(d->name), "%s", base_names[i]);
        d->min_level = 1;
        d->reward_exp = 50 + rand() % 51; // 50-100
        d->atk = 10 + rand() % 11; // 10-20
        d->hp = 30 + rand() % 21; // 30-50
        d->def = 5 + rand() % 6; // 5-10
    }

    // Tambahkan dungeon lainnya
    while (data->dungeon_count < MAX_DUNGEONS) {
        Dungeon *d = &data->dungeons[data->dungeon_count++];
        const char *base = base_names[rand() % base_name_count];
        snprintf(d->name, sizeof(d->name), "%s", base);
        d->min_level = 1 + rand() % 10;
        d->reward_exp = 50 + rand() % 101;
        d->atk = 20 + rand() % 101;
        d->hp = 50 + rand() % 151;
        d->def = 10 + rand() % 21;
    }

    printf("%d dungeons generated.\n", data->dungeon_count);
}

void reset_hunters(SharedData *data) {
    data->dungeon_count = 0;
    printf("All dungeons have been reset.\n");
}

void ban_hunter(SharedData *data) {
    printf("\n=== HUNTER LIST ===\n");
    for (int i = 0; i < data->hunter_count; i++) {
        if (!data->hunters[i].banned) {
            printf("[%d] %s (Lv%d)\n", i + 1, data->hunters[i].name, data->hunters[i].level);
        }
    }
    printf("Enter hunter index to ban: ");
    int index;
    scanf("%d", &index);
    if (index < 1 || index > data->hunter_count) {
        printf("Invalid index.\n");
        return;
    }
    data->hunters[index - 1].banned = 1;
    printf("Hunter %s has been banned.\n", data->hunters[index - 1].name);
}

int main() {
    key_t key = ftok("shared", 65);
    int shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    int choice;
    do {
        printf("\n=== SYSTEM MENU ===\n");
        printf("1. Generate Dungeon\n");
        printf("2. View Dungeons\n");
        printf("3. Reset Dungeons\n");
        printf("4. Ban Hunter\n");
        printf("5. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            srand(time(NULL));
            generate_dungeon(data);
        } else if (choice == 2) {
            printf("\n=== DUNGEONS ===\n");
            for (int i = 0; i < data->dungeon_count; i++) {
                Dungeon *d = &data->dungeons[i];
                printf("[%d] %s (Level %d+) - EXP: %d, ATK: %d, HP: %d, DEF: %d\n",
                       i + 1, d->name, d->min_level, d->reward_exp, d->atk, d->hp, d->def);
            }
        } else if (choice == 3) {
            reset_hunters(data);
        } else if (choice == 4) {
            ban_hunter(data);
        }
    } while (choice != 5);

    shmdt(data);
    return 0;
}
