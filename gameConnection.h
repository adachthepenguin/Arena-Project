#pragma once

#include <WinSock2.h>
#include <string>
#include <thread>
#include "..\..\common\vector.h"
#include "connectionUtil.h"



#define PACKET_MAX_LENGTH	236

#define PACKET_INT			1
#define PACKET_FLOAT		2
#define PACKET_UNSIGNED		3
#define PACKET_TEXT			4
#define PACKET_ID			16
#define PACKET_MOVEMENT		32
#define PACKET_ROTATION		33
#define PACKET_ANIMATION	35
#define PACKET_CREATION		40
#define PACKET_DESTRUCTION	41
#define PACKET_COMMAND		50
#define PACKET_UNSIGNEDINT		101
#define PACKET_UNSIGNEDFLOAT	102
#define PACKET_UNSIGNEDUNSIGNED	103
#define PACKET_UNSIGNEDTEXT		104

#define MESSAGE_WELCOME				50
#define MESSAGE_FOLLOWED_ID			100
#define MESSAGE_PLAYER_LEFT			150
#define MESSAGE_MATCH_FINISHED		151
#define MESSAGE_HOT_MESSAGE			200
#define MESSAGE_PLAYER_NAME			300
#define MESSAGE_PLAYER_ID			301
#define MESSAGE_PING				1000
#define MESSAGE_PING_RESPONSE		1001

#define MESSAGE_HP_CURRENT			1500
#define MESSAGE_HP_MAX				1501
#define MESSAGE_HP_TILLRESPAWN		1510
#define MESSAGE_MANA_CURRENT		1520
#define MESSAGE_MANA_MAX			1521
#define MESSAGE_INFO_NAME			1550
#define MESSAGE_INFO_USERID			1551
#define MESSAGE_INFO_LEVEL			1552
#define MESSAGE_SKILLS_SPEED		1600
#define MESSAGE_SKILLS_AGILITY		1601
#define MESSAGE_SKILLS_STRENGTH		1602
#define MESSAGE_SKILLS_DAMAGE		1603
#define MESSAGE_SKILLS_REGEN		1604
#define MESSAGE_SKILLS_LEVEL		1610
#define MESSAGE_SKILLS_TILLNEXT		1611
#define MESSAGE_STATS_TAKEDOWNS		1700
#define MESSAGE_STATS_DEATHS		1701
#define MESSAGE_STATS_DMGDEALT		1702
#define MESSAGE_STATS_DMGTAKEN		1703
#define MESSAGE_STATS_POINTS		1704
#define MESSAGE_STATS_EFFICIENCY	1705
#define MESSAGE_STATS_PING			1720

#define MESSAGE_GAMETIME_PROGRESS	1550
#define MESSAGE_GAMETIME_OVERTIME	1551

#define MESSAGE_ARENA_STARTX		1800
#define MESSAGE_ARENA_STARTY		1801
#define MESSAGE_ARENA_ENDX			1802
#define MESSAGE_ARENA_ENDY			1803
#define MESSAGE_ARENA_SUBLEVELS		1804
#define MESSAGE_ARENA_CREATE		1805
#define MESSAGE_ARENA_MOVE			1810
#define MESSAGE_ARENA_SETTAKEN		1820

#define MESSAGE_LOGIN_USERNAME		3000
#define MESSAGE_LOGIN_PASSWORD		3001
#define MESSAGE_LOGIN_CONFIRM		3002
#define MESSAGE_LOGIN_ACCEPTED		3010
#define MESSAGE_LOGIN_REJECTED		3011

#define MESSAGE_CHARACTERSELECT		4000

#define MESSAGE_IMPLAYER			20000

#define MESSAGE_PORT				21000

#define MESSAGE_MATCHMAKING_ENABLE	22000

#define MESSAGE_PREVIEW_SPEEDB		60000
#define MESSAGE_PREVIEW_AGILITYB	60001
#define MESSAGE_PREVIEW_STRENGTHB	60002
#define MESSAGE_PREVIEW_DAMAGEB		60003
#define MESSAGE_PREVIEW_HEALTHB		60004
#define MESSAGE_PREVIEW_REGENB		60005
#define MESSAGE_PREVIEW_SPEEDP		60010
#define MESSAGE_PREVIEW_AGILITYP	60011
#define MESSAGE_PREVIEW_STRENGTHP	60012
#define MESSAGE_PREVIEW_DAMAGEP		60013
#define MESSAGE_PREVIEW_HEALTHP		60014
#define MESSAGE_PREVIEW_REGENP		60015



class Connection;



class PacketQueue
{
private:
	int m_start;
	int m_capacity;
	int m_lastMessage;

	int* m_aCode;
	int* m_aDataType;
	int* m_aDataSize;
	char** m_aData;

public:
	PacketQueue(const int capacity);
	~PacketQueue();

	bool isMessageAvailable() const;
	void handleTopMessage(Connection* pConnection);
	bool insertMessage(const int code, const int dataType, const int dataSize, char* buffer);
};



class Connection
{
	friend class GameState;

private:
	HANDLE m_threadHandle;

	int m_userID;

	SOCKET m_connectionSocket;

	GameState* m_pCurrentGameState;

	char m_buffer[256];
	int m_dataSize;

	PacketQueue m_messageQueue;
	PacketTranslator m_packetTranslator;

	// LOG FILE
	FILE* m_pLogFile;

	// MUTEXES
	std::thread::id m_threadID;

public:
	Connection();
	~Connection();

	SOCKET getSocket() const;

	bool initializeLog(const char* filebase);
	bool connectToServer(const char* address, const int port);
	void disconnectFromServer();

	void sendInt(const int code, const int data);
	void sendFloat(const int code, const float data);
	void sendUnsigned(const int code, const unsigned data);
	void sendText(const int code, const std::string data);
	void sendUnsignedInt(const int code, const unsigned id, const int data);
	void sendUnsignedFloat(const int code, const unsigned id, const float data);
	void sendUnsignedUnsigned(const int code, const unsigned id, const unsigned data);
	void sendUnsignedText(const int code, const unsigned id, const std::string data);
	void sendCommand(const int code, const float arg1, const float arg2, const float arg3, const float arg4);

	void log(const char* message);
	void logSenderError(const int errorCode, const int code, const int dataType, const int size, const int targetSize);
	void logReceiverError(const int errorCode, const int code, const int dataType, const int size, const int targetSize, const bool isHeader);

	void saveBytes(char* pSrc, const int bytes);
	void translateMessages();

	void storePacket(const int code, const int dataType, const int dataSize, char* data);
	void receivePacket(const int code, const int dataType, const int dataSize, char* data);
	void handleDelayedMessages();

private:
	void setHeader(const int code, const int dataType, const int dataSize);
	void confirmSending();
	void captureMutex();
	void releaseMutex();
};