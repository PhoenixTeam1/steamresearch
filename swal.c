#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "swal.h"

#define STEAM_WEB_API_HOSTNAME "api.steampowered.com"
#define STEAM_COMMUNITY_DATA_HOSTNAME "steamcommunity.com"
#define STEAM_WEB_API_SERVICE "http"
#define BUFLEN 512

#define HTTP_STATUS_OK "HTTP/1.1 200 OK"
#define HTTP_CONTENT_LENGTH "Content-Length: "

// Internal functions
int swal_get_socket(int type);
int swal_save_response(int sock, char* filename);
int swal_execute_query(int sock, char* query, char* filename);

///////////////////////// PUBLIC FUNCTIONS //////////////////////////
/**************************************************
  Generate a list of 100 sequential 64-bit Steam
  IDs beginning with the input start ID.

  params:
   -startID: base-10 integer specifying beginning
   of sequence
   -type: 0 or 1 depending on ID type

  return: char buffer of 100 64-bit Steam IDs

  Notes: requires a 64-bit machine.  Will change
  this later if needed.
**************************************************/
char* generateIDs(int type, int startID) {
	unsigned long int steamID64;
	char* buffer = malloc(7000);
	char generator[64];
	
	int i;
	for (i = startID; i < startID+100; i++)	{
		char typeID = (char)(((int)'0')+type);
		char steamid[] = "STEAM_0:";
		int len = strlen(steamid);
		steamid[len] = typeID;
		steamid[len+1] = ':';
		steamid[len+2] = '\0';
		unsigned long int generatedID = i;
		sprintf(generator,"%lu",generatedID);
		strcat(steamid, generator);
		steamID64 = getSteamID64(steamid);

		if ( i == startID) {
			sprintf(buffer,"%lu",steamID64); }

		else {
			sprintf(buffer + strlen(buffer),"%lu",steamID64); }
		
		if ( (i+1) != (startID + 100) ) {
			sprintf(buffer + strlen(buffer), "%C", ','); }
	}

	return buffer;
}

/**************************************************
  Converts a Steam ID 32 string into its 
  Steam ID 64 equivlant.
  [NO API CONNECTION REQUIRED]

  params:
   -steamID32: A valid Steam ID fitting the form
    of the regex: /^STEAM_0:[01]:[0-9]{6,8}$/
    (e.g. STEAM_0:1:29072874)

  return: 64-bit Steam ID 64

  Notes: requires a 64-bit machine.  Will change
  this later if needed.
**************************************************/
unsigned long int getSteamID64(char* steamID32) {
	char buffer[64];
	char* substr;
	unsigned long int steamID64;
	unsigned long int serverID;
	unsigned long int userID;
	strcpy(buffer, steamID32);
	substr = strtok(buffer, ":");
	substr = strtok(NULL, ":");
	sscanf(substr, "%lu", &serverID);
	substr = strtok(NULL, ":");
	sscanf(substr, "%lu", &userID);
	steamID64 = serverID + userID * 2LU + 76561197960265728LU;
	return steamID64;
}


