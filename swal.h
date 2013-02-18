#ifndef SWAL_H
#define SWAL_H

#define SWAL_TYPE_LEGACY 0
#define SWAL_TYPE_MODERN 1
#define SWAL_TYPE_BOTH 2

typedef struct swal_con {
	int legacy_sock;
	int modern_sock;
	char apiKey[64];
} swal_con;

// Public Functions
swal_con* swal_connect(int type, char* apiKey);
void swal_disconnect(swal_con* connection);
int swal_get_player_summaries(swal_con* connection, char* steamIDs, char* filename); // Modern API only
int swal_get_friend_list(swal_con* connection, char* steamID, char* filename); // Modern API only
int swal_get_player_achievements(swal_con* connection, int appID, char* steamID, char* filename); // Modern API only
int swal_get_player_grouplist(swal_con* connection, char* steamID, char* filename); // Modern API only
int swal_get_player_profile(swal_con* connection, char* steamID, char* filename); // Legacy API only
int swal_get_group_memberlist(swal_con* connection, char* groupID, char* filename); // Legacy API only
int swal_get_player_games(swal_con* connection, char* steamID, char* filename); // Legacy API only
unsigned long int getSteamID64(char* steamID32);
char* generateIDs(int type, int startID);

#endif

