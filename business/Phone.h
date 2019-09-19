#ifndef PHONE_H
#define PHONE_H

#include <string>
#include <log.h>
#include "AdbShell.h"
#include <vector>
#include "../data/Common.h"
#include <lock.h>

namespace phone
{
    class Phone :public Log::ModuleLog
    {
    public:
        Phone(const std::string& serial_no);

        ~Phone();

    public:
        bool init(unsigned def_result_size = 1024 * 1024 * 2);
        void uninit();

        bool get_dir_file(const std::string& dir, 
            int start_page,
            int numbers,
            eSortType sort_type,
            eSortField sort_field,
            std::vector<FileInfo>& file_lst);

        bool get_all_dir_file(const std::string& dir, std::vector<FileInfo>& file_lst);

        int get_dir_file_count(const std::string& dir);

        bool get_all_file(int file_type,
            int start_page,
            int numbers,
            eSortType sort_type,
            eSortField sort_field, 
            std::vector<FileInfo>& file_lst);

        int get_all_file_count(int file_type);

        void set_camera_img_path(const std::string& path);
        void set_show_all(bool show_all);
        void set_root_path(const std::string& path);

        bool get_file(const std::string& file_path, const std::string& storage_path);

        std::string get_file_modify_time(const std::string& path);

        bool get_pic_list(const std::string& file_path, std::vector<std::pair<std::string, std::string>>& pic_list);

        bool delete_file(const std::string& file);

        bool rename_file(const std::string& name, const std::string& new_name);
    private:
        unsigned get_file_numbers(const std::string& result);
        void parse_result(const std::string& result, const std::string& dir, std::vector<FileInfo>& file_lst);
        void parse_image_list_result(const std::string& result, const std::string& dir, std::vector<std::pair<std::string, std::string>>& pic_list);
    private:
        std::shared_ptr<adb::AdbShell> adb_shell_;
        lock::Mutex lock_;
    public:
        std::string serial_no_;
       
        //相机照片目录
        std::string camera_img_path_;

        //是否显示所有文件
        bool show_all_;

        //手机磁盘根目录
        std::string root_path_;

        bool        sync_local_to_phone_;   //同步本地数据到手机
        bool        sync_phone_to_local_;   //同步手机数据到本地
        bool        sync_phone_pic_;        //同步手机图片
        bool        sync_phone_video_;      //同步手机视频
    };

}

#endif  //PHONE_H