/**************************************************
  Retrieves the profile summaries of a given set
  of users.
  [MODERN API]

  params:
   -connection: A valid swal_con connection 
                pointer
   -steamIDs: A comma-separated string of
              Steam ID 64 of the users
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_get_player_summaries(swal_con* connection, char* steamIDs, char* filename) {
	int steamIDsLength = strlen(steamIDs);
	int success = 0;
	char* buffer = (char*)malloc(steamIDsLength + 256); // 256 Bytes is enough for the base query and API key (static length)
	sprintf(buffer, "GET /ISteamUser/GetPlayerSummaries/v0002/?key=%s&steamids=%s HTTP/1.1\r\nhost: api.steampowered.com\r\n\r\n", connection->apiKey, steamIDs);
	success = swal_execute_query(connection->modern_sock, buffer, filename);
	free(buffer);
	return success;
}


/**************************************************
  Retrieves the list of friends of a given user.
  [MODERN API]

  params:
   -connection: A valid swal_con connection 
                pointer
   -steamID: Steam ID 64 of the user
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_get_friend_list(swal_con* connection, char* steamID, char* filename) {
	int success = 0;
	char buffer[256];
	sprintf(buffer, "GET /ISteamUser/GetFriendList/v0001/?key=%s&steamid=%s&relationship=friend HTTP/1.1\r\nhost: api.steampowered.com\r\n\r\n", connection->apiKey, steamID);
	success = swal_execute_query(connection->modern_sock, buffer, filename);
	return success;
}


/**************************************************
  Retrieves a list of groups of which a given
  user is a member.
  [MODERN API]

  params:
   -connection: A valid swal_con connection 
                pointer
   -steamID: Steam ID 64 of the user
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_get_player_grouplist(swal_con* connection, char* steamID, char* filename) {
	int success = 0;
	char buffer[256];
	sprintf(buffer, "GET /ISteamUser/GetUserGroupList/v0001/?key=%s&steamid=%s HTTP/1.1\r\nhost: api.steampowered.com\r\n\r\n", connection->apiKey, steamID);
	success = swal_execute_query(connection->modern_sock, buffer, filename);
	return success;
}


/**************************************************
  Retrieves the game list of a given user
  [LEGACY API]

  params:
   -connection: A valid swal_con connection 
                pointer
   -steamID: Steam ID 64 of the user
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_get_player_achievements(swal_con* connection, int appID, char* steamID, char* filename) {
	int success = 0;
	char buffer[256];
	sprintf(buffer, "GET /ISteamUserStats/GetPlayerAchievements/v0001/?appid=%d&key=%s&steamid=%s HTTP/1.1\r\nhost: api.steampowered.com\r\n\r\n", appID, connection->apiKey, steamID);
	success = swal_execute_query(connection->modern_sock, buffer, filename);
	return success;
}


/**************************************************
  Retrieves the full profile of a given user
  [LEGACY API]

  params:
   -connection: A valid swal_con connection 
                pointer
   -steamID: Steam ID 64 of the user
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_get_player_profile(swal_con* connection, char* steamID, char* filename) {
	int success = 0;
	char buffer[256];
	sprintf(buffer, "GET /profiles/%s/?xml=1&l=english HTTP/1.1\r\nhost: steamcommunity.com\r\n\r\n", steamID);
	success = swal_execute_query(connection->legacy_sock, buffer, filename);
	return success;
}


/**************************************************
  Retrieves the game list of a given user
  [LEGACY API]

  params:
   -connection: A valid swal_con connection 
                pointer
   -steamID: Steam ID 64 of the user
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_get_player_games(swal_con* connection, char* steamID, char* filename) {
	int success = 0;
	char buffer[256];
	sprintf(buffer, "GET /profiles/%s/games?xml=1 HTTP/1.1\r\nhost: steamcommunity.com\r\n\r\n", steamID);
	success = swal_execute_query(connection->legacy_sock, buffer, filename);
	return success;
}


/**************************************************
  Retrieves a memberlist of a given group ID
  [LEGACY API]

  params:
   -connection: A valid swal_con connection 
                pointer
   -groupID: Steam ID 64 of the group
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_get_group_memberlist(swal_con* connection, char* groupID, char* filename) {
	int success = 0;
	char buffer[256];
	sprintf(buffer, "GET /gid/%s/memberslistxml/?xml=1 HTTP/1.1\r\nhost: steamcommunity.com\r\n\r\n", groupID);
	success = swal_execute_query(connection->legacy_sock, buffer, filename);
	return success;
}


/**************************************************
  This function creates the sockets for future
  API communication.  It must be called before
  any data retrieval functions.

  params:
   -type: Indicates which API to use.  Legal
          values are SWAL_TYPE_LEGACY,
          SWAL_TYPE_MODERN, and SWAL_TYPE_BOTH
   -apiKey: The API key string for the Steam
          Web API.  Not needed if type is
          SWAL_TYPE_LEGACY

  return: A swal_con structure pointer to be
          used in all subsequent library calls
**************************************************/
swal_con* swal_connect(int type, char* apiKey) {
	swal_con* connection = (swal_con*)malloc(sizeof(swal_con));
	if (type == SWAL_TYPE_LEGACY || type == SWAL_TYPE_BOTH) {
		connection->legacy_sock = swal_get_socket(SWAL_TYPE_LEGACY);
	}
	else {
		connection->legacy_sock = -1;
	}

	if (type == SWAL_TYPE_MODERN || type == SWAL_TYPE_BOTH) {
		if (apiKey == 0 || strlen(apiKey) <= 1) {
			fprintf(stderr, "Please specify an API Key for modern API.\n");
			return 0;
		}
		connection->modern_sock = swal_get_socket(SWAL_TYPE_MODERN);
		strncpy(connection->apiKey, apiKey, 32);
	}
	else {
		connection->modern_sock = -1;
		connection->apiKey[0] = '\0';
	}
	return connection;
}


/**************************************************
  This function closes all sockets to all
  active Steam APIs.

  params:
   -connection: A pointer to the swal_con
          to be closed.

  return: Nothing
**************************************************/
void swal_disconnect(swal_con* connection) {
	if (connection->legacy_sock >= 0) {
		close(connection->legacy_sock);
	}
	if (connection->modern_sock >= 0) {
		close(connection->modern_sock);
	}
	free(connection);
	return;
}

///////////////////////// INTERNAL FUNCTIONS //////////////////////////


/**************************************************
  This function executes a given API query.  The
  query must be created beforehand by a valid
  public function.

  params:
   -sock: Correct socket descriptor
   -filename: The file in which to store the
              data retrieved.

  return: 1 on success, 0 on failure
**************************************************/
int swal_execute_query(int sock, char* query, char* filename) {
	int success = 0;
	int bytesToWrite = strlen(query);
	int totalBytesWritten = 0;
	int bytesWritten;
	while (totalBytesWritten < bytesToWrite) {
		bytesWritten = write(sock, query+totalBytesWritten, strlen(query)+totalBytesWritten);
		if (bytesWritten < 0) {
			fprintf(stderr, "Error while writing to socket: %s\n", strerror(errno));
			exit(-1);
		}
		totalBytesWritten += bytesWritten;
	}
	success = swal_save_response(sock, filename);
	return success;
}


