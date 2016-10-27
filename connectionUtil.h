#pragma once



#include <WinSock2.h>



#define PACKET_TRANSLATOR_CAPACITY		4096



class Connection;



class PacketTranslator
{
private:
	bool m_isHeaderNext;

	int m_senderID;
	int m_msgCode;
	int m_msgType;
	int m_msgSize;

	char m_aBytecode[PACKET_TRANSLATOR_CAPACITY];
	int m_writeIndex;
	int m_readIndex;

public:
	PacketTranslator();

	int saveBytes(char* pSrc, const int bytes);
	bool readMessage(Connection* pConnection);

private:
	int getSavedBytes() const;
};