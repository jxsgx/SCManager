#include "stdafx.h"
#include "NTService.h"
LNTService *LNTService::s_ServiceInstance = NULL;
bool LNTService::Succeeded (BOOL success) 
{
	DWORD SysError = GetLastError();
	return SysError == ERROR_SUCCESS;
}
bool LNTService::Open (SC_HANDLE &hService) 
{
	bool success = false;
	SC_HANDLE	hSCM;
	if ((hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS)) != NULL)	
	{
		hService = OpenService(hSCM,Name(),SERVICE_ALL_ACCESS);
		success = Succeeded (hService != NULL);
		CloseServiceHandle (hSCM);
	}
	else 
		Succeeded (false);
	return success;
}
void LNTService::Close (SC_HANDLE hService) 
{
	CloseServiceHandle (hService);
}
void LNTService::SetStatus (DWORD status) 
{
	s_StatusCode  = status;

	SERVICE_STATUS ss;
	ss.dwServiceType              = SERVICE_WIN32_OWN_PROCESS;
	ss.dwCurrentState             = s_StatusCode;
	ss.dwControlsAccepted	      = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
	ss.dwWin32ExitCode            = NOERROR;
	ss.dwServiceSpecificExitCode  = NOERROR;
	ss.dwCheckPoint               = 0;
	ss.dwWaitHint                 = 3000;

	if (! Succeeded (SetServiceStatus (s_StatusHandle,&ss)));

}
void WINAPI ServiceHandler (DWORD control) 
{
	switch (control) 
	{
	case SERVICE_CONTROL_INTERROGATE:
		LNTService::Instance()->SetStatus (s_StatusCode);
		break;
	case SERVICE_CONTROL_STOP:
		{
			//LOGMESSAGE(gc_NTServiceLog,"SERVICE_CONTROL_STOP");

			LNTService::Instance()->SetStatus (SERVICE_STOP_PENDING);
			break;
		}
	case SERVICE_CONTROL_SHUTDOWN:
		{
			//LOGMESSAGE(gc_NTServiceLog,"SERVICE_CONTROL_SHUTDOWN");

			LNTService::Instance()->SetStatus (SERVICE_STOP_PENDING);
			break;
		}
	}
}
BOOL WINAPI ConsoleHandler1 (DWORD dwCtrlType) 
{
	if (dwCtrlType == CTRL_LOGOFF_EVENT)
	{	
		//LOGMESSAGE(gc_NTServiceLog,"CTRL_LOGOFF_EVENT");
		//s_ServiceDlg->EndDialog(0);
	}
	return TRUE;
}

void WINAPI ServiceMain1 (DWORD argc, WCHAR* argv[]) 
{
	s_StatusHandle = RegisterServiceCtrlHandler(LNTService::Instance()->Name(),ServiceHandler);

	LNTService::Instance()->SetStatus (SERVICE_START_PENDING);
	LNTService::Instance()->SetStatus (SERVICE_RUNNING);
	int a= 3;
	while(a)
	{
		Sleep(1000);
		a--;
	}
	LNTService::Instance()->SetStatus (SERVICE_STOP_PENDING);

	LNTService::Instance()->SetStatus (SERVICE_STOPPED);
}


void LNTService::ShowHelp()
{

}
void LNTService::PrintLastError (const WCHAR *cap) 
{

}
bool LNTService::Start() 
{
	bool success = false;
	SC_HANDLE   hService;

	if (Open(hService)) {
		success = Succeeded (StartService (hService,0,NULL));
		Close(hService);
	}
	return success;
}
bool LNTService::Stop() 
{
	bool         success = false;
	SC_HANDLE   hService;

	if (Open(hService)) {
		SERVICE_STATUS   state;
		success = Succeeded (ControlService (hService,SERVICE_CONTROL_STOP,&state));
		Close(hService);
	}
	return success;
}
bool LNTService::Remove() 
{
	Stop();

	bool         success = false;
	SC_HANDLE   hService;

	if (Open (hService)) {
		success = Succeeded (DeleteService (hService));
		Close (hService);

		HKEY hKey;
		if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,REG_EVENTLOG,0,KEY_ALL_ACCESS,&hKey)== ERROR_SUCCESS) {
			RegDeleteKey (hKey,Name());
			RegCloseKey (hKey);
		}
	}
	return success;
}
bool LNTService::Install()//最好增加一个服务器是否已经安装的判断
{
	bool        success = false;
	SC_HANDLE   hService,hSCManager;
	WCHAR        imagePath[MAX_PATH];

	GetModuleFileName(NULL,imagePath,MAX_PATH);
	if (Succeeded ((hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS))!= NULL)) 
	{
		if (Succeeded ((hService = ::CreateService (hSCManager,
			Name(),
			L"23",
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,
			//SERVICE_DEMAND_START
			SERVICE_AUTO_START,SERVICE_ERROR_NORMAL,
			imagePath,NULL,NULL,NULL,NULL,NULL)) != NULL)) 
		{

			success = true;
		}
		CloseServiceHandle (hSCManager);
	}

	if (success) 
	{
		UINT f = EVENTLOG_ERROR_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_INFORMATION_TYPE;

		WCHAR   szKey [MAX_PATH];
		wsprintf (szKey,L"%s\\%s",REG_EVENTLOG,Name());

		HKEY hKey;
		if (RegCreateKey (HKEY_LOCAL_MACHINE,szKey,&hKey) == ERROR_SUCCESS) 
		{
			WCHAR   mod[MAX_PATH];
			DWORD  len = GetModuleFileName (NULL,mod,MAX_PATH);

			RegSetValueEx(hKey,L"TypesSupported",0,REG_DWORD,(BYTE*) &f,sizeof(DWORD));
			RegSetValueEx(hKey,L"EventMessageFile",0,REG_SZ,(BYTE*) mod,len+1);
			RegCloseKey (hKey);
		}
	}
	return success;
}
bool LNTService::Execute() 
{
	Succeeded(SetConsoleCtrlHandler (ConsoleHandler1,TRUE));
	s_EventLog = RegisterEventSource (NULL,Name());

	SERVICE_TABLE_ENTRY entries[2];
	entries[0].lpServiceName = (WCHAR*) Name();
	entries[0].lpServiceProc = ServiceMain1;
	entries[1].lpServiceName = NULL;
	entries[1].lpServiceProc = NULL;

	bool  success = Succeeded(StartServiceCtrlDispatcher(entries));
	if (! success);

	return success;
}
void LNTService::ConsoleMode (void) 
{

}

LNTService * LNTService::Instance()
{
	if (s_ServiceInstance == NULL)
	{
		s_ServiceInstance = new LNTService();
	}
	return s_ServiceInstance;
}
