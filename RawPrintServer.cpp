/*
PrintServer Copyright (c) 2006, Henk Jonas
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (
INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (
INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//--
// This version is modified code to allow compilation in Dev-C++ and includes required WinSpool.h and WinSpool.lib
// Tested sucessfully with TDM-GCC 9.2.0 32bit Release - option.
// You will also need to modified MakefileCustom.win with you own path to g++ and other locations.

*/

//-------------------------
// INCLUDE
//-------------------------

#include <windows.h>
#include <stdio.h>
//#include <winspool.h>
#include "inc\WinSpool.h" //!important

#include "getCurrentTime.cpp"

//-------------------------
// DEFINE
//-------------------------

#define SLEEP_TIME 5000
//#define LOGFILE "C:\\PrintServer.log"
#define LOGFILE "RawPrintServer"
#define NUM_COMMANDS (sizeof commands / sizeof *commands)


//-------------------------
// PREDECLARE FUNCTIONS
//-------------------------

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

int InnerLoop(DWORD port, int service);
void ServiceMain(int argc, char **argv);
void ControlHandler(DWORD request);
int InitService();

//-------------------------
// DECLARE
//-------------------------

typedef enum
{
	INSTALL = 0,
	REMOVE,
	STANDALONE,
	BACKGROUND,
	PRIVATE_SERVICE,
	INVALID
} CommandType;

/* Same order as CommandType */
struct command
{
	const char *commandName;
	int maxCommandArgs;
} commands[] = {
	{"INSTALL", 2},
	{"REMOVE", 1},
	{"STANDALONE", 2},
	{"BACKGROUND", 2},
	{"PRIVATE_SERVICE", 1}
};

const char* serviceKey 		= "System\\CurrentControlSet\\Services\\%s";
const char* standaloneKey 	= "SOFTWARE\\Alexander_Pruss\\RawPrintServer\\%s";
const char *regKey 			= serviceKey;

char printerName[256] 		= {0};
char strServiceName[64];

DWORD serverPort;
DWORD startPort;

bool showLog 				= true;

char fname[33];

//-------------------------
// CODE START
//-------------------------

/**
 * Writes data to log and optionally show them on screen
 * may log time also
 * @param str string to be saved to file
 * @param showTime if true then saves time
 * @return int -1 for error and 0 if ok
 */
int WriteToLog(const char *str, bool showTime)
{
	//Declare
	char* timL = getCurrentTime(); //datetime string
	FILE *log; //loghandler
	
	//open file
	log = fopen(fname, "a+");
	
	//on error return
	if (log == NULL)
	{	
	    return -1;
	}
	
	//if we want datetime
	if(showTime)
	{
		fprintf(log, "%s %s\n", timL, str);
	}
	//otherwise
	else
	{
		fprintf(log, "%s\n", str);
	}
	//close file
	fclose(log);
	
	
	if(showLog & showTime)
	{
		fprintf(stderr,"%s %s\n", timL, str);
	}
	else if(showLog)
	{
		fprintf(stderr,"%s\n", str);
	}
	return 0;
}

/**
 * Creates Print Server
 * @param strMyPath
 * @param strPrinter
 * @param port
 * @param service
 * @returns nothing
 */
