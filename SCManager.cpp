// SCManager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include<tchar.h>
#include "PService.h"
#include "NTService.h"
using namespace std;
using namespace Func;
//!����̨-ע��Ϊ�����Ӧ�ó���
/*------------------------------------------------------------
****�����ע�����ķ���:
sc create [service name] [binpath]= <option1> <option2>

//����:
sc create WaterNash	binpath= "C:\1.exe" displayname= "Water" depend= Tcpip start auto
ÿһ��=���涼�����һ���ո�
ɾ������ʹ��sc delete [service name]


****���񴴽��ɹ��Ժ����ʹ��net start [service name]��������
�鿴�����״��,��cmd������services.msc

*/




//������ʹ�÷���

//1.�Թ���ԱȨ������cmd
//2.�л�������������Ŀ¼
//3.SCManager -install  ��װ����
//4.SCManager -start ��������
//5.SCManager -uninstall ж�ط���
//6.SCManager -stop ֹͣ����
//7.SCManager -debug �����Զ���ĳ���

//����˵��:��exe���ڵ�Ŀ¼����11��txt�ļ�
int _tmain(int argc, _TCHAR* argv[])
{
  	if (argc > 1)
  	{
  		if ((_tcsicmp(L"-install",argv[1]) == 0) 
  			|| (_tcsicmp(L"--install",argv[1]) == 0) 
  			 || (_tcsicmp(L"/install",argv[1]) == 0 ) )
  		{
  			Install();
  		}
  		else if ((_tcsicmp(L"-uninstall",argv[1]) == 0) 
  			|| (_tcsicmp(L"--uninstall",argv[1]) == 0) 
  			|| (_tcsicmp(L"/uninstall",argv[1]) == 0 ) )
  		{
  			//remove
  			Remove();
  		}
  		else if ((_tcsicmp(L"-start",argv[1]) == 0) 
  			|| (_tcsicmp(L"--start",argv[1]) == 0) 
  			|| (_tcsicmp(L"/start",argv[1]) == 0 ) )
  		{
  			//start
  			Start();
  		}
  		else if ((_tcsicmp(L"-stop",argv[1]) == 0) 
  			|| (_tcsicmp(L"--stop",argv[1]) == 0) 
  			|| (_tcsicmp(L"/stop",argv[1]) == 0 ) )
  		{
  			//stop
  			Stop();
  		}
  		else if ((_tcsicmp(L"-debug",argv[1]) == 0) 
  			|| (_tcsicmp(L"--debug",argv[1]) == 0) 
  			|| (_tcsicmp(L"/debug",argv[1]) == 0 ) )
  		{
  			//debug
  			Debug();
  		}
  		else 
  		{
  			//help
  			Help();
  		}
  	}
  	else
  	{
  		Run();
  	}

	//----------------------------------------------------------------------

	//��ʵ����ʵ�ַ�ʽ

//  	if ( (argc > 1) &&
//  		((*argv[1] == '-') || (*argv[1] == '/')) )
//  	{
//  		if ( _wcsicmp( L"install", argv[1]+1 ) == 0 )//_wcsicmp�����û��ַ���ȫ��Сдת����Ȼ��Ƚϴ�С
//  		{
// 			LNTService::Instance()->Install();
//  		}
//  		else if ( _wcsicmp( L"remove", argv[1]+1 ) == 0 )
//  		{
//  			LNTService::Instance()->Remove();
//  		}
//  		else if ( _wcsicmp( L"start", argv[1]+1 ) == 0 )
//  		{
//  			LNTService::Instance()->Start();
//  		}
//  		else if ( _wcsicmp( L"stop", argv[1]+1 ) == 0 )
//  		{
//  			LNTService::Instance()->Stop();
//  		}
//  		else if ( _wcsicmp( L"test", argv[1]+1 ) == 0 )
//  		{
//  			LNTService::Instance()->ConsoleMode();
//  		}
//  		else
//  		{
//  			LNTService::Instance()->ShowHelp();
//  		}
//  		LNTService::Instance()->PrintLastError (LNTService::Instance()->Name());
//  	}
//  	else
//  		LNTService::Instance()->Execute();
 	return true;

}

