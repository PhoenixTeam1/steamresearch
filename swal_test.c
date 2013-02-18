#include "swal.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

char* readKey();

int main(int argc, char* argv[]) {
	int i;
	int index;
	char* buffer;
	char filename[sizeof("Data/PlayerSummaries_Exhaustive/player_summaries_10000000.json")];

	// Load API Key
	char* key = readKey();

	// Some Modern API calls
	swal_con* conn = swal_connect(SWAL_TYPE_BOTH, key);
	if (!conn) {
		printf("Connection failed\n");
	}
	for (i = 39723; i < 300000; i++) {
		index = i*100;
		sprintf(filename, "Data/PlayerSummaries_Exhaustive/player_summaries_%08d.json", index);
		printf("%s\n",filename);
		buffer = generateIDs(1, index);
		swal_get_player_summaries(conn, buffer, filename);
		free(buffer);
		sleep(1);
	}
	swal_disconnect(conn);
//	swal_get_friend_list(conn, "76561197961965701", "friendlist.json");
//	swal_get_player_achievements(conn, 440, "76561197961965701", "achievements.json");
//	swal_get_player_grouplist(conn, "76561197961965701", "grouplist.json");

	// Some Legacy API calls
//	int returnvalue = swal_get_player_profile(conn, "76561197961965701", "testfile.xml");
//	printf("%d",returnvalue);
//	swal_get_group_memberlist(conn, "103582791432996535", "memberlist.xml");
//	unsigned long int steamID64 = getSteamID64("STEAM_0:1:849986");
	//printf("%lu\n", steamID64);
	free(key);
	return 0;
}

char* readKey() {
	long int length;
	char* key;
	size_t bytesRead;
	FILE* keyfile = fopen("steamapikey", "r");
	if (keyfile == NULL) {
		fprintf(stderr, "You are missing your Steam API Key file.  Please place your key inside a text file named steamapikey and put that file in the same working directory as the program.\n");
		return 0;
	}

	// Get byte length of key
	fseek(keyfile, 0, SEEK_END);
	length = ftell(keyfile);
	rewind(keyfile);

	// Get key
	key = malloc(length);
	bytesRead = fread(key, 1, length, keyfile);
	if (bytesRead != length) {
		fprintf(stderr, "Failed to read contents of key file.\n");
		return 0;
	}

	fclose(keyfile);
	return key;
}