VOID CreatePrintServer(char *strMyPath, char *strPrinter, DWORD port, int service)
{
	//Declare
	char strTemp[1024];
	HKEY hdlKey;
	
	sprintf(strTemp, "\"%s\" PRIVATE_SERVICE %d", strMyPath, port);

	if (service)
	{	
		//Declare	
		SC_HANDLE schSCManager;
		SC_HANDLE schService;
	
		schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		
    	if (!schSCManager)
		{
      		printf("Error: ServiceManager %d\n", GetLastError());
      		return;
    	}

    	schService = CreateService(
	        schSCManager,   			// SCManager database
	        strServiceName, 			// name of service
	        strServiceName, 			// service name to display (Attention: we better use the
	                        			// same name here, or you will never find it...)
	        SERVICE_ALL_ACCESS,        	// desired access
	        SERVICE_WIN32_OWN_PROCESS, 	// service type
	        SERVICE_AUTO_START, 		// SERVICE_DEMAND_START,      // start type
	        SERVICE_ERROR_NORMAL, 		// error control type
	        strTemp, 					// lpszBinaryPathName,        // service's binary
	        NULL,  						// no load ordering group
	        NULL,  						// no tag identifier
	        NULL,  						// no dependencies
	        NULL,  						// LocalSystem account
	        NULL						// no password
		); 

	    if (schService == NULL)
		{
	    	printf("Error: CreateService %d\n", GetLastError());
	    	return;
	    }
		else
		{
	    	printf("CreateService SUCCESS.\n");
		}
	
	    if (StartService(schService, 0, NULL))
	    {
	    	printf("Service started.\n");
		}
	    else
	    {
	    	printf("Error starting Service: %d, please do it by hand.\n", GetLastError());
		}

	    CloseServiceHandle(schService);
  	}

  	sprintf(strTemp, regKey, strServiceName);
  	
  	if (!service)
	{
    	RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			(char*)"SOFTWARE\\Alexander_Pruss", 
			0,
			(char*)"", 
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			NULL,
			NULL
		);
  	}
  	
	RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		strTemp,
		0,
		(char*)"", 
	  	REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hdlKey,
		NULL
	);
  	RegSetValueEx(
	  	hdlKey, 
	  	"Description", 
	  	0, 
	  	REG_SZ, 
      	(const unsigned char *)"Routes all traffic from port 910x to a local printer",
      	53
	);
	
  	RegSetValueEx(
	  	hdlKey,
	 	"Printer",
	  	0,
	  	REG_SZ,
	  	(const unsigned char *)strPrinter,
      	strlen(strPrinter) + 1
	);
  	
	RegSetValueEx(
		hdlKey,
		"Port",
		0,
		REG_DWORD,
		(const unsigned char *)&port,
        sizeof(port)
	);

  	RegCloseKey(hdlKey);
}

/**
 *
 */
VOID DeletePrintServerService(DWORD port)
{
	//Declare
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ss;
	
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
	if (!schSCManager)
	{
		printf("Error: ServiceManager %d\n", GetLastError());
		return;
  	}

  	schService = OpenService(schSCManager,			// SCManager database
                 			 strServiceName,		// name of service
                  			 SERVICE_ALL_ACCESS);	// only need DELETE access

	if (schService == NULL)
	{
		printf("Error: OpenService (%s) %d\n", strServiceName, GetLastError());
		return;
	}

	if (ControlService(schService, SERVICE_CONTROL_STOP, &ss))
	{
		printf("Server stopped.\n");
	}
	else
	{
		//Declare
		int err;
		
		err = GetLastError();
		if (err != ERROR_SERVICE_NOT_ACTIVE)
		{
			printf("Error stopping Service: %d, please do it by hand.\n", err);
			return;
	    }
	}

	if (!DeleteService(schService))
	{
	    printf("Error: DeleteService %d\n", GetLastError());
	    return;
	}
	else
	{
	  	printf("DeleteService SUCCESS\n");
	}
	
	CloseServiceHandle(schService);
}

