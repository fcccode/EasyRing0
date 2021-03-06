#include <Windows.h>
#include <iostream>
#include <filesystem>

void ShowErrorMessage(const std::string & szError, DWORD dwErrorCode)
{
	char szErrorMessage[4096] = { 0 };

	auto dwFlags = DWORD(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK);
	FormatMessageA(dwFlags, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorMessage, _countof(szErrorMessage), NULL);

	printf("%s | Error: %u - Description: %s\n", szError.c_str(), dwErrorCode, szErrorMessage);
}

void PrintDetailedLog(SC_HANDLE shServiceHandle)
{
	DWORD bytesNeeded;
	SERVICE_STATUS_PROCESS ssStatus;
	if (QueryServiceStatusEx(shServiceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded) == FALSE)
	{
		ShowErrorMessage("QueryServiceStatusEx fail!", GetLastError());
		return;
	}

	printf("Service detailed logs handled. \n");
	printf("  Current State: %d\n", ssStatus.dwCurrentState);
	printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
	printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
	printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
}

bool GetServiceStatus(const std::string & szServiceName)
{
	auto dwResult	= 0UL;
	auto bRet		= false;
	auto hSCManager = SC_HANDLE(nullptr);
	auto hService	= SC_HANDLE(nullptr);
	auto sStatus	= SERVICE_STATUS { 0 };

	hSCManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		ShowErrorMessage("OpenSCManagerA fail!", GetLastError());
		goto _Complete;
	}
	hService = OpenServiceA(hSCManager, szServiceName.c_str(), SERVICE_QUERY_STATUS);
	if (hService == NULL)
	{
		ShowErrorMessage("OpenServiceA fail!", GetLastError());
		goto _Complete;
	}
	if (!QueryServiceStatus(hService, &sStatus))
	{
		ShowErrorMessage("QueryServiceStatus fail!", GetLastError());
		goto _Complete;
	}

	dwResult = sStatus.dwCurrentState;
	printf("Service: %s Current Status: %u\n", szServiceName.c_str(), dwResult);

_Complete:
	if (dwResult == 0 && hService)
		PrintDetailedLog(hService);

	if (hSCManager)
	{
		CloseServiceHandle(hSCManager);
		hSCManager = nullptr;
	}
	if (hService)
	{
		CloseServiceHandle(hService);
		hService = nullptr;
	}

	return dwResult;
}

bool LoadService(const std::string & szServiceName, const std::string & szServiceDisplayName, const std::string & szServiceFile)
{
	auto bRet		= false;
	auto hSCManager = SC_HANDLE(nullptr);
	auto hService	= SC_HANDLE(nullptr);
 
	hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager)
	{
		ShowErrorMessage("OpenSCManager fail!", GetLastError());
		goto _Complete;
	}
 
	hService = CreateServiceA(hSCManager, szServiceName.c_str(), szServiceDisplayName.c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, szServiceFile.c_str(), NULL, NULL, NULL, NULL, NULL);
	if (!hService)
	{
		ShowErrorMessage("CreateServiceA fail!", GetLastError());
		goto _Complete;
	}

	bRet = true;

_Complete:
	if (bRet == false && hService)
		PrintDetailedLog(hService);

	if (hSCManager)
	{
		CloseServiceHandle(hSCManager);
		hSCManager = nullptr;
	}
	if (hService)
	{
		CloseServiceHandle(hService);
		hService = nullptr;
	}

	return bRet;
}

bool UnloadService(const std::string & szServiceName)
{
	auto bRet		= false;
	auto hSCManager = SC_HANDLE(nullptr);
	auto hService	= SC_HANDLE(nullptr);

	hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		ShowErrorMessage("OpenSCManager fail!", GetLastError());
		goto _Complete;
	}

	hService = OpenServiceA(hSCManager, szServiceName.c_str(), DELETE | SERVICE_QUERY_STATUS);
	if (!hService)
	{
		ShowErrorMessage("OpenServiceA fail!", GetLastError());
		goto _Complete;
	}
 
	if (DeleteService(hService) == FALSE)
	{
		ShowErrorMessage("DeleteService fail!", GetLastError());
		goto _Complete;
	}
 
	bRet = true;

_Complete:
	if (bRet == false && hService)
		PrintDetailedLog(hService);

	if (hSCManager)
	{
		CloseServiceHandle(hSCManager);
		hSCManager = nullptr;
	}
	if (hService)
	{
		CloseServiceHandle(hService);
		hService = nullptr;
	}

	return bRet;
}