/**************************************************
  This function reads and strips out HTTP headers
  sent back by the Steam servers.  It then saves
  the content to a given file location.

  params:
   -sock: A socket identifer for an API
   -filename: The name of the file in which to
              save data returned.

  return: 1 on success, 0 on failure

  notes: Failure is caused by failure to save or
  any HTTP response code other than 200 OK.  This
  function assumes HTTP headers will be sent
  within first array of bytes returned from the
  read call.  Will fix this soon.
**************************************************/
int swal_save_response(int sock, char* filename) {
	char buffer[BUFLEN];
	int bytesRead;
	int totalBytesRead = 0;
	int headersParsed = 0;
	int success = 0;
	int contentLength;
	int headerLength;
	char* headerPtr = 0;
	FILE* saveFile;
	struct timeval timeout;

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	fd_set readSet;

	while( success < 1 ) {
		saveFile = NULL;
		FD_ZERO(&readSet);
		FD_SET(sock, &readSet);
		int returnValue = select(sock+1, &readSet, NULL, NULL, &timeout);

		if (returnValue == -1) {
			fprintf(stderr, "Select failed.\n");
			return -1;
		}

		else if (returnValue > 0) {
			while ((bytesRead = read(sock, buffer, BUFLEN-1))) {
				if (bytesRead < 0) {
					fprintf(stderr, "Error while reading from socket: %s\n", strerror(errno));
					exit(-1);
				}
				totalBytesRead += bytesRead;
				buffer[bytesRead] = 0; // Null terminate
				//printf("%s", buffer);
				if (!headersParsed) {
					if (strncmp(buffer, HTTP_STATUS_OK, sizeof(HTTP_STATUS_OK)-1) == 0) {
						success = 1;
					}
					else {
						fprintf(stderr, "Status code other than 200 OK sent back or no code sent at all\n");
					}
					if ((headerPtr = strcasestr(buffer, HTTP_CONTENT_LENGTH))) {
						headerPtr += sizeof(HTTP_CONTENT_LENGTH)-1;
						sscanf(headerPtr, "%d", &contentLength);
					}
					else {
						fprintf(stderr, "Content length header value not sent within first %d bytes sent back\n", bytesRead);
					}
					if ((headerPtr = strstr(buffer, "\r\n\r\n"))) {
						headerPtr += sizeof("\r\n\r\n")-1;
						if (success) { // only write out contents if successful
							saveFile = fopen(filename, "w");
							if ( saveFile != NULL) {
								fputs(headerPtr, saveFile);
							}
						}
						headerLength = headerPtr - buffer;
					}
					else {
						fprintf(stderr, "HTTP header end sentinel not reached within first %d bytes sent back\n", bytesRead);
					}
					headersParsed = 1;
				}
				else {
					if (success && (saveFile != NULL)) fputs(buffer, saveFile); // only write out contents if successful
					if (totalBytesRead == contentLength + headerLength) {
						//printf("Finished Reading\n");
						break;
					}
				}
			}
		}
	}

	if (success && (saveFile != NULL)) {
		fclose(saveFile); // only write out contents if successful
	}

	//if (success) printf("Query successful\n");
	//else printf("Query failed\n");
	//printf("Content-Length: %d\n", contentLength);
	return success;
}


/**************************************************
  This function creates a socket connection to
  a given steam API service indicated by the
  type parameter.

  params:
   -type: Indicates which API to use.  Legal
          values are SWAL_TYPE_LEGACY,
          SWAL_TYPE_MODERN, and SWAL_TYPE_BOTH

  return: Socket descriptor on success, -1 on 
          failure
**************************************************/
int swal_get_socket(int type) {
	int tcpSocket;
	int error;
	struct addrinfo* steamAddressInfo;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;		/* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;	/* Stream socket */
	hints.ai_protocol = IPPROTO_TCP;	/* TCP protocol */

	error = getaddrinfo(type == SWAL_TYPE_MODERN ? STEAM_WEB_API_HOSTNAME : STEAM_COMMUNITY_DATA_HOSTNAME, STEAM_WEB_API_SERVICE, &hints, &steamAddressInfo);
	if (error != 0) {   
		fprintf(stderr, "Error in getaddrinfo: %s\n", gai_strerror(error));
		return -1;
	}

	// Assume first address returned is the one we're going to use
	// We can change this later to iterate over other addresses returned if we run into issues
	tcpSocket = socket(steamAddressInfo->ai_family, steamAddressInfo->ai_socktype, steamAddressInfo->ai_protocol);
	if (tcpSocket < 0) {
		fprintf(stderr, "Failed to create socket.\n");
		return -1;
	}

	if (connect(tcpSocket, steamAddressInfo->ai_addr, steamAddressInfo->ai_addrlen) != 0) {
		close(tcpSocket);
		fprintf(stderr, "Failed to connect.\n");
		return -1;
	}

	freeaddrinfo(steamAddressInfo);
	return tcpSocket;
}

