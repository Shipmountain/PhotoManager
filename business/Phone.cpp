#include "Phone.h"
#include <character_cvt.hpp>

namespace phone
{
    Phone::Phone(const std::string& serial_no) :ModuleLog("Phone")
        , serial_no_(serial_no), adb_shell_(NULL), show_all_(false)
    {
        root_path_ = "/storage/sdcard0/";
    }

    Phone::~Phone()
    {
        uninit();
    }

    bool Phone::init(unsigned def_result_size/* = 1024 * 1024 * 2*/)
    {
        adb_shell_ = std::make_shared<adb::AdbShell>();
        if (NULL == adb_shell_)
        {
            TraceLog(LogError, "new AdbShell fail");
            return false;
        }

        return adb_shell_->init(def_result_size);
    }

    void Phone::uninit()
    {
        if (NULL != adb_shell_)
        {
            adb_shell_->uninit();
            adb_shell_.reset();
            adb_shell_ = NULL;
        }
    }

    void Phone::parse_result(const std::string& result, const std::string& dir, std::vector<FileInfo>& file_lst)
    {
        if (std::string::npos != result.find("total 0"))
        {
            return;
        }

        std::wstring dir_w = character::mbyte_to_unicode(dir);
        char tmp[256] = { 0 };
        FileInfo file_info;
        file_info.is_phone_ = true;
        file_info.serial_no_ = character::mbyte_to_unicode(serial_no_);
        bool set_file_type = true;

        const char* p = result.c_str();
        p = p + result.size() - 1;
        const char* end = p;

        int len = result.size();
        int sp_num = 0;  //分隔符个数
        while (len > 0)
        {
            if ('\n' == *p)
            {
                --p;
                --len;

                if ('\r' == *p)
                {
                    --p;
                    --len;
                }
                sp_num = 0;
                end = p;
                continue;
            }
            else if (' ' == *p)
            {
                ++sp_num;
                if (1 == sp_num)
                {
                    set_file_type = true;
                    memset(tmp, 0, 256);
                    if ('/' == *end)
                    {
                        file_info.file_type_ = eFileDir;
                        --end;
                        set_file_type = false;
                    }

                    memcpy(tmp, p + 1, end - p);
                    std::string file_name = character::utf8_to_mbyte(tmp);
                    file_info.file_name_ = character::mbyte_to_unicode(file_name);
                    file_info.file_path_ = dir_w;
                    if (set_file_type)
                    {
                        file_info.file_type_ = get_file_type(file_name);
                    }
                    end = p - 1;
                }
                else if (2 == sp_num)
                {
                    end = p - 1;
                }
                else if (4 == sp_num)
                {
                    memset(tmp, 0, 256);
                    memcpy(tmp, p + 1, end - p);
                    file_info.modify_time_ = character::mbyte_to_unicode(tmp);
                    int pos = file_info.modify_time_.find('.');
                    if (pos != std::string::npos)
                    {
                        file_info.modify_time_ = file_info.modify_time_.substr(0, pos);
                    }
                    end = p - 1;
                }
                else if (5 == sp_num)
                {
                    memset(tmp, 0, 256);
                    memcpy(tmp, p + 1, end - p);
                    file_info.file_size_ = atoi(tmp);
                    file_lst.insert(file_lst.begin(), file_info);
                }
            }
            --p;
            --len;
        }
    }

