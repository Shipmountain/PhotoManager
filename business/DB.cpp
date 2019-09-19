#include "DB.h"
#include <system.h>
#include <character_cvt.hpp>

namespace db
{

    DB::DB() :ModuleLog("DB")
        , sqlite_db_(NULL)
        , sqlite_errmsg_(NULL)
        , update_thumbnail_stmt_(NULL)
        , file_info_stmt_(NULL)
        , update_file_stmt_(NULL)
        , del_file_stmt_(NULL)
        , add_file_stmt_(NULL)
        , file_exist_stmt_(NULL)
        , add_sync_dir_stmt_(NULL)
        , del_sync_file_stmt_(NULL)
        , get_sync_lst_stmt_(NULL)
        , is_sync_exist_stmt_(NULL)
        , update_sync_info_stmt_(NULL)
        , get_sync_file_time_stmt_(NULL)
        , get_pic_list_stmt_(NULL)
        , get_sync_file_info_stmt_(NULL)
        , update_sync_file_id_stmt_(NULL)
        , update_file_id_stmt_(NULL)
        , get_file_list_stmt_(NULL)
    {

    }

    DB::~DB()
    {
        uninit();
    }

    bool DB::init()
    {
        std::string db_file;
        if (!sys::get_module_path(db_file))
        {
            return false;
        }

        db_file.append(PROGRAM_CONF_FILE);

        if (SQLITE_OK != sqlite3_open(db_file.c_str(), &sqlite_db_))
        {
            TraceLog(LogError, "open database:%s fail", db_file.c_str());
            return false;
        }

        if (!create_table())
        {
            return false;
        }

        if (!create_index())
        {
            return false;
        }

        if (!create_procedure())
        {
            return false;
        }

        if (!exe_sql("PRAGMA synchronous = OFF;"))
        {
            return false;
        }        

        return true;
    }

    void DB::uninit()
    {
        sqlite3_finalize(update_thumbnail_stmt_);
        update_thumbnail_stmt_ = NULL;

        sqlite3_finalize(file_info_stmt_);
        file_info_stmt_ = NULL;

        sqlite3_finalize(update_file_stmt_);
        update_file_stmt_ = NULL;

        sqlite3_finalize(add_file_stmt_);
        add_file_stmt_ = NULL;

        sqlite3_finalize(del_file_stmt_);
        del_file_stmt_ = NULL;

        sqlite3_finalize(file_exist_stmt_);
        file_exist_stmt_ = NULL;

        sqlite3_finalize(add_sync_dir_stmt_);
        add_sync_dir_stmt_ = NULL;

        sqlite3_finalize(del_sync_file_stmt_);
        del_sync_file_stmt_ = NULL;
        
        sqlite3_finalize(get_sync_lst_stmt_);
        get_sync_lst_stmt_ = NULL;

        sqlite3_finalize(is_sync_exist_stmt_);
        is_sync_exist_stmt_ = NULL;

        sqlite3_finalize(update_sync_info_stmt_);
        update_sync_info_stmt_ = NULL;

        sqlite3_finalize(get_sync_file_time_stmt_);
        get_sync_file_time_stmt_ = NULL;

        sqlite3_finalize(get_pic_list_stmt_);
        get_pic_list_stmt_ = NULL;

        sqlite3_finalize(get_sync_file_info_stmt_);
        get_sync_file_info_stmt_ = NULL;

        sqlite3_finalize(update_sync_file_id_stmt_);
        update_sync_file_id_stmt_ = NULL;

        sqlite3_finalize(update_file_id_stmt_);
        update_file_id_stmt_ = NULL;

        sqlite3_finalize(get_file_list_stmt_);
        get_file_list_stmt_ = NULL;

        sqlite3_close(sqlite_db_);
        sqlite_db_ = NULL;
    }

    bool DB::create_table()
    {
        //创建配置表
        if (!exe_sql("CREATE TABLE IF NOT EXISTS ConfInfo(serial_no VARCHAR(50), conf_item VARCHAR(255),conf_value TEXT)"))
        {
            return false;
        }

        //创建同步表
        if (!exe_sql("CREATE TABLE IF NOT EXISTS SyncInfo(serial_no VARCHAR(50), file_id VARCHAR(32), file_path VARCHAR(255), modify_time VARCHAR(20), is_root INT, is_dir INT, root_id VARCHAR(32))"))
        {
            return false;
        }

        //创建目录表
        if (!exe_sql("CREATE TABLE IF NOT EXISTS DirInfo(file_id VARCHAR(32) PRIMARY KEY, file_name VARCHAR(255), file_path VARCHAR(255), file_size INT, modify_time VARCHAR(20), file_type INT, dir_id VARCHAR(32), modify_date INT, serial_no VARCHAR(50), thumbnail_path VARCHAR(255), thumbnail_width INT, thumbnail_height INT)"))
        {
            return false;
        }
        return true;
    }