/**
 * Main Function
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv)
{
	//Declare
	int command;

	sprintf(fname, "RawPrintServer_%s.txt", getShortTime());
	//remove(LOGFILE);

	//  WriteToLog("RawPrintServer 1.00 created by Henk Jonas (www.metaviewsoft.de)");
	WriteToLog(" _____                                                                                  _____ ", false);
	WriteToLog("( ___ )--------------------------------------------------------------------------------( ___ )", false);
	WriteToLog(" |   |      RawPrintServer 1.00 created by Henk Jonas (www.metaviewsoft.de)             |   | ", false);
	WriteToLog(" |   |                                                                                  |   | ", false);
	WriteToLog(" |___|      This is 32bit app! Version compiled in Dev-C++                              |___| ", false);
	WriteToLog("(_____)--------------------------------------------------------------------------------(_____)\n", false);
	WriteToLog("PrintServer start", true);

	command = INVALID;

	if (1 < argc) {
	    for (command = 0; command < NUM_COMMANDS; command++)
		{
	    	if (0 == stricmp(argv[1], commands[command].commandName))
	    	{
	        	break;
			}
		}
	
	    if (NUM_COMMANDS <= command)
		{
			command = INVALID;	
		}
	      
	}
	else
	{
	  	WriteToLog("[!] No parameters detected!",false);
	}
	
	if (command != INVALID && argc == 2 + commands[command].maxCommandArgs - 1)
	{
	    serverPort = 9100;
	}
	else if (command != INVALID && argc == 2 + commands[command].maxCommandArgs)
	{
		int p = atoi(argv[2 + commands[command].maxCommandArgs - 1]);
	
		if (0 < p)
	    	serverPort = p;
	    else
	    	serverPort = 9100;
	}
	else
	{
	    char *lastPart = argv[0];
	    char *p = argv[0];
	
	    while (*p)
		{
	    	if (*p == ':' || *p == '/' || *p == '\\')
			{
	        	lastPart = p + 1;
	    	}
	      	p++;
	    }
	
	    fprintf(stderr, "%s INSTALL \"Printer Name\" [port]\n"
	                    "%s REMOVE [port]\n"
	                    "%s STANDALONE \"Printer Name\" [port]\n"
	                    "%s BACKGROUND \"Printer Name\" [port]\n"
	                    "If port is unspecified, 9100 is assumed.\n",
	            lastPart, lastPart, lastPart, lastPart);

	    return 1;
	}
	
	startPort = serverPort;
	sprintf(strServiceName, "RawPrintServer_%d", serverPort);
	
	switch (command) 
	{
		case INSTALL:
		    CreatePrintServer(argv[0], argv[2], serverPort, 1);
		    break;
		case STANDALONE:
		case BACKGROUND:
		{
		    WORD wVersionRequested;
		    WSADATA wsaData;
		
		    wVersionRequested = MAKEWORD(2, 2);
		    regKey = standaloneKey;
		
		    CreatePrintServer(argv[0], argv[2], serverPort, 0);
		
		    if (command == BACKGROUND)
		      FreeConsole();
		
		    WSAStartup(wVersionRequested, &wsaData);
		
		    while (InnerLoop(serverPort, 0))
		      ;
		
		    WSACleanup();
		    break;
		}
		case REMOVE:
		    DeletePrintServerService(serverPort);
		    break;
		case PRIVATE_SERVICE:
		    SERVICE_TABLE_ENTRY ServiceTable[2];
		    ServiceTable[0].lpServiceName = strServiceName;
		    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
		
		    ServiceTable[1].lpServiceName = NULL;
		    ServiceTable[1].lpServiceProc = NULL;
		
		    // Start the control dispatcher thread for our service
		    StartServiceCtrlDispatcher(ServiceTable);
		
		    WriteToLog("PrintServer exit",true);
		    break;
		default:
		    break;
	}
	
	return 0;
}

/**
 *
 */
int InnerLoop(DWORD port, int service)
{
	//Declare
  	char strTemp[256];
  	DWORD valueSize;
  	SOCKET sock;
  	sockaddr_in addr;
  	sockaddr_in client;
  	int size;

  	sock = socket(AF_INET, SOCK_STREAM, 0);
  	if (sock == INVALID_SOCKET)
	{
    	sprintf(strTemp, "Error: no socket: %d", WSAGetLastError());
    	WriteToLog(strTemp,true);
    	if (service)
		{
      		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
      		ServiceStatus.dwWin32ExitCode = 3;
      		SetServiceStatus(hStatus, &ServiceStatus);
    	}
    	return 0;
  	}
  	
  	addr = {0};
  	addr.sin_family = AF_INET;
  	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  	addr.sin_port = htons(serverPort);
	
	if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
	    sprintf(strTemp, "Error: couldn't bind: %d", WSAGetLastError());
	    WriteToLog(strTemp,true);
	    closesocket(sock);
	    if (service)
		{
	      ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	      ServiceStatus.dwWin32ExitCode = 4;
	      SetServiceStatus(hStatus, &ServiceStatus);
	    }
	    return 0;
	}
	
  	client = {0};
  	size = sizeof(client);
  	
  	while (listen(sock, 2) != SOCKET_ERROR)
	{
		//Declare
		SOCKET sock2;
		
    	sock2 = accept(sock, (struct sockaddr *)&client, &size);
    	if (sock2 != INVALID_SOCKET)
		{
			//Declare
			HANDLE printer;
			HKEY hdlKey;
			DOC_INFO_1 info;
			
	      	printer = NULL;
	
	      	sprintf(strTemp, regKey, strServiceName);
	      	
	      	RegOpenKeyEx(HKEY_LOCAL_MACHINE, strTemp, 0, KEY_ALL_ACCESS, &hdlKey);
	      	
			valueSize = sizeof(printerName);
	      	
			RegQueryValueEx(hdlKey, "Printer", NULL, NULL, (BYTE *)printerName, &valueSize);
	      	
			RegCloseKey(hdlKey);
	
	      	sprintf(
			  	strTemp,
			  	"Accept print job for %s from %d.%d.%d.%d", 
			  	printerName,
				client.sin_addr.S_un.S_un_b.s_b1,
				client.sin_addr.S_un.S_un_b.s_b2,
				client.sin_addr.S_un.S_un_b.s_b3,
				client.sin_addr.S_un.S_un_b.s_b4
			);
	      	WriteToLog(strTemp,true);
	
	      	info.pDocName = (char*)"Forwarded Job";
	      	info.pOutputFile = NULL;
	      	info.pDatatype = (char*)"RAW";
	
	      	//if (!OpenPrinterW((wchar_t*) printerName, &printer, NULL) ||
		  	if (!OpenPrinter(printerName, &printer, NULL) || !StartDocPrinter(printer, 1, (LPBYTE)&info))
		  	{
		        WriteToLog("Error opening print job.",true);
	      	}
		  	else
		  	{
		  		//Declare
	        	char buffer[1024];
	        	DWORD wrote;
	        	
	        	while (1)
				{
					int result = recv(sock2, buffer, sizeof(buffer), 0);
		          	if (result <= 0)
					{
		        		break;
					}
		          	WritePrinter(printer, buffer, result, &wrote);
		          	if (wrote != (DWORD)result)
					{
		            	WriteToLog("Couldn't print all data.",true);
		            	break;
		          	}
	        	}
	        	ClosePrinter(printer);
	    	}
    	}
    	closesocket(sock2);
  	}
  	if (service)
  	{
    	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    	ServiceStatus.dwWin32ExitCode = 5;
    	SetServiceStatus(hStatus, &ServiceStatus);
	}
	closesocket(sock);
	return 1;
}