    bool Phone::get_dir_file(const std::string& dir,
        int start_page,
        int numbers,
        eSortType sort_type,
        eSortField sort_field,
        std::vector<FileInfo>& file_lst)
    {
        char page[50] = { 0 };
        sprintf(page, "|tail -n +%d|head -n %d", (start_page-1)*numbers + 2, numbers);

        std::string option;
        if (show_all_)
        {
            option = "-llAp";
        }
        else
        {
            option = "-llp";
        }

        if (eSortByTime == sort_field)
        {
            option.append("t");
        }
        else if (eSortBySize == sort_field)
        {
            option.append("S");
        }

        if (eSortDesc == sort_type)
        {
            option.append("r");
        }

        std::string script;
        script.resize(dir.size() + root_path_.size() + serial_no_.size() + strlen(page) + 70);
        int n = sprintf((char*)script.data(), "adb -s %s shell \"ls %s%s %s%s\"",
            serial_no_.c_str(), root_path_.c_str(), dir.c_str(), option.c_str(), page);
        script.resize(n);
        
        lock::Lock locl(&lock_);
        if (!adb_shell_->exe_adb_script(script))
        {
            return false;
        }

        const std::string& result = adb_shell_->get_result();
        parse_result(result, dir, file_lst);       

        return true;
    }

    bool Phone::get_all_dir_file(const std::string& dir, std::vector<FileInfo>& file_lst)
    {
        std::string script;
        script.resize(dir.size() + root_path_.size() + serial_no_.size() + 70);
        int n = sprintf((char*)script.data(), "adb -s %s shell \"ls %s%s -llAp\"",
            serial_no_.c_str(), root_path_.c_str(), dir.c_str());
        script.resize(n);

        lock::Lock locl(&lock_);
        if (!adb_shell_->exe_adb_script(script))
        {
            return false;
        }

        const std::string& result = adb_shell_->get_result();
        parse_result(result, dir, file_lst);

        return true;
    }

    int Phone::get_dir_file_count(const std::string& dir)
    {
        std::string option;
        if (show_all_)
        {
            option = "-lA|wc -l";
        }
        else
        {
            option = "-l|wc -l";
        }

        std::string script;
        script.resize(dir.size() + root_path_.size() + serial_no_.size() + 70);
        int n = sprintf((char*)script.data(), "adb -s %s shell \"ls %s%s %s\"",
            serial_no_.c_str(), root_path_.c_str(), dir.c_str(), option.c_str());
        script.resize(n);

        lock::Lock locl(&lock_);
        if (!adb_shell_->exe_adb_script(script))
        {
            return -1;
        }

        const std::string& result = adb_shell_->get_result();
        return get_file_numbers(result);
    }

    bool Phone::get_all_file(int file_type,
        int start_page,
        int numbers,
        eSortType sort_type,
        eSortField sort_field,
        std::vector<FileInfo>& file_lst)
    {
        //从默认的程序目录中取文件
//         std::string script;
//         script.resize(dir.size() + root_path_.size() + serial_no_.size() + 50);
//         script.clear();
//         if (show_all_)
//         {
//             sprintf((char*)script.data(), "-s %s shell ls %s%s  -lAp", serial_no_.c_str(), root_path_.c_str(), dir.c_str());
//         }
//         else
//         {
//             sprintf((char*)script.data(), "-s %s shell ls %s%s  -lp", serial_no_.c_str(), root_path_.c_str(), dir.c_str());
//         }
// 
//         if (!adb_shell_->exe_adb_script(script))
//         {
//             return false;
//         }
        return false;
    }

    int Phone::get_all_file_count(int file_type)
    {
        return 0;
    }

    void Phone::set_camera_img_path(const std::string& path)
    {
        camera_img_path_ = path;
    }

    void Phone::set_show_all(bool show_all)
    {
        show_all_ = show_all;
    }

    void Phone::set_root_path(const std::string& path)
    {
        root_path_ = path;
    }

    unsigned Phone::get_file_numbers(const std::string& result)
    {
        int n = atoi(result.c_str());
        --n;
        return n;
    }

    bool Phone::get_file(const std::string& file_path, const std::string& storage_path)
    {
        std::string script;
        script.resize(file_path.size() + root_path_.size() + serial_no_.size() + storage_path.size()+ 70);
        int n = sprintf((char*)script.data(), "adb -s %s pull -a %s%s %s",
            serial_no_.c_str(), root_path_.c_str(), file_path.c_str(), storage_path.c_str());
        script.resize(n);

        lock::Lock locl(&lock_);
        return adb_shell_->exe_adb_script(script);
    }

