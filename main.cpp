#include "gameConnection.h"
#include "gameState.h"
#include "directX.h"
#include "..\..\common\performanceUtility.h"
#include <sstream>



GameState* g_pCurrentGameState = 0;



const LPSTR WND_CLASS_NAME = "ARENA";

HRESULT CALLBACK MainEventProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	float xPerc = static_cast<float>(x) / screenWidth;
	float yPerc = static_cast<float>(y) / screenHeight;

	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_ESCAPE:
					if (MessageBox(hWnd, "Are you sure you want to quit?", "QUIT", MB_YESNO) == IDYES)
					{
						PostQuitMessage(0);
					}
					break;
				default:
					if (g_pCurrentGameState) { g_pCurrentGameState->onKeyPressed(wParam); }
					break;
			}
			break;
		case WM_KEYUP:
			if (g_pCurrentGameState) { g_pCurrentGameState->onKeyReleased(wParam); }
			break;
		case WM_LBUTTONDOWN:
			if (g_pCurrentGameState) { g_pCurrentGameState->onMousePressed(0, xPerc, yPerc); }
			break;
		case WM_MBUTTONDOWN:
			if (g_pCurrentGameState) { g_pCurrentGameState->onMousePressed(1, xPerc, yPerc); }
			break;
		case WM_RBUTTONDOWN:
			if (g_pCurrentGameState) { g_pCurrentGameState->onMousePressed(2, xPerc, yPerc); }
			break;
		case WM_LBUTTONUP:
			if (g_pCurrentGameState) { g_pCurrentGameState->onMouseReleased(0, xPerc, yPerc); }
			break;
		case WM_MBUTTONUP:
			if (g_pCurrentGameState) { g_pCurrentGameState->onMouseReleased(1, xPerc, yPerc); }
			break;
		case WM_RBUTTONUP:
			if (g_pCurrentGameState) { g_pCurrentGameState->onMouseReleased(2, xPerc, yPerc); }
			break;
		case WM_MOUSEMOVE:
			if (g_pCurrentGameState) { g_pCurrentGameState->onMouseMove(xPerc, yPerc); }
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND initializeWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wnd;
	ZeroMemory(&wnd, sizeof(wnd));
	wnd.cbSize = sizeof(wnd);
	wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainEventProc;
	wnd.lpszClassName = WND_CLASS_NAME;
	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	if (!RegisterClassEx(&wnd))
	{
		return 0;
	}

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	HWND hWnd = CreateWindowEx(0,//WS_EX_TOPMOST,
		WND_CLASS_NAME,
		WND_CLASS_NAME,
		WS_POPUP,
		0, 0,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL
		);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	return hWnd;
}

int gameLoop(HWND hWnd)
{
	PerformanceUtility performanceUtility;
	PerformanceUtilityProvider::provide(&performanceUtility);

	WSAData wsaData;
	WORD dllVersion = MAKEWORD(2, 1);
	if (WSAStartup(dllVersion, &wsaData) != 0) { return false; }

	Connection mainServer;
	Connection matchServer;

	DirectX directX;
	if (!directX.initialize(hWnd))
	{
		return false;
	}
	directX.setPosition(0.0f, 0.0f);
	directX.setDistance(10.0f);

	DeathmatchState deathmatchState(matchServer, directX, &g_pCurrentGameState);
	MenuState menuState(mainServer, directX, &g_pCurrentGameState);
	g_pCurrentGameState = &menuState;
	g_pCurrentGameState->captureConnection();
	deathmatchState.initialize();
	menuState.initialize();
	menuState.captureConnection();
	menuState.captureDirectX();
	deathmatchState.setNextGameState(&menuState);
	menuState.setNextGameState(&deathmatchState);

	FILE* pAddressFile;
	fopen_s(&pAddressFile, "data\\server.txt", "rt");
	if (!pAddressFile)
	{
		return false;
	}
	char buffer[16];
	char digits[] = "1234567890.";
	fread(buffer, sizeof(char), 16, pAddressFile);
	fclose(pAddressFile);
	int digitsCount = strspn(buffer, digits);
	std::string addressName(buffer, digitsCount);

	if (!mainServer.connectToServer(addressName.c_str(), 12992))
	{
		MessageBox(hWnd, "Game server is not available", "CONNECTION ERROR", MB_OK);
		return -1;
	}
	if (!mainServer.initializeLog("logs\\server_logs\\log_"))
	{
		MessageBox(hWnd, "Cannot create server log file", "NO LOGS", MB_OK);
	}
	mainServer.sendInt(MESSAGE_IMPLAYER, 0);

	__int64 countsPerSec;
	__int64 startCounter;
	__int64 endCounter;
	__int64 startRender;
	__int64 endRender;

	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);

	double secondsPerCount = 1.0f / countsPerSec;

	int currentTick = 0;
	int ticksPerSec = 50;
	float secondsForTick = 1.0f / ticksPerSec;

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
			if (g_pCurrentGameState)
			{
				g_pCurrentGameState->update(secondsForTick);
			}
			QueryPerformanceCounter((LARGE_INTEGER*)&endCounter);
			float secondsTaken = (endCounter - startCounter) * secondsPerCount;
			float secondsForSleep = secondsForTick - secondsTaken;
			if (secondsForSleep > 0.0f)
			{
				QueryPerformanceCounter((LARGE_INTEGER*)&startRender);
				if (g_pCurrentGameState)
				{
					g_pCurrentGameState->draw();
				}
				QueryPerformanceCounter((LARGE_INTEGER*)&endRender);
				float renderSeconds = (endRender - startRender) * secondsPerCount;
				secondsForSleep -= renderSeconds;
				int milisecondsForSleep = secondsForSleep * 1000;
				if (milisecondsForSleep > 0)
				{
					Sleep(milisecondsForSleep);
				}
			}
			currentTick++;
		}
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	mainServer.sendInt(MESSAGE_PLAYER_LEFT, 0);

	return 0;
}



int main()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);

	HWND hWnd = initializeWindow(hInstance);

	return gameLoop(hWnd);
}