// SCManager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include<tchar.h>
#include "PService.h"
#include "NTService.h"
using namespace std;
using namespace Func;
//!控制台-注册为服务的应用程序
/*------------------------------------------------------------
****命令号注册服务的方法:
sc create [service name] [binpath]= <option1> <option2>

//范例:
sc create WaterNash	binpath= "C:\1.exe" displayname= "Water" depend= Tcpip start auto
每一个=后面都必须跟一个空格
删除服务使用sc delete [service name]


****服务创建成功以后可以使用net start [service name]进行启动
查看服务的状况,在cmd中输入services.msc

*/




//本程序使用方法

//1.以管理员权限运行cmd
//2.切换到本程序坐在目录
//3.SCManager -install  安装服务
//4.SCManager -start 开启服务
//5.SCManager -uninstall 卸载服务
//6.SCManager -stop 停止服务
//7.SCManager -debug 调试自定义的程序

//功能说明:在exe所在的目录生成11个txt文件
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

	//单实例的实现方式

//  	if ( (argc > 1) &&
//  		((*argv[1] == '-') || (*argv[1] == '/')) )
//  	{
//  		if ( _wcsicmp( L"install", argv[1]+1 ) == 0 )//_wcsicmp函数用户字符串全部小写转化，然后比较大小
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

