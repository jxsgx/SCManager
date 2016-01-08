#pragma once
#include "PService.h"
class LNTService
{
protected:
	LNTService(void){}
	virtual ~LNTService(void){}
public:
	static LNTService *Instance();
	//!��������
	virtual const wchar_t* Name(void){ return L"NTService";}
	//!��ʾ����-Ĭ�Ϸ�����
	virtual const wchar_t* DisplayName(void){return L"NTService1";}
	//!�����÷���Ĺ���
	virtual const wchar_t* Description(void){return L"NTService2";};
	bool Succeeded (BOOL success);
	bool Open (SC_HANDLE &hService);
	void ConsoleMode (void);
	bool Execute();
	bool Install()/**/;
	bool Remove();
	bool Stop();
	bool Start();
	void PrintLastError (const WCHAR *cap);
	void ShowHelp();
	void Close (SC_HANDLE hService);
	void SetStatus (DWORD status);

protected:
	static LNTService *s_ServiceInstance;
};


#define REG_EVENTLOG L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System"


static const wchar_t *gc_NTServiceLog = L"NTService";
static SERVICE_STATUS_HANDLE  s_StatusHandle = NULL;
static DWORD                  s_StatusCode = SERVICE_STOPPED;
static HANDLE                 s_EventLog;
static bool                   s_Terminated;


