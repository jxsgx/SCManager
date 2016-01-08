#include "stdafx.h"
#include "PService.h"
using namespace Func;
PServiceManage *PServiceManage::s_PServiceManage = NULL;
static SERVICE_STATUS_HANDLE hServiceCtrlStatus = NULL;
static HANDLE hEventSource = NULL;

namespace cc
{
	string ff = "fdd";
}



namespace Func
{
	_TCHAR * GetSysErrorMessage()
	{
		DWORD ErrorCode = GetLastError();
		if(ErrorCode == 0)
			return _T("");
		_TCHAR szBuf[MAX_PATH];
		memset(szBuf,0,MAX_PATH);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ARGUMENT_ARRAY,NULL,ErrorCode,0,szBuf,MAX_PATH*2,NULL);
		return szBuf;
	}

	void Install()
	{
		if (IsInstall())//�жϷ����Ƿ��Ѿ�ע��
		{
			printf("Service has Exists\n");
			return;
		}
		//ע�����
		SC_HANDLE pSC = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if(pSC == NULL)
		{
			printf("Open SCManager failed!\n");
			return;
		}
		_TCHAR szFilePath[MAX_PATH];
		::GetModuleFileName(NULL,szFilePath,MAX_PATH);//path for the file containing the current process
		SC_HANDLE pService = ::CreateService(pSC,PServiceManage::Instance()->ServiceName(),PServiceManage::Instance()->DisPlayName(),SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,szFilePath,
			NULL,NULL,_T(""),NULL,NULL);
		if (pService == NULL)
		{
			::CloseServiceHandle(pSC);
			printf("Install Service failed!");
			return;
		}

		::CloseServiceHandle(pService);
		::CloseServiceHandle(pSC);

		//ע������ڼ�¼ϵͳ��־
		_TCHAR szKey[MAX_PATH];
		wsprintf(szKey,_T("%s\\%s"),SYSTEMLOG,PServiceManage::Instance()->ServiceName());
		HKEY hKey;
		UINT f = EVENTLOG_ERROR_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_INFORMATION_TYPE;//֧�ֵ��¼�����
		if (RegCreateKey(HKEY_LOCAL_MACHINE,szKey,&hKey) == ERROR_SUCCESS)
		{
			wchar_t logPath[MAX_PATH];
			memset(logPath,0,MAX_PATH*2);
			DWORD length = GetModuleFileName(NULL,logPath,MAX_PATH);
			RegSetValueEx(hKey,_T("TypesSupported"),0,REG_DWORD,(BYTE*)&f,sizeof(DWORD));
			RegSetValueEx(hKey,_T("EventMessageFile"),0,REG_SZ,(BYTE*)logPath,length*2);
			RegCloseKey(hKey);
		}

	}

	void Remove()
	{
		if (!IsInstall())//
		{
			printf(("Service isn't exists !\n"));
			return;
		}
		SC_HANDLE pSC = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if (pSC == NULL)
		{
			printf("Open SCManager failed!\n");
			return;
		}
		SC_HANDLE pService = ::OpenService(pSC,PServiceManage::Instance()->ServiceName(),SERVICE_STOP | DELETE);//�򿪷����Ȩ��(����ֹͣ��ɾ������)
		if (pService == NULL)
		{
			::CloseServiceHandle(pSC);
			printf("Open Service Failed\n");
			return;
		}
		SERVICE_STATUS pStatus;
		::ControlService(pService,SERVICE_CONTROL_STOP,&pStatus);//��pService ����ֹͣ���������,  ǰ����pService�򿪷����ʱ��Ҫ��ֹͣ�����Ȩ��

		//ɾ������
		BOOL pDelete = ::DeleteService(pService);
		::CloseServiceHandle(pSC);
		::CloseServiceHandle(pService);

		if (!pDelete)
			printf("Service Delete failed!\n");
		else
		{
			//ɾ��ע���¼
			HKEY hKey;
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,SYSTEMLOG,0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS)
			{
				RegDeleteKey(hKey,PServiceManage::Instance()->ServiceName());
				RegCloseKey(hKey);
			}
			printf("Service Delete Success!\n");
		}
		return;	
	}

	void Start()
	{
		SC_HANDLE pSC = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if(pSC == NULL)
		{
			printf("Open SCManager failed!\n");
			return;
		}
		SC_HANDLE pService = ::OpenService(pSC,PServiceManage::Instance()->ServiceName(),SERVICE_ALL_ACCESS);
		if (pService  == NULL)
		{
			CloseServiceHandle(pSC);
			printf("Open Service failed!\n");
			return;
		}
		if (!::StartService(pService,0,NULL))
		{
			CloseServiceHandle(pSC);
			CloseServiceHandle(pService);
			printf("Start Service failed!\n");
			return;
		}
		CloseServiceHandle(pSC);
		CloseServiceHandle(pService);
	}

	void Stop()
	{
		SC_HANDLE pSC = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if(pSC == NULL)
		{
			printf("Open SCManager failed!\n");
			return;
		}
		SC_HANDLE pService = ::OpenService(pSC,PServiceManage::Instance()->ServiceName(),SERVICE_STOP);
		if (pService  == NULL)
		{
			CloseServiceHandle(pSC);
			printf("Open Service failed!\n");
			return;
		}
		SERVICE_STATUS pStatus;
		if (!::ControlService(pService,SERVICE_CONTROL_STOP,&pStatus))
		{
			CloseServiceHandle(pSC);
			CloseServiceHandle(pService);
			printf("Stop Service failed!\n");
			return;
		}
		CloseServiceHandle(pSC);
		CloseServiceHandle(pService);
	}

	void Debug()
	{
		PServiceManage::Instance()->CustomeFunc();
	}

	void Help()
	{

	}

	void Run()
	{
		//SetConsoleCtrlHandler (ConsoleHandler,TRUE);

		SERVICE_TABLE_ENTRY tables[2];
		tables[0].lpServiceName = PServiceManage::Instance()->ServiceName();
		tables[0].lpServiceProc = ServiceMain;
		tables[1].lpServiceName = NULL;
		tables[1].lpServiceProc = NULL;
		BOOL success = ::StartServiceCtrlDispatcher(tables);
		if (!success)
		{
			_TCHAR *buf = (_TCHAR*)calloc(MAX_PATH*2,sizeof(_TCHAR));
			memcpy(buf,GetSysErrorMessage(),MAX_PATH*2);
			//buf = GetSysErrorMessage();			
			printf_s("����%s",buf);
			LogErrorEvent(_T("Register Service Main Function Error!"));
			return;
		}
	}

	void Init()
	{
		hServiceCtrlStatus = NULL;
		hEventSource = NULL;

	}



	//SC_HANDLE OpenSCManager(  LPCTSTR lpMachineName,   // computer name
	//						LPCTSTR lpDatabaseName,  // SCM database name
	//						DWORD dwDesiredAccess    // access type);
	bool IsInstall()
	{
		bool pResult = false;
		//�򿪷��������ƹ�����
		SC_HANDLE pSC = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if (pSC != NULL)
		{
			//�򿪷���
			SC_HANDLE pService = ::OpenService(pSC,PServiceManage::Instance()->ServiceName(),SERVICE_QUERY_CONFIG);//��������������
			if (pService != NULL)
			{
				pResult = true;
				::CloseServiceHandle(pService);
			}
			::CloseServiceHandle(pSC);
		}
		else
		{
			_TCHAR *buf = (_TCHAR*)calloc(MAX_PATH,sizeof(_TCHAR));
			buf = GetSysErrorMessage();			
			printf("����%s",buf);
			//free(buf);
		}
		return pResult;
	}

	//��¼��־�¼���ϵͳ��
	void LogErrorEvent(LPCTSTR pFormat,...)
	{
		_TCHAR pMsg[MAX_PATH];

		va_list pArg;
		va_start(pArg,pFormat);
		_vstprintf(pMsg,pFormat,pArg);
		va_end(pArg);

		LPCTSTR pStr[1] = {pMsg};
		if (hEventSource == NULL)
			hEventSource = RegisterEventSource(NULL,PServiceManage::Instance()->ServiceName());
		ReportEvent(hEventSource,EVENTLOG_ERROR_TYPE,0,EVENTID,NULL,1,0,pStr,NULL);
	}

	void SetStatus(DWORD CurrentState)
	{
		SERVICE_STATUS status;
		status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;//��������
		//status.dwCurrentState = SERVICE_STOPPED;//����ǰ��״̬
		status.dwControlsAccepted = SERVICE_ACCEPT_STOP;//������Խ��յĿ�����(  |SERVICE_CONTROL_INTERROGATE;���еķ�����ɽ��ܵ�)
		status.dwWin32ExitCode = 0;//���������
		status.dwServiceSpecificExitCode = 0;//���������
		status.dwCheckPoint = 0;
		status.dwWaitHint = 3000;

		status.dwCurrentState = CurrentState;
		::SetServiceStatus(hServiceCtrlStatus,&status);
	}

	void SetRegisterMain()
	{
		hServiceCtrlStatus = ::RegisterServiceCtrlHandler(PServiceManage::Instance()->ServiceName(),ServiceCtrl);
		if (hServiceCtrlStatus == NULL)
		{
			//LogErrorEvent(_T("Register Service Control Function Error!"));
			return;
		}
		SetStatus(SERVICE_START_PENDING);
		SetStatus(SERVICE_RUNNING);
	}
}