bool StartService(const std::string & szServiceName)
{
	auto bRet		= false;
	auto hSCManager = SC_HANDLE(nullptr);
	auto hService	= SC_HANDLE(nullptr);
  
	hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		ShowErrorMessage("OpenSCManager fail!", GetLastError());
		goto _Complete;
	}

	hService = OpenServiceA(hSCManager, szServiceName.c_str(), SERVICE_START | SERVICE_QUERY_STATUS);
	if (!hService)
	{
		ShowErrorMessage("OpenServiceA fail!", GetLastError());
		goto _Complete;
	}
 
	if (StartServiceA(hService, 0, NULL) == FALSE)
	{
		ShowErrorMessage("StartServiceA fail!", GetLastError());
		goto _Complete;
	}
 
	bRet = true;

_Complete:
	if (bRet == false && hService)
		PrintDetailedLog(hService);

	if (hSCManager)
	{
		CloseServiceHandle(hSCManager);
		hSCManager = nullptr;
	}
	if (hService)
	{
		CloseServiceHandle(hService);
		hService = nullptr;
	}

	return bRet;
}

bool StopService(const std::string & szServiceName)
{
	auto bRet		= false;
	auto hSCManager = SC_HANDLE(nullptr);
	auto hService	= SC_HANDLE(nullptr);
	auto sStatus	= SERVICE_STATUS { 0 };

	hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		ShowErrorMessage("OpenSCManager fail!", GetLastError());
		goto _Complete;
	}

	hService = OpenServiceA(hSCManager, szServiceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
	if (!hService)
	{
		ShowErrorMessage("OpenServiceA fail!", GetLastError());
		goto _Complete;
	}

	if (ControlService(hService, SERVICE_CONTROL_STOP, &sStatus))
	{
		printf("Stopping %s ...\n", szServiceName.c_str());
		Sleep(1000);

		while (QueryServiceStatus(hService, &sStatus))
		{
			if (sStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				printf("Stopping pending %s ...\n", szServiceName.c_str());
				Sleep(1000);
			}

			else break;
		}

		if (sStatus.dwCurrentState == SERVICE_STOPPED)
		{
			printf("%s Has Successfully Stopped\n", szServiceName.c_str());
		}
		else
		{
			ShowErrorMessage("Service could not be stopped.", GetLastError());
			goto _Complete;
		}
	}
	
	bRet = true;

_Complete:
	if (bRet == false && hService)
		PrintDetailedLog(hService);

	if (hSCManager)
	{
		CloseServiceHandle(hSCManager);
		hSCManager = nullptr;
	}
	if (hService)
	{
		CloseServiceHandle(hService);
		hService = nullptr;
	}

	return bRet;
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <Service_File> <Service_Name>[OPTIONAL]\n", argv[0]);
		return 0;
	}

	auto szServiceFile = std::string(argv[1]);
	if (std::experimental::filesystem::exists(szServiceFile) == false)
	{
		printf("Target file: %s is not exist!\n", szServiceFile.c_str());
		return 0;
	}
	printf("Target file: %s\n", szServiceFile.c_str());

	auto szServiceName = std::string("");
	if (argc == 3) 
	{
		szServiceName = std::string(argv[2]);
	}
	else
	{
		auto uiDotPos		= szServiceFile.find_last_of(".");
		auto szExtsSplitted = szServiceFile.substr(0, uiDotPos);

		auto uiPathPos		= szExtsSplitted.find_last_of("\\/");
		if (uiPathPos)
			szExtsSplitted = szExtsSplitted.substr(uiPathPos + 1, szExtsSplitted.length() - uiPathPos);

		szServiceName = szExtsSplitted;
	}
	printf("Service name: %s\n", szServiceName.c_str());

	printf("--------------------------------------\n");

	char pInput = '0';
	while (pInput != 'x')
	{
		printf("Please select:\n1 --> Load service\n2 --> Unload service\n3 --> Start service\n4 --> Stop service\n5 --> Query service status\nx --> Exit\n");
		std::cin >> pInput;

		switch (pInput)
		{
			case '1':
			{
				if (::LoadService(szServiceName, szServiceName, szServiceFile) == false)
					printf("An error occured on load routine!\n");
				else
					printf("Load routine succesfully completed!\n");
			} break;

			case '2':
			{
				if (::UnloadService(szServiceName) == false)
					printf("An error occured on unload routine!\n");
				else
					printf("Unload routine succesfully completed!\n");
			} break;

			case '3':
			{
				if (::StartService(szServiceName) == false)
					printf("An error occured on start routine!\n");
				else
					printf("Start routine succesfully completed!\n");
			} break;

			case '4':
			{
				if (::StopService(szServiceName) == false)
					printf("An error occured on stop routine!\n");
				else
					printf("Stop routine succesfully completed!\n");
			} break;

			case '5':
			{
				if (::GetServiceStatus(szServiceName) == false)
					printf("An error occured on query routine!\n");
				else
					printf("Query routine succesfully completed!\n");
			} break;

			case 'x':
				return 0;

			default:
				continue;
		}
	}

    return 0;
}

