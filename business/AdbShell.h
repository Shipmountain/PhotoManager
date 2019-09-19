#ifndef ADB_SHELL_H
#define ADB_SHELL_H
#include <string>
#include <log.h>

namespace adb
{
    class AdbShell :public Log::ModuleLog
    {
    public:
        AdbShell();
        ~AdbShell();
    public:
        bool init(unsigned result_size);
        void uninit();

        bool exe_adb_script(const std::string& script);
        const std::string& get_result()
        {
            return result_;
        }

        static bool start_adb_server();
        static void stop_adb_server();

    private:
        bool create_piep(void** read_handle, void** write_handle);
        bool create_process(const std::string& script, void* write_handle);
    private:
        std::string result_;
        char*       buffer_;
        std::string adb_path_;

    };
}

#endif  //ADB_SHELL_H

