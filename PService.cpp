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
		if (IsInstall())//判断服务是否已经注册
		{
			printf("Service has Exists\n");
			return;
		}
		//注册服务
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

		//注册表，用于记录系统日志
		_TCHAR szKey[MAX_PATH];
		wsprintf(szKey,_T("%s\\%s"),SYSTEMLOG,PServiceManage::Instance()->ServiceName());
		HKEY hKey;
		UINT f = EVENTLOG_ERROR_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_INFORMATION_TYPE;//支持的事件类型
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
		SC_HANDLE pService = ::OpenService(pSC,PServiceManage::Instance()->ServiceName(),SERVICE_STOP | DELETE);//打开服务的权限(进行停止和删除操作)
		if (pService == NULL)
		{
			::CloseServiceHandle(pSC);
			printf("Open Service Failed\n");
			return;
		}
		SERVICE_STATUS pStatus;
		::ControlService(pService,SERVICE_CONTROL_STOP,&pStatus);//给pService 发送停止服务的请求,  前提是pService打开服务的时候要有停止服务的权限

		//删除服务
		BOOL pDelete = ::DeleteService(pService);
		::CloseServiceHandle(pSC);
		::CloseServiceHandle(pService);

		if (!pDelete)
			printf("Service Delete failed!\n");
		else
		{
			//删除注册记录
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
			printf_s("错误%s",buf);
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
		//打开服务器控制管理器
		SC_HANDLE pSC = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if (pSC != NULL)
		{
			//打开服务
			SC_HANDLE pService = ::OpenService(pSC,PServiceManage::Instance()->ServiceName(),SERVICE_QUERY_CONFIG);//第三个参数随意
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
			printf("错误%s",buf);
			//free(buf);
		}
		return pResult;
	}

	//记录日志事件到系统中
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
		status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;//服务类型
		//status.dwCurrentState = SERVICE_STOPPED;//服务当前的状态
		status.dwControlsAccepted = SERVICE_ACCEPT_STOP;//服务可以接收的控制码(  |SERVICE_CONTROL_INTERROGATE;所有的服务均可接受的)
		status.dwWin32ExitCode = 0;//服务错误码
		status.dwServiceSpecificExitCode = 0;//服务错误码
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
			LogErrorEvent(_T("打开文件失败!"));
			break;
		}

		char info[]="测试文本";
		//转化成多字节
		DWORD a;
		BOOL result = WriteFile(hFile,info,sizeof(info),&a,NULL);
		if (result == FALSE)
		{
			LogErrorEvent(_T("写入数据失败!"));
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
		//自定义代码区域
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
		printf("异常错误!");
	}
	return;
}

// void WINAPI PServiceManage::ServiceMain( DWORD argc,_TCHAR *argv[] )//因为这个里面有个隐藏的参数是this,编译器自己加的
// {
// 	return;
// }