    std::string Phone::get_file_modify_time(const std::string& path)
    {
        std::string time_str;
        std::string parent;
        std::string file_name = path;
        int pos = path.find('/');
        if (pos != std::string::npos)
        {
            parent = path.substr(0, pos);
            file_name = path.substr(pos + 1);
        }

        std::string script;
        script.resize(parent.size() + file_name .size() + root_path_.size() + serial_no_.size() + 70);
        int n = sprintf((char*)script.data(), "adb -s %s shell \"ls %s%s -ll | grep ' %s$'\"",
            serial_no_.c_str(), root_path_.c_str(), parent.c_str(), file_name.c_str());
        script.resize(n);

        lock::Lock locl(&lock_);
        if (!adb_shell_->exe_adb_script(script))
        {
            return std::move(time_str);
        }

        const std::string& result = adb_shell_->get_result();
        pos = result.rfind(file_name);
        pos -= 36;
        if (pos > 0)
        {
            time_str = result.substr(pos, 19);
        }

        return std::move(time_str);
    }

    bool Phone::get_pic_list(const std::string& file_path, std::vector<std::pair<std::string, std::string>>& pic_list)
    {
        std::string script;
        script.resize(file_path.size() + root_path_.size() + serial_no_.size() + 70);
        int n = sprintf((char*)script.data(), "adb -s %s shell \"ls %s%s -llp\"",
            serial_no_.c_str(), root_path_.c_str(), file_path.c_str());
        script.resize(n);

        lock::Lock locl(&lock_);
        if (!adb_shell_->exe_adb_script(script))
        {
            return false;
        }

        const std::string& result = adb_shell_->get_result();
        parse_image_list_result(result, file_path, pic_list);
        return true;
    }

    void Phone::parse_image_list_result(const std::string& result, const std::string& dir, std::vector<std::pair<std::string, std::string>>& pic_list)
    {
        if (std::string::npos != result.find("total 0"))
        {
            return;
        }

        char tmp[256] = { 0 };

        const char* p = result.c_str();
        p = p + result.size() - 1;
        const char* end = p;

        int len = result.size();
        int sp_num = 0;  //分隔符个数
        while (len > 0)
        {
            if ('\n' == *p)
            {
                --p;
                --len;

                if ('\r' == *p)
                {
                    --p;
                    --len;
                }
                sp_num = 0;
                end = p;
                continue;
            }
            else if (' ' == *p)
            {
                ++sp_num;
                if (1 == sp_num && '/' != *end)
                {
                    memset(tmp, 0, 256);
                    memcpy(tmp, p + 1, end - p);
                    std::string file_name = character::utf8_to_mbyte(tmp);
                    if (eFilePic == get_file_type(file_name))
                    {
                        std::pair<std::string, std::string> pic_info;
                        pic_info.first = file_name;
                        pic_info.second = dir;
                        pic_list.push_back(pic_info);
                    }
                }
            }
            --p;
            --len;
        }
    }

    bool Phone::delete_file(const std::string& file)
    {
        std::string script;
        script.resize(file.size() + root_path_.size() + serial_no_.size() + 70);
        int n = sprintf((char*)script.data(), "adb -s %s shell \"rm -r %s%s\"",
            serial_no_.c_str(), root_path_.c_str(), file.c_str());
        script.resize(n);

        lock::Lock locl(&lock_);
        return adb_shell_->exe_adb_script(script);
    }

    bool Phone::rename_file(const std::string& name, const std::string& new_name)
    {
        std::string script;
        script.resize(name.size() + new_name.size() + root_path_.size()*2 + serial_no_.size() + 70);
        int n = sprintf((char*)script.data(), "adb -s %s shell \"rename %s%s  %s%s\"",
            serial_no_.c_str(), root_path_.c_str(), name.c_str(), root_path_.c_str(), new_name.c_str());
        script.resize(n);

        lock::Lock locl(&lock_);
        return adb_shell_->exe_adb_script(script);
    }
}


