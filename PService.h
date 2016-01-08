#pragma once
#include <string>
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>
#include <WinSvc.h>
#include <WinReg.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>
using namespace std;
#define SERVICENUM 2
#define SYSTEMLOG _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application")
#define EVENTID  0
void WINAPI ServiceMain(DWORD argc,_TCHAR *argv[]);
void WINAPI ServiceCtrl(DWORD dwOpcode);

class PServiceManage
{
public:
	static PServiceManage *Instance();
protected:
	PServiceManage();
	~PServiceManage();
	static PServiceManage *s_PServiceManage;
	
public:
	virtual void CustomeFunc();
	virtual _TCHAR *ServiceName() const{ return L"Water"; }
	virtual _TCHAR *DisPlayName() const { return L"Water Cloud Inc Register by nash";} 
	virtual _TCHAR *Describe() const { return L"专注于成长";}

public:
	void PostStopMessage();
	_TCHAR *GetSysErrorMessage();
	bool GetFlag(){ return pFlag;}
private:
	bool pFlag;
	SERVICE_STATUS status;
	SERVICE_STATUS_HANDLE hServiceCtrlStatus;
	HANDLE hEventSource;
};



namespace Func
{
	void Init();
	void Install();
	bool IsInstall();
	void Remove();
	void Start();
	void Stop();
	void Debug();
	void Help();
	void Run();
	void LogErrorEvent(LPCTSTR pFormat,...);//用于记录错误日志
	void SetStatus(DWORD CurrentState);
	void SetRegisterMain();
}

