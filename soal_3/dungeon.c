#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "shop.h"

#define MAX_PLAYER 10

struct Player {
    char name[50];
    int gold;
    int base_damage;
    Weapon equipped_weapon;
    int enemy_killed;
};

struct Player players[MAX_PLAYER];
int player_count = 0;

Player* register_player_1_svc(char **name, struct svc_req *req) {
    if (player_count >= MAX_PLAYER) return NULL;
    strcpy(players[player_count].name, *name);
    players[player_count].gold = 500;
    players[player_count].base_damage = 10;
    players[player_count].equipped_weapon = default_weapon();
    players[player_count].enemy_killed = 0;
    return &players[player_count++];
}

Player* get_player_stats_1_svc(char **name, struct svc_req *req) {
    for (int i = 0; i < player_count; i++) {
        if (strcmp(players[i].name, *name) == 0) {
            return &players[i];
        }
    }
    return NULL;
}

WeaponList* get_shop_weapons_1_svc(void *arg, struct svc_req *req) {
    return get_shop_weapon_list();
}

int* buy_weapon_1_svc(BuyRequest *req, struct svc_req *rqstp) {
    static int result;
    for (int i = 0; i < player_count; i++) {
        if (strcmp(players[i].name, req->player_name) == 0) {
            result = buy_weapon(&players[i], req->weapon_id);
            return &result;
        }
    }
    result = -1;
    return &result;
}

BattleResult* battle_1_svc(char **name, struct svc_req *req) {
    static BattleResult result;
    for (int i = 0; i < player_count; i++) {
        if (strcmp(players[i].name, *name) == 0) {
            int enemy_hp = (rand() % 151) + 50;
            int total_damage = players[i].base_damage + players[i].equipped_weapon.damage;
            int crit = rand() % 100 < 20 ? 2 : 1;
            int passive_bonus = (players[i].equipped_weapon.has_passive && rand() % 100 < 25) ? 5 : 0;
            int final_dmg = (total_damage + passive_bonus) * crit;

            enemy_hp -= final_dmg;
            result.damage = final_dmg;
            result.enemy_hp = enemy_hp > 0 ? enemy_hp : 0;
            if (enemy_hp <= 0) {
                int reward = (rand() % 51) + 50;
                players[i].gold += reward;
                players[i].enemy_killed++;
                result.reward = reward;
            } else {
                result.reward = 0;
            }
            result.crit = crit == 2;
            result.passive_triggered = passive_bonus > 0;
            return &result;
        }
    }
    return NULL;
}

int main() {
    register SVCXPRT *transp;
    pmap_unset(DUNGEON_PROG, DUNGEON_VERS);

    transp = svcudp_create(RPC_ANYSOCK);
    if (!transp) {
        fprintf(stderr, "cannot create udp service.\n");
        exit(1);
    }
    svc_register(transp, DUNGEON_PROG, DUNGEON_VERS, dungeon_prog_1, IPPROTO_UDP);

    svc_run();
    fprintf(stderr, "svc_run returned\n");
    exit(1);
}