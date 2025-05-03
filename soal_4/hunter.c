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

int main() {
    key_t key = ftok("shared", 65);
    int shmid = shmget(key, sizeof(SharedData), 0666);
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
    char name[50];
    Hunter *current = NULL;

    while (1) {
        printf("\n1. Register\n2. Login\n3. Exit\nChoice: ");
        scanf("%d", &choice);
        getchar();
        if (choice == 1) {
            if (data->hunter_count >= MAX_HUNTERS) {
                printf("Hunter list full.\n");
                continue;
            }
            printf("Enter name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = 0;
            int exists = 0;
            for (int i = 0; i < data->hunter_count; i++) {
                if (strcmp(data->hunters[i].name, name) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (exists) {
                printf("Hunter already exists.\n");
            } else {
                Hunter *h = &data->hunters[data->hunter_count++];
                strcpy(h->name, name);
                h->level = 1;
                h->exp = 0;
                h->atk = 20;
                h->hp = 100;
                h->def = 10;
                h->banned = 0;
                printf("Registered hunter %s!\n", name);
            }
        } else if (choice == 2) {
            printf("Enter name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = 0;
            current = NULL;
            for (int i = 0; i < data->hunter_count; i++) {
                if (strcmp(data->hunters[i].name, name) == 0 && !data->hunters[i].banned) {
                    current = &data->hunters[i];
                    break;
                }
            }
            if (current == NULL) {
                printf("Hunter not found or banned.\n");
                continue;
            }
            printf("Welcome back, %s!\n", current->name);
            while (1) {
                printf("\n=== HUNTER MENU ===\n");
                printf("1. View Available Dungeons\n");
                printf("2. Raid Dungeon\n");
                printf("3. Logout\n");
                printf("Choice: ");
                scanf("%d", &choice);
                getchar();
                if (choice == 1) {
                    printf("\n=== AVAILABLE DUNGEONS ===\n");
                    for (int i = 0; i < data->dungeon_count; i++) {
                        Dungeon *d = &data->dungeons[i];
                        if (current->level >= d->min_level) {
                            printf("%s | EXP: %d | ATK: %d | HP: %d | DEF: %d\n",
                                d->name, d->reward_exp, d->atk, d->hp, d->def);
                        }
                    }
                } else if (choice == 2) {
                    char dname[50];
                    printf("Enter dungeon name: ");
                    fgets(dname, sizeof(dname), stdin);
                    dname[strcspn(dname, "\n")] = 0;
                    int found = 0;
                    for (int i = 0; i < data->dungeon_count; i++) {
                        Dungeon *d = &data->dungeons[i];
                        if (strcmp(d->name, dname) == 0 && current->level >= d->min_level) {
                            current->exp += d->reward_exp;
                            if (current->exp >= current->level * 100) {
                                current->exp -= current->level * 100;
                                current->level++;
                                current->atk += 10;
                                current->hp += 20;
                                current->def += 5;
                                printf("Leveled up to %d!\n", current->level);
                            }
                            printf("Dungeon cleared!\n");
                            // remove dungeon after raid
                            for (int j = i; j < data->dungeon_count - 1; j++) {
                                data->dungeons[j] = data->dungeons[j + 1];
                            }
                            data->dungeon_count--;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        printf("Dungeon not found or level too low.\n");
                    }
                } else if (choice == 3) {
                    break;
                }
            }
        } else if (choice == 3) {
            break;
        }
    }

    shmdt(data);
    return 0;
}
