#include "connectionUtil.h"
#include "gameConnection.h"



PacketTranslator::PacketTranslator()
{
	m_isHeaderNext = true;

	m_readIndex = 0;
	m_writeIndex = 0;
}

int PacketTranslator::saveBytes(char* pSrc, const int bytes)
{
	int spaceLeft = PACKET_TRANSLATOR_CAPACITY - getSavedBytes() - 1;
	int bytesSaved = bytes < spaceLeft ? bytes : spaceLeft;

	int saveAtEnd = PACKET_TRANSLATOR_CAPACITY - m_writeIndex < bytesSaved ? PACKET_TRANSLATOR_CAPACITY - m_writeIndex : bytesSaved;
	int saveAtStart = 0;
	if (saveAtEnd < bytesSaved)
	{
		saveAtStart = bytesSaved - saveAtEnd;
	}
	if (saveAtEnd > 0)
	{
		memcpy(&m_aBytecode[m_writeIndex], pSrc, saveAtEnd);
	}
	if (saveAtStart > 0)
	{
		memcpy(&m_aBytecode[0], pSrc + saveAtEnd, saveAtStart);
	}

	m_writeIndex = (m_writeIndex + bytesSaved) % PACKET_TRANSLATOR_CAPACITY;

	return bytesSaved;
}

bool PacketTranslator::readMessage(Connection* pConnection)
{
	if (m_isHeaderNext)
	{
		if (getSavedBytes() >= 16)
		{
			char tmpBuffer[16];
			int readFromEnd = PACKET_TRANSLATOR_CAPACITY - m_readIndex < 16 ? PACKET_TRANSLATOR_CAPACITY - m_readIndex : 16;
			int readFromStart = 0;
			if (readFromEnd < 16)
			{
				readFromStart = 16 - readFromEnd;
			}
			if (readFromEnd > 0)
			{
				memcpy(tmpBuffer, &m_aBytecode[m_readIndex], readFromEnd);
			}
			if (readFromStart > 0)
			{
				memcpy(tmpBuffer + readFromEnd, &m_aBytecode[0], readFromStart);
			}

			m_readIndex = (m_readIndex + 16) % PACKET_TRANSLATOR_CAPACITY;

			memcpy(&m_senderID, &tmpBuffer[0], sizeof(int));
			memcpy(&m_msgCode, &tmpBuffer[4], sizeof(int));
			memcpy(&m_msgType, &tmpBuffer[8], sizeof(int));
			memcpy(&m_msgSize, &tmpBuffer[12], sizeof(int));
			m_isHeaderNext = false;

			return true;
		}
	}
	else
	{
		if (getSavedBytes() >= m_msgSize)
		{
			char tmpBuffer[256];
			int readFromEnd = PACKET_TRANSLATOR_CAPACITY - m_readIndex < m_msgSize ? PACKET_TRANSLATOR_CAPACITY - m_readIndex : m_msgSize;
			int readFromStart = 0;
			if (readFromEnd < m_msgSize)
			{
				readFromStart = m_msgSize - readFromEnd;
			}
			if (readFromEnd > 0)
			{
				memcpy(tmpBuffer, &m_aBytecode[m_readIndex], readFromEnd);
			}
			if (readFromStart > 0)
			{
				memcpy(tmpBuffer + readFromEnd, &m_aBytecode[0], readFromStart);
			}

			m_readIndex = (m_readIndex + m_msgSize) % PACKET_TRANSLATOR_CAPACITY;
			pConnection->storePacket(m_msgCode, m_msgType, m_msgSize, tmpBuffer);

			m_isHeaderNext = true;

			return true;
		}
	}

	return false;
}

int PacketTranslator::getSavedBytes() const
{
	return ((m_writeIndex - m_readIndex + PACKET_TRANSLATOR_CAPACITY) % PACKET_TRANSLATOR_CAPACITY);
}