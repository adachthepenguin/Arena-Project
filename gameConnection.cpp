#include "gameConnection.h"
#include "gameState.h"
#include <iostream>
#include <ctime>



int waitForPacketsThread(Connection* pConnection)
{
	while (1)
	{
		char readBuffer[256];
		int bytesReceived = recv(pConnection->getSocket(), readBuffer, 256, 0);
		pConnection->saveBytes(readBuffer, bytesReceived);
		pConnection->translateMessages();
	}

	return 0;
}



PacketQueue::PacketQueue(const int capacity)
{
	m_start = 0;
	m_lastMessage = 0;
	m_capacity = capacity;

	m_aCode = new int[capacity];
	m_aDataType = new int[capacity];
	m_aDataSize = new int[capacity];
	m_aData = new char*[capacity];
	for (int i = 0; i < capacity; i++)
	{
		m_aData[i] = new char[PACKET_MAX_LENGTH];
	}
}

PacketQueue::~PacketQueue()
{
	for (int i = 0; i < m_capacity; i++)
	{
		delete[] m_aData[i];
	}
	delete[] m_aCode;
	delete[] m_aDataType;
	delete[] m_aDataSize;
	delete[] m_aData;
}

bool PacketQueue::isMessageAvailable() const
{
	if (m_start == m_lastMessage) { return false; }

	return true;
}

void PacketQueue::handleTopMessage(Connection* pConnection)
{
	pConnection->receivePacket(m_aCode[m_start], m_aDataType[m_start], m_aDataSize[m_start], m_aData[m_start]);
	m_start = (m_start + 1) % m_capacity;
}

bool PacketQueue::insertMessage(const int code, const int dataType, const int dataSize, char* buffer)
{
	if ((m_lastMessage + 1) % m_capacity == m_start)
	{
		return false;
	}

	m_aCode[m_lastMessage] = code;
	m_aDataType[m_lastMessage] = dataType;
	m_aDataSize[m_lastMessage] = dataSize;
	memcpy(m_aData[m_lastMessage], buffer, PACKET_MAX_LENGTH);

	m_lastMessage = (m_lastMessage + 1) % m_capacity;

	return true;
}



Connection::Connection()
: m_messageQueue(2500)
{
	m_pCurrentGameState = 0;

	m_pLogFile = 0;
}

Connection::~Connection()
{
	disconnectFromServer();

	TerminateThread(m_threadHandle, 0);

	if (m_pLogFile)
	{
		fclose(m_pLogFile);
	}
}

SOCKET Connection::getSocket() const
{
	return m_connectionSocket;
}

bool Connection::initializeLog(const char* filebase)
{
	time_t t;
	time(&t);
	tm date;
	localtime_s(&date, &t);
	std::string filename = filebase;
	filename += std::to_string(date.tm_year + 1900);
	filename += "_";
	filename += std::to_string(date.tm_mon + 1);
	filename += "_";
	filename += std::to_string(date.tm_mday);
	filename += "_";
	filename += std::to_string(date.tm_hour);
	filename += "_";
	filename += std::to_string(date.tm_min);
	filename += "_";
	filename += std::to_string(date.tm_sec);
	filename += ".txt";

	fopen_s(&m_pLogFile, filename.c_str(), "wt");
	if (!m_pLogFile) { return false; }

	return true;
}

bool Connection::connectToServer(const char* address, const int port)
{
	SOCKADDR_IN addr;
	int addrlen = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr(address);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	m_connectionSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(m_connectionSocket, (SOCKADDR*)&addr, addrlen) != 0)
	{
		int lastError = WSAGetLastError();
		return false;
	}

	m_threadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)waitForPacketsThread, this, 0, 0);

	return true;
}

void Connection::disconnectFromServer()
{
	sendInt(MESSAGE_PLAYER_LEFT, 0);
	//shutdown(m_connectionSocket, SD_BOTH);
	//closesocket(m_connectionSocket);
}

void Connection::sendInt(const int code, const int data)
{
	captureMutex();
	setHeader(code, PACKET_INT, sizeof(int));

	memcpy(&m_buffer[16], &data, sizeof(int));

	confirmSending();
	releaseMutex();
}

void Connection::sendFloat(const int code, const float data)
{
	captureMutex();
	setHeader(code, PACKET_FLOAT, sizeof(float));

	memcpy(&m_buffer[16], &data, sizeof(float));

	confirmSending();
	releaseMutex();
}

void Connection::sendUnsigned(const int code, const unsigned data)
{
	captureMutex();
	setHeader(code, PACKET_UNSIGNED, sizeof(unsigned));

	memcpy(&m_buffer[16], &data, sizeof(unsigned));

	confirmSending();
	releaseMutex();
}

void Connection::sendText(const int code, const std::string data)
{
	captureMutex();
	setHeader(code, PACKET_TEXT, data.length());

	for (int i = 0; i < data.length(); i++)
	{
		memcpy(&m_buffer[16 + i], &data[i], sizeof(char));
	}

	confirmSending();
	releaseMutex();
}

