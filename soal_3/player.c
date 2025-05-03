#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "shop.h"

char player_name[50];

void show_menu() {
    printf("\n=== THE LOST DUNGEON ===\n");
    printf("1. Show Player Stats\n");
    printf("2. Shop\n");
    printf("3. View Inventory\n");
    printf("4. Battle Mode\n");
    printf("5. Exit\n");
    printf("Choose: ");
}

void show_stats(Player *p) {
    printf("\n--- Player Stats ---\n");
    printf("Name: %s\n", p->name);
    printf("Gold: %d\n", p->gold);
    printf("Base Damage: %d\n", p->base_damage);
    printf("Equipped Weapon: %s\n", p->equipped_weapon.name);
    printf("Weapon Damage: %d\n", p->equipped_weapon.damage);
    if (p->equipped_weapon.has_passive)
        printf("Passive: %s\n", p->equipped_weapon.passive_desc);
    printf("Enemies Defeated: %d\n", p->enemy_killed);
}

void open_shop() {
    WeaponList *list = get_shop_weapons_1(NULL, NULL);
    printf("\n--- Weapon Shop ---\n");
    for (int i = 0; i < list->count; i++) {
        printf("[%d] %s - %d gold, %d damage", i, list->weapons[i].name, list->weapons[i].price, list->weapons[i].damage);
        if (list->weapons[i].has_passive)
            printf(" [Passive: %s]", list->weapons[i].passive_desc);
        printf("\n");
    }
    printf("Enter weapon ID to buy: ");
    int id;
    scanf("%d", &id);
    BuyRequest req;
    strcpy(req.player_name, player_name);
    req.weapon_id = id;
    int *res = buy_weapon_1(&req, NULL);
    if (*res == 0) printf("Weapon bought successfully!\n");
    else printf("Failed to buy weapon.\n");
}

void view_inventory() {
    Player *p = get_player_stats_1(&player_name, NULL);
    printf("\n--- Inventory ---\n");
    printf("Current Weapon: %s\n", p->equipped_weapon.name);
    printf("Damage: %d\n", p->equipped_weapon.damage);
    if (p->equipped_weapon.has_passive)
        printf("Passive: %s\n", p->equipped_weapon.passive_desc);
}

void battle_mode() {
    printf("\n--- Battle Mode ---\n");
    BattleResult *res = battle_1(&player_name, NULL);
    printf("Enemy HP: %d\n", res->enemy_hp);
    printf("You dealt %d damage!\n", res->damage);
    if (res->crit) printf("Critical hit!\n");
    if (res->passive_triggered) printf("Passive activated!\n");
    if (res->reward > 0) printf("You defeated the enemy! Earned %d gold.\n", res->reward);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <player_name>\n", argv[0]);
        return 1;
    }

    strcpy(player_name, argv[1]);
    Player *player = register_player_1(&player_name, NULL);
    if (!player) {
        printf("Server full or error connecting.\n");
        return 1;
    }

    int choice;
    while (1) {
        show_menu();
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                show_stats(get_player_stats_1(&player_name, NULL));
                break;
            case 2:
                open_shop();
                break;
            case 3:
                view_inventory();
                break;
            case 4:
                battle_mode();
                break;
            case 5:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid option. Try again.\n");
        }
    }
}