/**
 *
 */
void ServiceMain(int argc, char **argv)
{
	//Declare
	int error;
	char strTemp[256];
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	HKEY hdlKey;
	DWORD valueSize;
	
	ServiceStatus.dwServiceType 			= SERVICE_WIN32;
	ServiceStatus.dwCurrentState 			= SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted 		= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode 			= 1;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint				= 0;
	ServiceStatus.dwWaitHint 				= 0;
	
	hStatus = RegisterServiceCtrlHandler(strServiceName, (LPHANDLER_FUNCTION)ControlHandler);
	
	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{
		// Registering Control Handler failed
		sprintf(strTemp, "Registering Control Handler failed %d", GetLastError());
	    WriteToLog(strTemp,true);
	    return;
	}
	
	// Initialize Service
	error = InitService();
	if (error)
	{
	    // Initialization failed
	    sprintf(strTemp, "Initialization failed %d", GetLastError());
	    WriteToLog(strTemp,true);
	    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	    ServiceStatus.dwWin32ExitCode = 2;
	    SetServiceStatus(hStatus, &ServiceStatus);
	    return;
	}
	
	// We report the running status to SCM.
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus, &ServiceStatus);
	
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	
	sprintf(strTemp, regKey, strServiceName);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, strTemp, 0, KEY_ALL_ACCESS, &hdlKey);
	
	valueSize = sizeof(printerName);
	RegQueryValueEx(hdlKey, "Printer", NULL, NULL, (BYTE *)printerName, &valueSize);
	
	valueSize = sizeof(serverPort);
	RegQueryValueEx(hdlKey, "Port", NULL, NULL, (BYTE *)&serverPort, &valueSize);
	
	RegCloseKey(hdlKey);
	
	sprintf(strTemp, "%s on %d (%d)", printerName, serverPort, startPort);
	WriteToLog(strTemp,true);
	
	// The worker loop of a service
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING && InnerLoop(serverPort, 1))
		;
	
	WSACleanup();
	
	return;
}

/**
 * Service initialization
 */
int InitService()
{	
	return 0;
}

/**
 * Control handler function
 */
void ControlHandler(DWORD request)
{
	switch (request)
	{
		case SERVICE_CONTROL_STOP:
			WriteToLog("PrintServer stopped.",true);
	
	    	ServiceStatus.dwWin32ExitCode = 0;
	    	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	    	SetServiceStatus(hStatus, &ServiceStatus);
	    	return;
	
	  	case SERVICE_CONTROL_SHUTDOWN:
	    	WriteToLog("PrintServer stopped.",true);
	
	    	ServiceStatus.dwWin32ExitCode = 0;
	    	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	    	SetServiceStatus(hStatus, &ServiceStatus);
	    	return;
	
	  	default:
	    	break;
	}

  	// Report current status
  	SetServiceStatus(hStatus, &ServiceStatus);

  	return;
}