void Connection::sendUnsignedInt(const int code, const unsigned id, const int data)
{
	captureMutex();
	setHeader(code, PACKET_UNSIGNEDINT, sizeof(unsigned)+sizeof(int));

	memcpy(&m_buffer[16], &id, sizeof(unsigned));
	memcpy(&m_buffer[20], &data, sizeof(int));

	confirmSending();
	releaseMutex();
}

void Connection::sendUnsignedFloat(const int code, const unsigned id, const float data)
{
	captureMutex();
	setHeader(code, PACKET_UNSIGNEDFLOAT, sizeof(unsigned)+sizeof(float));

	memcpy(&m_buffer[16], &id, sizeof(unsigned));
	memcpy(&m_buffer[20], &data, sizeof(float));

	confirmSending();
	releaseMutex();
}

void Connection::sendUnsignedUnsigned(const int code, const unsigned id, const unsigned data)
{
	captureMutex();
	setHeader(code, PACKET_UNSIGNEDUNSIGNED, sizeof(unsigned)+sizeof(unsigned));

	memcpy(&m_buffer[16], &id, sizeof(unsigned));
	memcpy(&m_buffer[20], &data, sizeof(unsigned));

	confirmSending();
	releaseMutex();
}

void Connection::sendUnsignedText(const int code, const unsigned id, const std::string data)
{
	captureMutex();
	setHeader(code, PACKET_UNSIGNEDTEXT, sizeof(unsigned)+data.length());

	memcpy(&m_buffer[16], &id, sizeof(unsigned));
	for (int i = 0; i < data.length(); i++)
	{
		memcpy(&m_buffer[20 + i], &data[i], sizeof(char));
	}

	confirmSending();
	releaseMutex();
}

void Connection::sendCommand(const int code, const float arg1, const float arg2, const float arg3, const float arg4)
{
	captureMutex();
	setHeader(code, PACKET_COMMAND, 4 * sizeof(float));

	memcpy(&m_buffer[16], &arg1, sizeof(float));
	memcpy(&m_buffer[20], &arg2, sizeof(float));
	memcpy(&m_buffer[24], &arg3, sizeof(float));
	memcpy(&m_buffer[28], &arg4, sizeof(float));

	confirmSending();
	releaseMutex();
}

void Connection::log(const char* message)
{
	std::cout << message << std::endl;

	if (m_pLogFile)
	{
		fprintf(m_pLogFile, message);
		fprintf(m_pLogFile, "\n");
	}
}

void Connection::logSenderError(const int errorCode, const int code, const int dataType, const int size, const int targetSize)
{
	std::string logMessage;
	logMessage += "Sender error.";
	logMessage += "\nError code: ";
	logMessage += std::to_string(errorCode);
	logMessage += "\nCode: ";
	logMessage += std::to_string(code);
	logMessage += "\nType: ";
	logMessage += std::to_string(dataType);
	logMessage += "\nSize: ";
	logMessage += std::to_string(size);
	logMessage += "\nTarget size: ";
	logMessage += std::to_string(targetSize);

	log(logMessage.c_str());
}

void Connection::logReceiverError(const int errorCode, const int code, const int dataType, const int size, const int targetSize, const bool isHeader)
{
	std::string logMessage;
	logMessage += "Receiver error (";
	if (isHeader) { logMessage += "header"; }
	else { logMessage += "data"; }
	logMessage += ").";
	logMessage += "\nError code: ";
	logMessage += std::to_string(errorCode);
	logMessage += "\nCode: ";
	logMessage += std::to_string(code);
	logMessage += "\nType: ";
	logMessage += std::to_string(dataType);
	logMessage += "\nSize: ";
	logMessage += std::to_string(size);
	logMessage += "\nTarget size: ";
	logMessage += std::to_string(targetSize);

	log(logMessage.c_str());
}

void Connection::saveBytes(char* pSrc, const int bytes)
{
	m_packetTranslator.saveBytes(pSrc, bytes);
}

void Connection::translateMessages()
{
	while (m_packetTranslator.readMessage(this));
}

void Connection::storePacket(const int code, const int dataType, const int dataSize, char* data)
{
	m_messageQueue.insertMessage(code, dataType, dataSize, data);
}

