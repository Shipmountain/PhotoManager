#ifndef DB_H
#define DB_H
#include <string>
#include <log.h>
#include <sqlite3.h>
#include "../data/Common.h"
#include <lock.h>

namespace db
{
    class DB :public Log::ModuleLog
    {
    public:
        DB();
        ~DB();
    public:
        bool init();
        void uninit();

        void save_local_dir(const std::string& path);

        void load_local_dir(std::string& path);

        bool get_all_file(int file_type,
            const std::string& phone_sn,
            int start_page,
            int numbers,
            eSortType sort_type,
            eSortField sort_field,
            std::vector<FileInfo>& file_lst);

        int get_all_file_count(int file_type, const std::string& phone_sn);

        bool get_dir_file(const std::string& dir_id,
            const std::string& phone_sn,
            int start_page,
            int numbers,
            eSortType sort_type,
            eSortField sort_field,
            std::vector<FileInfo>& file_lst);

        int get_dir_file_count(const std::string& dir_id, const std::string& phone_sn);

        bool is_file_exist(const std::string& file_id);

        bool add_file(const std::string& file_id,
            const std::string& file_name,
            const std::string& file_path,
            const std::string& modify_time,
            int         file_size,
            eFileType   file_type,
            const std::string& serial_no);

        bool del_file(const std::string& file_id);

        bool update_file(const std::string& file_id, const std::string& modify_time, int file_size);

        bool get_file_info(const std::string& file_id, FileInfo& file_info);

        bool update_file_thumbnail(const std::string& file_id, const std::string& thumbnail_path, int width, int height);

        bool add_phone_sync_file(const std::string& phone_sn,
            const std::string& file_id,
            const std::string& file_path,
            const std::string& modify_time,
            bool is_root,
            bool is_dir,
            const std::string& root_id);

        bool get_phone_sync_file_info(const std::string& phone_sn,
            const std::string& file_id,
            std::string& file_path,
            std::string& modify_time,
            bool& is_root,
            bool& is_dir,
            std::string& root_id);

        bool del_phone_sync_file(const std::string& phone_sn, const std::string& file_id);

        void get_phone_sync_lst(const std::string& phone_sn, std::vector<SyncInfo>& sync_lst);

        bool is_sync_file_exist(const std::string& phone_sn, const std::string& file_id);

        void update_sync_file_time(const std::string& phone_sn, const std::string& file_id, const std::string& modify_time);
        
        std::string get_sync_file_time(const std::string& phone_sn, const std::string& file_id);

        bool get_pic_list(const std::string& dir_id, std::vector<std::pair<std::string, std::string>>& pic_list);

        void update_sync_file_id(const std::string& phone_sn,
            const std::string& old_id,
            const std::string& new_id,
            const std::string& file_path,
            const std::string& modify_time,
            const std::string& root_id);

        void update_file_id(const std::string& old_id,
            const std::string& new_id,
            const std::string& file_name,
            const std::string& file_path,
            const std::string& dir_id,
            const std::string& serial_no,
            const std::string& thumbnail_path);

        void get_file_list(const std::string& dir_id, std::vector<FileInfo>& file_lst);
    private:

        bool create_table();
        bool create_index();
        bool create_procedure();
        //数据是否存在
        bool is_exist(const std::string& sql);

        //执行SQL语句
        bool exe_sql(const std::string& sql);

        //保存配置项
        void save_conf_item(const std::string& serial_no, const std::string& conf_name, const std::string& conf_value);

        void get_db_file_info(sqlite3_stmt* stmt_ptr, FileInfo& file_info);

        //获取排序字符串
        std::string get_sort_string(eSortType sort_type, eSortField sort_field);
    private:
        sqlite3*                    sqlite_db_;             //数据库的指针
        char*                       sqlite_errmsg_;         //数据库错误消息
        sqlite3_stmt*               update_thumbnail_stmt_;
        sqlite3_stmt*               file_info_stmt_;
        sqlite3_stmt*               update_file_stmt_;
        sqlite3_stmt*               add_file_stmt_;
        sqlite3_stmt*               del_file_stmt_;
        sqlite3_stmt*               file_exist_stmt_;
        sqlite3_stmt*               add_sync_dir_stmt_;
        sqlite3_stmt*               del_sync_file_stmt_;
        sqlite3_stmt*               get_sync_lst_stmt_;
        sqlite3_stmt*               is_sync_exist_stmt_;
        sqlite3_stmt*               update_sync_info_stmt_;
        sqlite3_stmt*               get_sync_file_time_stmt_;
        sqlite3_stmt*               get_pic_list_stmt_;
        sqlite3_stmt*               get_sync_file_info_stmt_;
        sqlite3_stmt*               update_sync_file_id_stmt_;
        sqlite3_stmt*               update_file_id_stmt_;
        sqlite3_stmt*               get_file_list_stmt_;

        lock::Mutex                 lock_;
    };
}

#endif  //DB_H

