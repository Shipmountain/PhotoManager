#include "AdbShell.h"
#include <system.h>
#include <Psapi.h>

namespace adb
{
    static const int k_default_buff_size = 4096;
    static const int k_default_result_size = 1024*1024*2;

    AdbShell::AdbShell() :ModuleLog("AdbShell")
        , buffer_(NULL)
    {

    }

    AdbShell::~AdbShell()
    {
        uninit();
    }

    bool AdbShell::init(unsigned result_size)
    {
        if (!sys::get_module_path(adb_path_))
        {
            TraceLog(LogError, "get module path fail:%d", GetLastError());
            return false;
        }
        adb_path_.append("adb.exe");

        buffer_ = (char*)malloc(k_default_buff_size);
        if (NULL == buffer_)
        {
            TraceLog(LogError, "CreatePipe fail:%d", GetLastError());
            return false;
        }

        if (0 != result_size)
        {
            result_.resize(result_size);
        }
        else
        {
            result_.resize(k_default_result_size);
        }      

        return true;
    }

    void AdbShell::uninit()
    {
        if (NULL != buffer_)
        {
            free(buffer_);
            buffer_ = NULL;
        }
    }

    bool AdbShell::exe_adb_script(const std::string& script)
    {
        result_.clear();
        void* read_handle = NULL;
        void* write_handle = NULL;
        if (!create_piep(&read_handle, &write_handle))
        {
            return false;
        }

        if (!create_process(script, write_handle))
        {
            CloseHandle(read_handle);
            return false;
        }        

        DWORD bytesRead = 0;
        while (true)
        {
            memset(buffer_, 0, k_default_buff_size);
            if (::ReadFile(read_handle, buffer_, k_default_buff_size - 1, &bytesRead, NULL) == NULL)
            {
                break;
            }

            result_.append(buffer_);
        }  

        CloseHandle(read_handle);
        if (result_.size() <= 2)
        {
            TraceLog(LogError, "empty data");
            return true;
        }

        //remove the enter key
        if (0x0a == result_[result_.size() - 1])
        {
            result_.erase(result_.size() - 1);
        }

        if (0x0d == result_[result_.size() - 1])
        {
            result_.erase(result_.size() - 1);
        }

        if (result_.find("device not found") != std::string::npos ||
            result_.find("no devices") != std::string::npos)
        {
            result_.clear();
            TraceLog(LogError, "device not found");
            return false;
        }

        return true;
    }

    bool AdbShell::create_piep(void** read_handle, void** write_handle)
    {
        //创建管道
        SECURITY_ATTRIBUTES sa = { 0 };
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        if (!::CreatePipe(read_handle, write_handle, &sa, 0))
        {
            TraceLog(LogError, "CreatePipe fail:%d", GetLastError());
            return false;
        }

        ::SetHandleInformation(*read_handle, HANDLE_FLAG_INHERIT, 0);
        return true;
    }

    bool AdbShell::create_process(const std::string& script, void* write_handle)
    {
        //创建进程
        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };

        si.cb = sizeof(STARTUPINFOA);
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = write_handle;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.dwFlags = STARTF_USESTDHANDLES;
        int ret = ::CreateProcessA(
            adb_path_.c_str(),                              /* program path  */
            (LPSTR)script.c_str(),
            /* the fork-server argument will set the
            debug = 2 in the child           */
            NULL,                   /* process handle is not inheritable */
            NULL,                    /* thread handle is not inheritable */
            TRUE,                          /* yes, inherit some handles */
            DETACHED_PROCESS, /* the new process doesn't have a console */
            NULL,                     /* use parent's environment block */
            NULL,                    /* use parent's starting directory */
            &si,                 /* startup info, i.e. std handles */
            &pi);

        CloseHandle(write_handle);

        if (!ret) 
        {
            TraceLog(LogError, "CreateProcessA fail:%d", GetLastError());
            return false;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    bool AdbShell::start_adb_server()
    {
        std::string cmd;
        if (!sys::get_module_path(cmd))
        {
            GenericLog("AdbShell", LogError, "get module path fail:%d", GetLastError());
            return false;
        }
        cmd.append("adb.exe devices");

        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(STARTUPINFOA);
        ::GetStartupInfoA(&si);
        si.wShowWindow = SW_HIDE;
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        if (!::CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
        {
            GenericLog("AdbShell", LogError, "CreateProcessA fail:%d", GetLastError());
            return false;
        }

        GenericLog("AdbShell", LogInfo, "start adb server success");
        return true;
    }

    void AdbShell::stop_adb_server()
    {       
        DWORD aProcesses[1024] = { 0 };
        DWORD cbNeeded = 0;
        DWORD cProcesses = 0;

        if (!::EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
            return;

        cProcesses = cbNeeded / sizeof(DWORD);
        for (unsigned i = 0; i < cProcesses; i++)
        {
            if (aProcesses[i] != 0)
            {
                bool shouldKill = false;
                wchar_t szProcessName[MAX_PATH] = L"<unknown>";

                //Get a handle to the process.
                HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION |
                    PROCESS_VM_READ | PROCESS_TERMINATE,
                    FALSE, aProcesses[i]);
                if (NULL != hProcess)
                {
                    HMODULE hMod;
                    DWORD cbNeeded;

                    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
                        &cbNeeded))
                    {
                        GetModuleFileNameExW(hProcess, hMod, szProcessName,
                            sizeof(szProcessName) / sizeof(TCHAR));
                        int len = wcslen(szProcessName);
                        if (!wcscmp(L"\\adb.exe", szProcessName + len - 8))
                        {
                            shouldKill = true;
                        }
                    }
                }

                if (shouldKill) TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
    }
}