void Connection::receivePacket(const int code, const int dataType, const int dataSize, char* data)
{
	//std::string logMessage = "Message read successfully!";
	//logMessage += "\nCode: ";
	//logMessage += std::to_string(code);
	//logMessage += "\nType: ";
	//logMessage += std::to_string(dataType);
	//logMessage += "\nSize: ";
	//logMessage += std::to_string(dataSize);
	//logMessage += "\n";
	//log(logMessage.c_str());

	switch (dataType)
	{
		case PACKET_INT:
		{
			int translatedData;
			memcpy(&translatedData, data, sizeof(int));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onInt(code, translatedData);
			}
			break;
		}
		case PACKET_FLOAT:
		{
			float translatedData;
			memcpy(&translatedData, data, sizeof(float));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onFloat(code, translatedData);
			}
			break;
		}
		case PACKET_UNSIGNED:
		{
			unsigned translatedData;
			memcpy(&translatedData, data, sizeof(unsigned));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onUnsigned(code, translatedData);
			}
			break;
		}
		case PACKET_TEXT:
		{
			std::string translatedData(data, dataSize);
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onText(code, translatedData);
			}
			break;
		}
		case PACKET_UNSIGNEDINT:
		{
			unsigned id;
			int translatedData;
			memcpy(&id, &data[0], sizeof(unsigned));
			memcpy(&translatedData, &data[4], sizeof(int));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onUnsignedInt(code, id, translatedData);
			}
			break;
		}
		case PACKET_UNSIGNEDFLOAT:
		{
			unsigned id;
			float translatedData;
			memcpy(&id, &data[0], sizeof(unsigned));
			memcpy(&translatedData, &data[4], sizeof(float));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onUnsignedFloat(code, id, translatedData);
			}
			break;
		}
		case PACKET_UNSIGNEDUNSIGNED:
		{
			unsigned id;
			unsigned translatedData;
			memcpy(&id, &data[0], sizeof(unsigned));
			memcpy(&translatedData, &data[4], sizeof(unsigned));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onUnsignedUnsigned(code, id, translatedData);
			}
		}
		case PACKET_UNSIGNEDTEXT:
		{
			unsigned id;
			memcpy(&id, &data[0], sizeof(unsigned));
			std::string translatedData(&data[4], dataSize - 4);
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onUnsignedText(code, id, translatedData);
			}
		}
		case PACKET_ID:
			memcpy(&m_userID, data, sizeof(int));
			break;
		case PACKET_MOVEMENT:
		{
			unsigned movedObjectID;
			Vector startPosition;
			Vector positionPerSec;
			float movementDuration;
			memcpy(&movedObjectID, &data[0], sizeof(unsigned));
			memcpy(&startPosition, &data[4], sizeof(Vector));
			memcpy(&positionPerSec, &data[12], sizeof(Vector));
			memcpy(&movementDuration, &data[20], sizeof(float));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onMovement(code, movedObjectID, startPosition, positionPerSec, movementDuration);
			}
			break;
		}
		case PACKET_ROTATION:
		{
			unsigned movedObjectID;
			float startDirection;
			float directionPerSec;
			float rotationDuration;
			memcpy(&movedObjectID, &data[0], sizeof(unsigned));
			memcpy(&startDirection, &data[4], sizeof(float));
			memcpy(&directionPerSec, &data[8], sizeof(float));
			memcpy(&rotationDuration, &data[12], sizeof(float));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onRotation(code, movedObjectID, startDirection, directionPerSec, rotationDuration);
			}
			break;
		}
		case PACKET_ANIMATION:
		{
			unsigned animatedObjectID;
			int animationID;
			memcpy(&animatedObjectID, &data[0], sizeof(unsigned));
			memcpy(&animationID, &data[4], sizeof(int));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onAnimation(code, animatedObjectID, animationID);
			}
			break;
		}
		case PACKET_CREATION:
		{
			unsigned createdObjectID;
			int createdObjectType;
			memcpy(&createdObjectID, &data[0], sizeof(unsigned));
			memcpy(&createdObjectType, &data[4], sizeof(int));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onCreation(code, createdObjectID, createdObjectType);
			}
			break;
		}
		case PACKET_DESTRUCTION:
		{
			unsigned destructedObjectID;
			memcpy(&destructedObjectID, &data[0], sizeof(unsigned));
			if (m_pCurrentGameState)
			{
				m_pCurrentGameState->onDestruction(code, destructedObjectID);
			}
			break;
		}
	}
}

void Connection::handleDelayedMessages()
{
	while (m_messageQueue.isMessageAvailable())
	{
		m_messageQueue.handleTopMessage(this);
	}
}

void Connection::setHeader(const int code, const int dataType, const int dataSize)
{
	int dataSent = dataSize;
	if (dataSent > PACKET_MAX_LENGTH)
	{
		dataSent = PACKET_MAX_LENGTH;
	}

	memcpy(&m_buffer[0], &m_userID, sizeof(int));
	memcpy(&m_buffer[4], &code, sizeof(int));
	memcpy(&m_buffer[8], &dataType, sizeof(int));
	memcpy(&m_buffer[12], &dataSent, sizeof(int));

	m_dataSize = dataSent;
}

void Connection::confirmSending()
{
	int bytesSent = send(m_connectionSocket, m_buffer, 16 + m_dataSize, 0);
	if (bytesSent != 16 + m_dataSize)
	{
		int code, dataType;
		memcpy(&code, &m_buffer[4], sizeof(int));
		memcpy(&dataType, &m_buffer[8], sizeof(int));
		int errorID = WSAGetLastError();
		logSenderError(errorID, code, dataType, m_dataSize, bytesSent);
	}
}

void Connection::captureMutex()
{
	std::thread::id threadID = std::this_thread::get_id();
	std::thread::id defaultThreadID = std::thread::id();

	if (m_threadID == defaultThreadID)
	{
		m_threadID = threadID;
	}
	else
	{
		log("At least two threads are trying to access the same part of code");
	}
}

void Connection::releaseMutex()
{
	std::thread::id defaultThreadID = std::thread::id();

	m_threadID = defaultThreadID;
}