    bool DB::create_index()
    {
        //创建索引
        if (!is_exist("SELECT * FROM sqlite_master WHERE type='index' AND name='file_type_index_DirInfo'"))
        {
            if (!exe_sql("CREATE INDEX file_type_index_DirInfo ON DirInfo (file_type)"))
            {
                return false;
            }
        }

        if (!is_exist("SELECT * FROM sqlite_master WHERE type='index' AND name='dir_id_index_DirInfo'"))
        {
            if (!exe_sql("CREATE INDEX dir_id_index_DirInfo ON DirInfo (dir_id)"))
            {
                return false;
            }
        }

        if (!is_exist("SELECT * FROM sqlite_master WHERE type='index' AND name='modify_date_index_DirInfo'"))
        {
            if (!exe_sql("CREATE INDEX modify_date_index_DirInfo ON DirInfo (modify_date)"))
            {
                return false;
            }
        }

        if (!is_exist("SELECT * FROM sqlite_master WHERE type='index' AND name='serial_no_index_DirInfo'"))
        {
            if (!exe_sql("CREATE INDEX serial_no_index_DirInfo ON DirInfo (serial_no)"))
            {
                return false;
            }
        }

        return true;
    }

    bool DB::create_procedure()
    {
        std::string sql = "UPDATE DirInfo SET thumbnail_path=?, thumbnail_width=?, thumbnail_height=? WHERE file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &update_thumbnail_stmt_, 0))
        {
            return false;
        }

        sql = "SELECT file_name,file_path,modify_time,file_size,file_type,serial_no,thumbnail_path,thumbnail_width,thumbnail_height FROM DirInfo WHERE file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &file_info_stmt_, 0))
        {
            return false;
        }

        sql = "UPDATE DirInfo SET modify_time=?, modify_date=?, file_size=? WHERE file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &update_file_stmt_, 0))
        {
            return false;
        }

        sql = "INSERT INTO DirInfo(file_id,file_name,file_path,file_size, modify_time,file_type,dir_id,modify_date,serial_no) VALUES(?,?,?,?,?,?,?,?,?) ";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &add_file_stmt_, 0))
        {
            return false;
        }

        sql = "DELETE FROM DirInfo WHERE file_id=? OR dir_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &del_file_stmt_, 0))
        {
            return false;
        }

        sql = "SELECT file_id FROM DirInfo WHERE file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &file_exist_stmt_, 0))
        {
            return false;
        }

        sql = "INSERT INTO SyncInfo(serial_no, file_id, file_path, modify_time, is_root, is_dir, root_id) VALUES(?,?,?,?,?,?,?)";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &add_sync_dir_stmt_, 0))
        {
            return false;
        }

        sql = "DELETE FROM SyncInfo WHERE serial_no=? AND (file_id=? OR root_id=?)";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &del_sync_file_stmt_, 0))
        {
            return false;
        }

        sql = "SELECT file_path, modify_time, is_dir FROM SyncInfo WHERE serial_no=? AND is_root=1";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &get_sync_lst_stmt_, 0))
        {
            return false;
        }

        sql = "SELECT serial_no FROM SyncInfo WHERE serial_no=? AND file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &is_sync_exist_stmt_, 0))
        {
            return false;
        }

        sql = "UPDATE SyncInfo SET modify_time=? WHERE serial_no=? AND file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &update_sync_info_stmt_, 0))
        {
            return false;
        }

        sql = "SELECT modify_time FROM SyncInfo WHERE serial_no=? AND file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &get_sync_file_time_stmt_, 0))
        {
            return false;
        }

        sql = "SELECT file_name, file_path FROM DirInfo WHERE dir_id=? AND file_type=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &get_pic_list_stmt_, 0))
        {
            return false;
        }
        
        sql = "SELECT file_path, modify_time, is_root, is_dir, root_id FROM SyncInfo WHERE serial_no=? AND file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &get_sync_file_info_stmt_, 0))
        {
            return false;
        }        

        sql = "UPDATE SyncInfo SET file_id=?, file_path=?, modify_time=?, root_id=? WHERE serial_no=? AND file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &update_sync_file_id_stmt_, 0))
        {
            return false;
        }
        
        sql = "UPDATE DirInfo SET file_id=?, file_name=?, file_path=?, dir_id=?, serial_no=?, thumbnail_path=? WHERE file_id=?";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &update_file_id_stmt_, 0))
        {
            return false;
        }

        sql = "SELECT file_name,file_path,modify_time,file_size,file_type,serial_no,thumbnail_path,thumbnail_width,thumbnail_height FROM DirInfo WHERE dir_id=? ";
        if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &get_file_list_stmt_, 0))
        {
            return false;
        }

        return true;
    }


    void DB::load_local_dir(std::string& path)
    {
        lock::Lock locl(&lock_);

        char** result_ptr = NULL;
        int row = 0;
        int column = 0;
        std::string sql = "select conf_value from ConfInfo where serial_no='local_path' and conf_item='local_path'";
        if (SQLITE_OK != sqlite3_get_table(sqlite_db_, sql.c_str(), &result_ptr, &row, &column, &sqlite_errmsg_))
        {
            TraceLog(LogError, "load_local_dir fail:%s", sqlite_errmsg_);
            return;
        }

        if (1 == row && NULL != result_ptr)
        {
            path = get_string_field(result_ptr + column, 0);
            TraceLog(LogInfo, "local store path:%s", path.c_str());
        }

        if (NULL != result_ptr)
        {
            sqlite3_free_table(result_ptr);
        }
    }

    void DB::save_local_dir(const std::string& path)
    {
        save_conf_item("local_path", "local_path", path);
    }
    
    void DB::save_conf_item(const std::string& serial_no, const std::string& conf_name, const std::string& conf_value)
    {
        if (NULL == sqlite_db_)
        {
            return;
        }

        lock::Lock locl(&lock_);

        char** result_ptr = NULL;
        int row = 0;
        int column = 0;
        std::string sql = "select conf_value from ConfInfo where serial_no='";
        sql.append(serial_no);
        sql.append("' and conf_item ='");
        sql.append(conf_name);
        sql.append("'");

        if (SQLITE_OK != sqlite3_get_table(sqlite_db_, sql.c_str(), &result_ptr, &row, &column, &sqlite_errmsg_))
        {
            TraceLog(LogError, "Determine whether a configuration item exists fail:%s", sqlite_errmsg_);
            return;
        }

        if (1 == row && NULL != result_ptr)
        {
            sql = "update ConfInfo set conf_value='";
            sql.append(conf_value);
            sql.append("' where serial_no='");
            sql.append(serial_no);
            sql.append("' and conf_item='");
            sql.append(conf_name);
            sql.append("'");
        }
        else
        {
            sql = "insert into ConfInfo(serial_no, conf_item, conf_value) values('";
            sql.append(serial_no);
            sql.append("','");
            sql.append(conf_name);
            sql.append("','");
            sql.append(conf_value);
            sql.append("')");
        }

        if (NULL != result_ptr)
        {
            sqlite3_free_table(result_ptr);
        }

        exe_sql(sql);
    }

    bool DB::is_exist(const std::string& sql)
    {
        if (NULL == sqlite_db_)
        {
            return false;
        }

        lock::Lock locl(&lock_);

        char** result_ptr = NULL;
        int row = 0;
        int column = 0;

        if (SQLITE_OK != sqlite3_get_table(sqlite_db_, sql.c_str(), &result_ptr, &row, &column, &sqlite_errmsg_))
        {
            TraceLog(LogError, "Determine whether a record exists fail:%s", sqlite_errmsg_);
            return false;
        }

        if (NULL != result_ptr)
        {
            sqlite3_free_table(result_ptr);
        }

        if (1 == row)
        {
            return true;
        }

        return false;
    }

    bool DB::exe_sql(const std::string& sql)
    {
        if (SQLITE_OK != sqlite3_exec(sqlite_db_, sql.c_str(), NULL, 0, &sqlite_errmsg_))
        {
            TraceLog(LogError, "exe sql:%s fail:%s", sql.c_str(), sqlite_errmsg_);
            return false;
        }

        return true;
    }

    void DB::get_db_file_info(sqlite3_stmt* stmt_ptr, FileInfo& file_info)
    {
        const unsigned char* tmp = sqlite3_column_text(stmt_ptr, 0);
        if (NULL != tmp)
        {
            file_info.file_name_ = character::mbyte_to_unicode((char*)tmp);
        }

        tmp = sqlite3_column_text(stmt_ptr, 1);
        if (NULL != tmp)
        {
            file_info.file_path_ = character::mbyte_to_unicode((char*)tmp);
        }

        tmp = sqlite3_column_text(stmt_ptr, 2);
        if (NULL != tmp)
        {
            file_info.modify_time_ = character::mbyte_to_unicode((char*)tmp);
        }

        file_info.file_size_ = sqlite3_column_int(stmt_ptr, 3);
        file_info.file_type_ = (eFileType)sqlite3_column_int(stmt_ptr, 4);

        tmp = sqlite3_column_text(stmt_ptr, 5);
        if (NULL != tmp)
        {
            file_info.serial_no_ = character::mbyte_to_unicode((char*)tmp);
        }

        tmp = sqlite3_column_text(stmt_ptr, 6);
        if (NULL != tmp)
        {
            file_info.thumbnail_path_ = character::mbyte_to_unicode((char*)tmp);
        }

        file_info.thumbnail_width_ = sqlite3_column_int(stmt_ptr, 7);
        file_info.thumbnail_height_ = sqlite3_column_int(stmt_ptr, 8);
    }

    bool DB::get_all_file(int file_type,
        const std::string& phone_sn,
        int start_page,
        int numbers,
        eSortType sort_type,
        eSortField sort_field,
        std::vector<FileInfo>& file_lst)
    {
        lock::Lock locl(&lock_);

        std::string sort_str = get_sort_string(sort_type, sort_field);
        std::string sql = "select file_name,file_path,modify_time,file_size,file_type,serial_no,thumbnail_path,thumbnail_width,thumbnail_height from DirInfo ";

        --start_page;
        sqlite3_stmt* stmt_ptr = NULL;

        if (eFilePic == file_type ||
            eFileVideo == file_type ||
            eFileMusic == file_type ||
            eFileDoc == file_type ||
            eFileOther == file_type ||
            eFileDir == file_type)
        {
            sql.append(" where file_type=? ");
            if (!phone_sn.empty())
            {
                sql.append(" and serial_no=? ");
                sql.append(sort_str);
                sql.append(" limit ? offset ?");
                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return false;
                }
                sqlite3_bind_int(stmt_ptr, 1, file_type);
                sqlite3_bind_text(stmt_ptr, 2, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
                sqlite3_bind_int(stmt_ptr, 3, numbers);
                sqlite3_bind_int(stmt_ptr, 4, start_page*numbers);
            }
            else
            {
                sql.append(sort_str);
                sql.append(" limit ? offset ?");
                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return false;
                }
                sqlite3_bind_int(stmt_ptr, 1, file_type);
                sqlite3_bind_int(stmt_ptr, 2, numbers);
                sqlite3_bind_int(stmt_ptr, 3, start_page*numbers);
            }
        }
        else if (!phone_sn.empty())
        {
            sql.append(" where serial_no=?");
            sql.append(sort_str);
            sql.append(" limit ? offset ?");
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return false;
            }
            sqlite3_bind_text(stmt_ptr, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
            sqlite3_bind_int(stmt_ptr, 2, numbers);
            sqlite3_bind_int(stmt_ptr, 3, start_page*numbers);
        }
        else
        {
            sql.append(sort_str);
            sql.append(" limit ? offset ?");
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return false;
            }
            sqlite3_bind_int(stmt_ptr, 1, numbers);
            sqlite3_bind_int(stmt_ptr, 2, start_page*numbers);
        }

        while (sqlite3_step(stmt_ptr) == SQLITE_ROW)
        {
            FileInfo file_info;
            get_db_file_info(stmt_ptr, file_info);
            file_lst.push_back(file_info);
        }

        sqlite3_finalize(stmt_ptr);
        return true;
    }

    int DB::get_all_file_count(int file_type, const std::string& phone_sn)
    {
        lock::Lock locl(&lock_);

        std::string sql = "select count(file_name) from DirInfo ";
        sqlite3_stmt* stmt_ptr = NULL;

        if (eFilePic == file_type ||
            eFileVideo == file_type ||
            eFileMusic == file_type ||
            eFileDoc == file_type ||
            eFileOther == file_type ||
            eFileDir == file_type)
        {
            sql.append(" where file_type=? ");
            if (!phone_sn.empty())
            {
                sql.append(" and serial_no=? ");
                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return -1;
                }
                sqlite3_bind_int(stmt_ptr, 1, file_type);
                sqlite3_bind_text(stmt_ptr, 2, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
            }
            else
            {
                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return -1;
                }
                sqlite3_bind_int(stmt_ptr, 1, file_type);
            }
        }
        else if (!phone_sn.empty())
        {
            sql.append(" where serial_no=?");
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return -1;
            }
            sqlite3_bind_text(stmt_ptr, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        }
        else
        {
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return -1;
            }
        }

        int ret = -1;
        if (sqlite3_step(stmt_ptr) == SQLITE_ROW)
        {
            ret = sqlite3_column_int(stmt_ptr, 0);
        }

        sqlite3_finalize(stmt_ptr);
        return ret;
    }

    bool DB::get_dir_file(const std::string& dir_id,
        const std::string& phone_sn,
        int start_page,
        int numbers,
        eSortType sort_type,
        eSortField sort_field,
        std::vector<FileInfo>& file_lst)
    {
        lock::Lock locl(&lock_);

        std::string sort_str = get_sort_string(sort_type, sort_field);

        --start_page;
        sqlite3_stmt* stmt_ptr = NULL;

        std::string sql = "select file_name,file_path,modify_time,file_size,file_type,serial_no,thumbnail_path,thumbnail_width,thumbnail_height from DirInfo ";
        if (!dir_id.empty())
        {
            sql.append(" where dir_id=? ");
            if (!phone_sn.empty())
            {
                sql.append(" and serial_no=? ");
                sql.append(sort_str);
                sql.append(" limit ? offset ?");

                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return false;
                }

                sqlite3_bind_text(stmt_ptr, 1, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);
                sqlite3_bind_text(stmt_ptr, 2, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
                sqlite3_bind_int(stmt_ptr, 3, numbers);
                sqlite3_bind_int(stmt_ptr, 4, start_page*numbers);
            }
            else
            {
                sql.append(sort_str);
                sql.append(" limit ? offset ?");
                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return false;
                }
                sqlite3_bind_text(stmt_ptr, 1, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);
                sqlite3_bind_int(stmt_ptr, 2, numbers);
                sqlite3_bind_int(stmt_ptr, 3, start_page*numbers);
            }
        }
        else if (!phone_sn.empty())
        {
            sql.append(" where serial_no=?");
            sql.append(sort_str);
            sql.append(" limit ? offset ?");
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return false;
            }
            sqlite3_bind_text(stmt_ptr, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
            sqlite3_bind_int(stmt_ptr, 2, numbers);
            sqlite3_bind_int(stmt_ptr, 3, start_page*numbers);
        }
        else
        {
            sql.append(sort_str);
            sql.append(" limit ? offset ?");
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return false;
            }
            sqlite3_bind_int(stmt_ptr, 1, numbers);
            sqlite3_bind_int(stmt_ptr, 2, start_page*numbers);
        }


        while (sqlite3_step(stmt_ptr) == SQLITE_ROW)
        {
            FileInfo file_info;
            get_db_file_info(stmt_ptr, file_info);
            file_lst.push_back(file_info);
        }

        sqlite3_finalize(stmt_ptr);
        return true;
    }

    int DB::get_dir_file_count(const std::string& dir_id, const std::string& phone_sn)
    {
        lock::Lock locl(&lock_);

        std::string sql = "select count(file_name) from DirInfo ";
        sqlite3_stmt* stmt_ptr = NULL;
        if (!dir_id.empty())
        {
            sql.append(" where dir_id=? ");
            if (!phone_sn.empty())
            {
                sql.append(" and serial_no=? ");
                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return -1;
                }

                sqlite3_bind_text(stmt_ptr, 1, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);
                sqlite3_bind_text(stmt_ptr, 2, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
            }
            else
            {
                if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
                {
                    TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                    return -1;
                }
                sqlite3_bind_text(stmt_ptr, 1, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);
            }
        }
        else if (!phone_sn.empty())
        {
            sql.append(" where serial_no=?");
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return -1;
            }
            sqlite3_bind_text(stmt_ptr, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        }
        else
        {
            if (SQLITE_OK != sqlite3_prepare_v2(sqlite_db_, sql.c_str(), sql.size(), &stmt_ptr, 0))
            {
                TraceLog(LogError, "sqlite3_prepare_v2 fail:%s, sql:%s", sqlite_errmsg_, sql.c_str());
                return -1;
            }
        }

        int ret = -1;
        if (sqlite3_step(stmt_ptr) == SQLITE_ROW)
        {
            ret = sqlite3_column_int(stmt_ptr, 0);
        }

        sqlite3_finalize(stmt_ptr);
        return ret;
    }

    std::string DB::get_sort_string(eSortType sort_type, eSortField sort_field)
    {
        std::string sort_str;
        sort_str.resize(25, '\0');
        sort_str.clear();
        if (eSortByTime == sort_field)
        {
            sort_str = "ORDER BY modify_time";
        }
        else if (eSortBySize == sort_field)
        {
            sort_str = "ORDER BY file_size";
        }
        else
        {
            sort_str = "ORDER BY file_name";
        }

        if (eSortDesc == sort_type)
        {
            sort_str.append(" DESC");
        }

        return std::move(sort_str);
    }

    bool DB::is_file_exist(const std::string& file_id)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(file_exist_stmt_);
        sqlite3_bind_text(file_exist_stmt_, 1, file_id.c_str(), file_id.size(), SQLITE_STATIC);

        if (sqlite3_step(file_exist_stmt_) == SQLITE_ROW)
        {
            return true;
        }

        return false;
    }

    bool DB::add_file(const std::string& file_id,
        const std::string& file_name,
        const std::string& file_path,
        const std::string& modify_time,
        int         file_size,
        eFileType   file_type,
        const std::string& serial_no)
    {
        lock::Lock locl(&lock_);

        std::string dir_id = get_str_md5(file_path);
        std::string modify_date = modify_time.substr(0, 10);
        sqlite3_reset(add_file_stmt_);
        sqlite3_bind_text(add_file_stmt_, 1, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(add_file_stmt_, 2, file_name.c_str(), file_name.size(), SQLITE_STATIC);
        sqlite3_bind_text(add_file_stmt_, 3, file_path.c_str(), file_path.size(), SQLITE_STATIC);
        sqlite3_bind_int(add_file_stmt_, 4, file_size);
        sqlite3_bind_text(add_file_stmt_, 5, modify_time.c_str(), modify_time.size(), SQLITE_STATIC);
        sqlite3_bind_int(add_file_stmt_, 6, file_type);
        sqlite3_bind_text(add_file_stmt_, 7, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(add_file_stmt_, 8, modify_date.c_str(), modify_date.size(), SQLITE_STATIC);
        sqlite3_bind_text(add_file_stmt_, 9, serial_no.c_str(), serial_no.size(), SQLITE_STATIC);
        sqlite3_step(add_file_stmt_);
        return true;
    }

    bool DB::del_file(const std::string& file_id)
    {
        lock::Lock locl(&lock_);
        sqlite3_reset(del_file_stmt_);
        sqlite3_bind_text(del_file_stmt_, 1, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(del_file_stmt_, 2, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_step(del_file_stmt_);
        return true;
    }

    bool DB::update_file(const std::string& file_id, const std::string& modify_time, int file_size)
    {
        lock::Lock locl(&lock_);

        std::string modify_date = modify_time.substr(0, 10);
        sqlite3_reset(update_file_stmt_);
        sqlite3_bind_text(update_file_stmt_, 1, modify_time.c_str(), modify_time.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_file_stmt_, 2, modify_date.c_str(), modify_date.size(), SQLITE_STATIC);
        sqlite3_bind_int(update_file_stmt_, 3, file_size);
        sqlite3_bind_text(update_file_stmt_, 4, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_step(update_file_stmt_);
        return true;
    }

    bool DB::get_file_info(const std::string& file_id, FileInfo& file_info)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(file_info_stmt_);
        sqlite3_bind_text(file_info_stmt_, 1, file_id.c_str(), file_id.size(), SQLITE_STATIC);

        if (sqlite3_step(file_info_stmt_) == SQLITE_ROW)
        {
            get_db_file_info(file_info_stmt_, file_info);
            return true;
        }

        return false;
    }

    bool DB::update_file_thumbnail(const std::string& file_id, const std::string& thumbnail_path, int width, int height)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(update_thumbnail_stmt_);
        sqlite3_bind_text(update_thumbnail_stmt_, 1, thumbnail_path.c_str(), thumbnail_path.size(), SQLITE_STATIC);
        sqlite3_bind_int(update_thumbnail_stmt_, 2, width);
        sqlite3_bind_int(update_thumbnail_stmt_, 3, height);
        sqlite3_bind_text(update_thumbnail_stmt_, 4, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_step(update_thumbnail_stmt_);
        return true;
    }

    bool DB::add_phone_sync_file(const std::string& phone_sn,
        const std::string& file_id,
        const std::string& file_path,
        const std::string& modify_time,
        bool is_root,
        bool is_dir,
        const std::string& root_id)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(add_sync_dir_stmt_);
        sqlite3_bind_text(add_sync_dir_stmt_, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        sqlite3_bind_text(add_sync_dir_stmt_, 2, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(add_sync_dir_stmt_, 3, file_path.c_str(), file_path.size(), SQLITE_STATIC);
        sqlite3_bind_text(add_sync_dir_stmt_, 4, modify_time.c_str(), modify_time.size(), SQLITE_STATIC);
        sqlite3_bind_int(add_sync_dir_stmt_, 5, is_root);
        sqlite3_bind_int(add_sync_dir_stmt_, 6, is_dir);
        sqlite3_bind_text(add_sync_dir_stmt_, 7, root_id.c_str(), root_id.size(), SQLITE_STATIC);
        sqlite3_step(add_sync_dir_stmt_);
        return true;
    }

    bool DB::del_phone_sync_file(const std::string& phone_sn, const std::string& file_id)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(del_sync_file_stmt_);
        sqlite3_bind_text(del_sync_file_stmt_, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        sqlite3_bind_text(del_sync_file_stmt_, 2, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(del_sync_file_stmt_, 3, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_step(del_sync_file_stmt_);
        return true;
    }

    void DB::get_phone_sync_lst(const std::string& phone_sn, std::vector<SyncInfo>& sync_lst)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(get_sync_lst_stmt_);
        sqlite3_bind_text(get_sync_lst_stmt_, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);

        while (sqlite3_step(get_sync_lst_stmt_) == SQLITE_ROW)
        {
            SyncInfo sync_info;
            const unsigned char* tmp = sqlite3_column_text(get_sync_lst_stmt_, 0);
            if (NULL != tmp)
            {
                sync_info.file_path_ = (char*)tmp;
            }

            tmp = sqlite3_column_text(get_sync_lst_stmt_, 1);
            if (NULL != tmp)
            {
                sync_info.modify_time_ = (char*)tmp;
            }
            int is_dir = sqlite3_column_int(get_sync_lst_stmt_, 2);
            sync_info.is_dir_ = (1 == is_dir);

            sync_lst.push_back(sync_info);
        }
    }

    bool DB::is_sync_file_exist(const std::string& phone_sn, const std::string& file_id)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(is_sync_exist_stmt_);
        sqlite3_bind_text(is_sync_exist_stmt_, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        sqlite3_bind_text(is_sync_exist_stmt_, 2, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        if (sqlite3_step(is_sync_exist_stmt_) == SQLITE_ROW)
        {
            return true;
        }
        return false;
    }

    void DB::update_sync_file_time(const std::string& phone_sn, const std::string& file_id, const std::string& modify_time)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(update_sync_info_stmt_);
        sqlite3_bind_text(update_sync_info_stmt_, 1, modify_time.c_str(), modify_time.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_sync_info_stmt_, 2, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_sync_info_stmt_, 3, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        sqlite3_step(update_sync_info_stmt_);
    }

    std::string DB::get_sync_file_time(const std::string& phone_sn, const std::string& file_id)
    {
        lock::Lock locl(&lock_);

        std::string modify_time;
        sqlite3_reset(get_sync_file_time_stmt_);
        sqlite3_bind_text(get_sync_file_time_stmt_, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        sqlite3_bind_text(get_sync_file_time_stmt_, 2, file_id.c_str(), file_id.size(), SQLITE_STATIC);
        if (sqlite3_step(get_sync_file_time_stmt_) == SQLITE_ROW)
        {
            const unsigned char* tmp = sqlite3_column_text(get_sync_file_time_stmt_, 0);
            if (NULL != tmp)
            {
                modify_time = (char*)tmp;
            }
        }

        return std::move(modify_time);
    }

    bool DB::get_pic_list(const std::string& dir_id, std::vector<std::pair<std::string,std::string>>& pic_list)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(get_pic_list_stmt_);
        sqlite3_bind_text(get_pic_list_stmt_, 1, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);
        sqlite3_bind_int(get_pic_list_stmt_, 2, eFilePic);
        
        while (sqlite3_step(get_pic_list_stmt_) == SQLITE_ROW)
        {
            std::pair<std::string, std::string> file_info;
            const unsigned char* tmp = sqlite3_column_text(get_pic_list_stmt_, 0);
            if (NULL != tmp)
            {
                file_info.first = (char*)tmp;
            }

            tmp = sqlite3_column_text(get_pic_list_stmt_, 1);
            if (NULL != tmp)
            {
                file_info.second = (char*)tmp;
            }

            pic_list.push_back(file_info);
        }

        return true;
    }

    bool DB::get_phone_sync_file_info(const std::string& phone_sn,
        const std::string& file_id,
        std::string& file_path,
        std::string& modify_time,
        bool& is_root,
        bool& is_dir,
        std::string& root_id)
    {
        lock::Lock locl(&lock_);

        sqlite3_reset(get_sync_file_info_stmt_);
        sqlite3_bind_text(get_sync_file_info_stmt_, 1, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        sqlite3_bind_text(get_sync_file_info_stmt_, 2, file_id.c_str(), file_id.size(), SQLITE_STATIC);

        if (sqlite3_step(get_sync_file_info_stmt_) == SQLITE_ROW)
        {
            const unsigned char* tmp = sqlite3_column_text(get_sync_file_info_stmt_, 0);
            if (NULL != tmp)
            {
                file_path = (char*)tmp;
            }

            tmp = sqlite3_column_text(get_sync_file_info_stmt_, 1);
            if (NULL != tmp)
            {
                modify_time = (char*)tmp;
            }

            is_root = sqlite3_column_int(get_sync_file_info_stmt_, 2);
            is_root = sqlite3_column_int(get_sync_file_info_stmt_, 3);
            tmp = sqlite3_column_text(get_sync_file_info_stmt_, 4);
            if (NULL != tmp)
            {
                root_id = (char*)tmp;
            }

            return true;
        }
        return false;
    }

    void DB::update_sync_file_id(const std::string& phone_sn,
        const std::string& old_id,
        const std::string& new_id,
        const std::string& file_path,
        const std::string& modify_time,
        const std::string& root_id)
    {
        lock::Lock locl(&lock_);
        sqlite3_reset(update_sync_file_id_stmt_);
        sqlite3_bind_text(update_sync_file_id_stmt_, 1, new_id.c_str(), new_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_sync_file_id_stmt_, 2, file_path.c_str(), file_path.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_sync_file_id_stmt_, 3, modify_time.c_str(), modify_time.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_sync_file_id_stmt_, 4, root_id.c_str(), root_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_sync_file_id_stmt_, 5, phone_sn.c_str(), phone_sn.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_sync_file_id_stmt_, 6, old_id.c_str(), old_id.size(), SQLITE_STATIC);

        sqlite3_step(update_sync_file_id_stmt_);
    }

    void DB::update_file_id(const std::string& old_id,
        const std::string& new_id,
        const std::string& file_name,
        const std::string& file_path,
        const std::string& dir_id,
        const std::string& serial_no,
        const std::string& thumbnail_path)
    {
        lock::Lock locl(&lock_);
        sqlite3_reset(update_file_id_stmt_);
        sqlite3_bind_text(update_file_id_stmt_, 1, new_id.c_str(), new_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_file_id_stmt_, 2, file_name.c_str(), file_name.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_file_id_stmt_, 3, file_path.c_str(), file_path.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_file_id_stmt_, 4, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_file_id_stmt_, 5, serial_no.c_str(), serial_no.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_file_id_stmt_, 6, thumbnail_path.c_str(), thumbnail_path.size(), SQLITE_STATIC);
        sqlite3_bind_text(update_file_id_stmt_, 7, old_id.c_str(), old_id.size(), SQLITE_STATIC);

        sqlite3_step(update_file_id_stmt_);
    }

    void DB::get_file_list(const std::string& dir_id, std::vector<FileInfo>& file_lst)
    {
        lock::Lock locl(&lock_);
        sqlite3_reset(get_file_list_stmt_);
        sqlite3_bind_text(get_file_list_stmt_, 1, dir_id.c_str(), dir_id.size(), SQLITE_STATIC);

        while (sqlite3_step(get_file_list_stmt_) == SQLITE_ROW)
        {
            FileInfo file_info;
            get_db_file_info(get_file_list_stmt_, file_info);
            file_lst.push_back(file_info);
        }
    }

}