PServiceManage::PServiceManage()
{
	//Init();
	pFlag = true;
}

PServiceManage::~PServiceManage()
{
	if (s_PServiceManage != NULL)
	{
		delete s_PServiceManage;
		s_PServiceManage = NULL;
	}
}

PServiceManage * PServiceManage::Instance()
{
	if (s_PServiceManage == NULL)
	{
		s_PServiceManage = new PServiceManage();
	}
	return s_PServiceManage;
}

void PServiceManage::CustomeFunc()
{	
	int i = 0;
	while(pFlag)
	{
		Sleep(2000);
		if (i>10)
			break;
		_TCHAR pPath[MAX_PATH];
		DWORD length = GetModuleFileName(NULL,pPath,MAX_PATH);
		wstring filePath;
		filePath.assign(pPath,length+1);
		int pos = filePath.find_last_of(_T("\\"));
		wstring fileName = filePath.substr(0,pos);

		srand((unsigned)time(0));

		int num = rand()%10000;
		//  		char buffer[20];
		//  		memset(buffer,0,20);
		//  		_itoa(num,buffer,10);



		_TCHAR tmp[20];
		memset(tmp,0,20);
		_itow(num,tmp,10);


		//_vsnwprintf(tmp,20,L"%d",buffer);
		fileName.append(_T("\\"));
		fileName.append(tmp);
		fileName.append(_T(".txt"));

		HANDLE hFile = ::CreateFile(fileName.c_str(),GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LogErrorEvent(_T("���ļ�ʧ��!"));
			break;
		}

		char info[]="�����ı�";
		//ת���ɶ��ֽ�
		DWORD a;
		BOOL result = WriteFile(hFile,info,sizeof(info),&a,NULL);
		if (result == FALSE)
		{
			LogErrorEvent(_T("д������ʧ��!"));
		}
		CloseHandle(hFile);
		i++;
		//CreateFile()
	}
}

void PServiceManage::PostStopMessage()
{
	pFlag = false;
}

void WINAPI ServiceCtrl( DWORD dwOpcode )
{
	switch(dwOpcode)
	{
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_STOP:
		{
			PServiceManage::Instance()->PostStopMessage();
			SetStatus(SERVICE_STOP_PENDING);
			break;
		}
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;

	}
}


void WINAPI ServiceMain( DWORD argc,_TCHAR *argv[] )
{
	try
	{
		SetRegisterMain();
		//�Զ����������
// 		while(PServiceManage::Instance()->GetFlag())
// 		{
// 			Sleep(1000);
// 		}
		PServiceManage::Instance()->CustomeFunc();
		//------------------
		SetStatus(SERVICE_STOPPED);
	}
	catch(std::exception &e)
	{
		printf(e.what());
	}
	catch(...)
	{
		printf("�쳣����!");
	}
	return;
}

// void WINAPI PServiceManage::ServiceMain( DWORD argc,_TCHAR *argv[] )//��Ϊ��������и����صĲ�����this,�������Լ��ӵ�
// {
// 	return;
// }
