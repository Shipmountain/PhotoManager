#include "BusinessInterface.h"
#include <functional>
#include "../data/DataFactory.h"
#include "../ui/ui_interface.h"
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <file.h>
#include <character_cvt.hpp>

namespace business
{
    CBusiness::CBusiness() :ModuleLog("Business")
        , unique_mutex_(NULL)
        , curr_phone_ptr_(nullptr)
        , db_ptr_(nullptr)
    {

    }

    CBusiness::~CBusiness()
    {

    }

    bool CBusiness::init()
	{
        //程序唯一性校验
        if (!program_unique_check())
        {
            return false;
        }

        dump::init();

        InitLog(LogDiskFile, 0, 10, 5, NULL, NULL);
        TraceLog( LogInfo, "********************PhotoManager Start********************");

        //数据初始化
        TraceLog( LogInfo, "init data module");
        CDataFactory& factory = theDataFactory::instance();
        if (!factory.init())
        {
            TraceLog( LogError, "init data module fail");
            return false;
        }

        //初始化数据库
        TraceLog( LogInfo, "init DB module");
        db_ptr_ = std::make_shared<db::DB>();
        if (!db_ptr_->init())
        {
            TraceLog( LogError, "init DB module fail");
            return false;
        }
        

        std::string path;
        db_ptr_->load_local_dir(path);
        factory.set_local_storage_path(path);
        db_ptr_->save_local_dir(factory.local_storage_path_);

        if (!adb::AdbShell::start_adb_server())
        {
            TraceLog( LogError, "start adb server fail");
            return false;
        }

        TraceLog( LogInfo, "init thread pool");
        if (!thread_pool_.begin(thread_pool_fun))
        {
            TraceLog( LogError, "begin thread pool fail");
            return false;
        }

        return true;
	}

    void CBusiness::uninit()
	{
        thread_pool_.end();

        adb::AdbShell::stop_adb_server();

        del_tmp_file();

        if (nullptr != curr_phone_ptr_)
        {
            curr_phone_ptr_.reset();
            curr_phone_ptr_ = nullptr;
        }

        if (nullptr != db_ptr_)
        {
            db_ptr_.reset();
            db_ptr_ = nullptr;
        }

        TraceLog( LogInfo, "Program exit");
        UninitLog();

        if (NULL != unique_mutex_)
        {
            CloseHandle(unique_mutex_);
            unique_mutex_ = NULL;
        }

        dump::uninit();
	}

    bool CBusiness::program_unique_check()
    {
        unique_mutex_ = CreateMutexA(NULL, FALSE, PROGRAM_UNIQUE_GUID);
        if (NULL != unique_mutex_)
        {
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                CloseHandle(unique_mutex_);
                HWND hWnd = ::FindWindowA(NULL, PROGRAM_MAIN_WIN_NAMEA);
                if (NULL != hWnd)
                {
                    if (::IsIconic(hWnd))
                    {
                        ::ShowWindow(hWnd, SW_RESTORE);
                    }
                    ::SetForegroundWindow(hWnd);
                }
                return false;
            }
        }
        else
        {
            return false;
        }
        return true;
    }
    
    void CBusiness::thread_pool_fun(TaskParam& params)
    {
        CBusiness* obj = (CBusiness*)params.object_;
        if (eTaskGetDevices == params.task_type_)
        {
            obj->on_get_device_lst();
        }
        else if (eTaskGetAllFile == params.task_type_)
        {
            obj->on_get_all_file(params.params_);
        }
        else if (eTaskGetDirFile == params.task_type_)
        {
            obj->on_get_dir_file(params.params_);
        }
        else if (eTaskRefreshLocalDir == params.task_type_)
        {
            obj->on_refresh_local_dir(params.params_);
        }
        else if (eTaskGetPhoneFileImage == params.task_type_)
        {
            obj->on_get_phone_file_image(params.params_);
        }
        else if (eTaskAddPhoneSyncDir == params.task_type_)
        {
            obj->on_add_phone_sync_dir(params.params_);
        }
        else if (eTaskDelPhoneSyncDir == params.task_type_)
        {
            obj->on_del_phone_sync_dir(params.params_);
        }
        else if (eTaskSyncPhoneData == params.task_type_)
        {
            obj->on_sync_phone_data(params.params_);
        }
        else if (eTaskSyncFile == params.task_type_)
        {
            obj->on_sync_file(params.params_);
        }
        else if (eTaskMatchFile == params.task_type_)
        {
            obj->on_match_file(params.params_);
        }
        else if (eTaskGetViewBigImage == params.task_type_)
        {
            obj->on_get_view_big_image(params.params_);
        }
        else if (eTaskGetViewImageList == params.task_type_)
        {
            obj->on_get_view_image_list(params.params_);
        }
        else if (eTaskGetViewThumbnail == params.task_type_)
        {
            obj->on_get_view_thumbnail(params.params_);
        }
        else if (eTaskDownloadPhoneFile == params.task_type_)
        {
            obj->on_download_phone_file(params.params_);
        }
        else if (eTaskDeleteFile == params.task_type_)
        {
            obj->on_delete_file(params.params_);
        }
        else if (eTaskRenameFile == params.task_type_)
        {
            obj->on_rename_file(params.params_);
        }
    }

    void CBusiness::post_task(int task_type, void* params)
    {
        TaskParam task(task_type, params, this);
        thread_pool_.post_task(task);
    }

    void CBusiness::do_get_device_lst()
    {
        post_task(eTaskGetDevices, NULL);
    }

    void CBusiness::on_get_device_lst()
    {
        adb::AdbShell shell;
        if (!shell.init(1024 * 5))
        {
            return;
        }

        std::vector<PhoneBaseInfo> base_lst;
        std::string script = "adb devices -l";
        if (shell.exe_adb_script(script))
        {
            const std::string& result = shell.get_result();
            parse_devices_data(result, base_lst);
        }
        else
        {
            TraceLog( LogError, "exe_adb_script fail, script:%s", script.c_str());
        }

        shell.uninit();
        TraceLog( LogInfo, "devices count:%d", base_lst.size());

        CDataFactory& factory = theDataFactory::instance();
        //获取新增的手机
        std::vector<PhoneBaseInfo>* new_list = new std::vector<PhoneBaseInfo>;
        std::vector<PhoneBaseInfo> phone_list = factory.get_phone_list();
        for (auto itor = base_lst.begin(); itor != base_lst.end(); ++itor)
        {
            auto phone_itor = phone_list.begin();
            for (; phone_itor != phone_list.end(); ++phone_itor)
            {
                if (0 == _wcsicmp(phone_itor->serial_no_.c_str(), itor->serial_no_.c_str()))
                {
                    break;
                }
            }

            if (phone_itor == phone_list.end())
            {
                new_list->push_back(*itor);
            }
        }

        if (base_lst.empty())
        {
            curr_phone_ptr_.reset();
            curr_phone_ptr_ = nullptr;
        }

        //设置手机列表
        factory.reset_phone_list(base_lst);
        ui::refresh_phone_device();

        //新增的手机同步数据
        if (!new_list->empty()) post_task(eTaskSyncPhoneData, new_list);
        else delete new_list;

    }

    void CBusiness::do_get_all_file(int file_type, int start_page, const std::wstring& phone_sn, bool is_local, bool need_total)
    {
        GetAllFileParam* info = new GetAllFileParam(file_type, start_page, phone_sn, is_local, need_total);

        post_task(eTaskGetAllFile, info);
    }

    void CBusiness::on_get_all_file(void* params)
    {
        CDataFactory& factory = theDataFactory::instance();
        GetAllFileParam* info = (GetAllFileParam*)params;
        std::vector<FileInfo>* file_lst = new std::vector<FileInfo>();
        bool is_success = false;
        int total = -1;
        int count = 0;
        std::string phone_sn = character::unicode_to_mbyte(info->phone_sn_);
        if (info->is_local_)
        {
            //从数据库获取
            if (info->need_total_)
            {
                total = db_ptr_->get_all_file_count(info->file_type_, phone_sn);
                if (total >= 0)
                {
                    is_success = true;
                }
            }

            if (!info->need_total_ || total > 0)
            {
                is_success = db_ptr_->get_all_file(info->file_type_,
                    phone_sn,
                    info->start_page_,
                    factory.max_num_,
                    factory.sort_type_,
                    factory.sort_field_,
                    *file_lst);
            }
        }
        else
        {
            //删除临时文件
            del_tmp_file();
            //从手机获取
            if (nullptr != curr_phone_ptr_)
            {
                if (info->need_total_)
                {
                    total = curr_phone_ptr_->get_all_file_count(info->file_type_);
                    if (total >= 0)
                    {
                        is_success = true;
                    }
                }

                if (!info->need_total_ || total > 0)
                {
                    is_success = curr_phone_ptr_->get_all_file(info->file_type_,
                        info->start_page_,
                        factory.max_num_,
                        factory.sort_type_,
                        factory.sort_field_,
                        *file_lst);
                }
            }
        }

        if (!is_success)
        {
            delete file_lst;
            file_lst = NULL;
        }
        else
        {
            count = file_lst->size();
        }

        lock::Lock lock(&factory.file_lst_lock_);
        delete factory.file_lst_;
        factory.file_lst_ = file_lst;

        ui::refresh_result(total, count, is_success, info->is_local_, info->start_page_, info->file_type_);
        delete info;
    }

    void CBusiness::do_get_dir_file(const std::wstring& dir, int start_page, const std::wstring& phone_sn, bool is_local, bool need_total)
    {
        GetDirFileParam* info = new GetDirFileParam(dir, start_page, phone_sn, is_local, need_total);

        post_task(eTaskGetDirFile, info);
    }    

    void CBusiness::on_get_dir_file(void* params)
    {
        CDataFactory& factory = theDataFactory::instance();
        GetDirFileParam* info = (GetDirFileParam*)params;
        std::vector<FileInfo>* file_lst = new std::vector<FileInfo>();
        bool result = false;
        int total = -1;
        int count = 0;
        if (info->is_local_)
        {
            result = get_local_dir_file(*info, total, *file_lst);
        }
        else
        {
            result = get_phone_dir_file(*info, total, *file_lst);
        }

        if (!result)
        {
            delete file_lst;
            file_lst = NULL;
        }
        else
        {
            count = file_lst->size();
        }

        lock::Lock lock(&factory.file_lst_lock_);
        delete factory.file_lst_;
        factory.file_lst_ = file_lst;

        ui::refresh_result(total, count, result, info->is_local_, info->start_page_, info->dir_);
        delete info;
    }

    void CBusiness::do_refresh_local_dir(const std::wstring& dir)
    {
        std::wstring* params = new std::wstring(dir);
        post_task(eTaskRefreshLocalDir, params);
    }

    void CBusiness::on_refresh_local_dir(void* params)
    {
        DWORD enter = ::GetTickCount();
        CDataFactory& factory = theDataFactory::instance();
        std::wstring* dir_ptr = (std::wstring*)params;
        std::string dir = character::unicode_to_mbyte(*dir_ptr);

        std::deque<std::string> dir_dequeue;
        dir_dequeue.push_back(get_complete_path(dir));

        WIN32_FIND_DATAA find_data;
        HANDLE find_handle = NULL;
        std::string root_path;
        std::string file_id;
        std::string tmp;
        std::string serial_no;
        std::string modify_time;
        modify_time.resize(20, '\0');
        while (!dir_dequeue.empty())
        {
            root_path = dir_dequeue.front();
            dir_dequeue.pop_front();

            //根据获路径取手机序列号
            serial_no = get_serial_no_by_path(root_path);

            tmp = root_path;
            if ('/' != tmp[tmp.size() - 1]) tmp.append("/");
            tmp.append("*");
            memset(&find_data, 0, sizeof(WIN32_FIND_DATAA));
            find_handle = FindFirstFileA(tmp.c_str(), &find_data);
            if (INVALID_HANDLE_VALUE == find_handle) continue;

            do 
            {
                if (strcmp(find_data.cFileName, "..") == 0 || strcmp(find_data.cFileName, ".") == 0)
                {
                    continue;
                }
                else if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                {
                    tmp = root_path + "/" + find_data.cFileName;
                    file_id = get_str_md5(tmp);
                    if (!db_ptr_->is_file_exist(file_id))
                    {
                        std::string modify_time = get_file_modify_time(find_data.ftLastWriteTime);
                        db_ptr_->add_file(file_id, find_data.cFileName, root_path, modify_time, find_data.nFileSizeLow, eFileDir, serial_no);
                        dir_dequeue.push_back(tmp);
                    }
                }
                else
                {
                    tmp = root_path + "/" + find_data.cFileName;
                    if (std::string::npos != tmp.rfind(".thumbnail.png"))
                    {
                        continue;
                    }

                    file_id = get_str_md5(tmp);
                    std::string modify_time = get_file_modify_time(find_data.ftLastWriteTime);
                    std::wstring modify_time_w = character::mbyte_to_unicode(modify_time);
                    FileInfo file_info;
                    if (!db_ptr_->get_file_info(file_id, file_info))
                    {
                        file_info.file_name_ = character::mbyte_to_unicode(find_data.cFileName);
                        file_info.file_path_ = character::mbyte_to_unicode(root_path);
                        file_info.modify_time_ = modify_time_w;
                        file_info.file_size_ = find_data.nFileSizeLow;
                        file_info.file_type_ = get_file_type(find_data.cFileName);
                        db_ptr_->add_file(file_id, find_data.cFileName, root_path, modify_time, file_info.file_size_, file_info.file_type_, serial_no);
                    }
                    else
                    {
                        if (file_info.file_size_ != find_data.nFileSizeLow || file_info.modify_time_ != modify_time_w)
                        {
                            db_ptr_->update_file(file_id, modify_time, find_data.nFileSizeLow);
                        }
                    }
                    
                    //生成缩略图
                    if (eFilePic == file_info.file_type_ && file_info.thumbnail_path_.empty())
                    {
                        int width = 0;
                        int height = 0;
                        std::string file_path = character::unicode_to_mbyte(file_info.file_path_ + L"/" + file_info.file_name_);
                        std::string thumbnail_path = character::unicode_to_mbyte(file_info.thumbnail_path_);
                        if (generate_thumbnail(file_path, thumbnail_path, width, height))
                        {
                            db_ptr_->update_file_thumbnail(file_id, thumbnail_path, width, height);
                        }                        
                    }
                }
            } while (FindNextFileA(find_handle, &find_data));

            FindClose(find_handle);
        }

        if (*dir_ptr == ui::get_curr_path())
        {
            ui::reload_data();
        }

        DWORD leave = ::GetTickCount();
        TraceLog( LogError, "***refresh tick, %d", leave - enter);
        delete dir_ptr;
    }

    std::string CBusiness::get_serial_no_by_path(const std::string& path)
    {
        std::string serial_no;
        CDataFactory& factory = theDataFactory::instance();
        if (path.size() > factory.local_storage_path_.size())
        {
            int n = path.find(factory.local_storage_path_);
            if (n != std::string::npos)
            {
                int len = factory.local_storage_path_.size();
                ++len;
                n = path.find('/', len);
                if (n != std::string::npos)
                {
                    serial_no = path.substr(len, n - len);
                }
                else
                {
                    serial_no = path.substr(len);
                }
            }
        }        

        return std::move(serial_no);
    }

    std::string CBusiness::get_file_modify_time(const FILETIME& ft)
    {
        SYSTEMTIME st = { 0 };
        SYSTEMTIME lt = { 0 };
        ::FileTimeToSystemTime(&ft, &st);
        ::SystemTimeToTzSpecificLocalTime(NULL, &st, &lt);

        std::string modify_time;
        modify_time.resize(20, '\0');
        int n = sprintf((char*)modify_time.c_str(), "%d-%02d-%02d %02d:%02d:%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
        modify_time.resize(n);
        return std::move(modify_time);
    }

    std::string CBusiness::get_complete_path(const std::string& dir)
    {
        std::string path;
        if (dir.empty())
        {
            path = theDataFactory::instance().local_storage_path_;
        }
        else
        {
            path = theDataFactory::instance().local_storage_path_ + "/" + dir;
        }

        if (path[path.size()-1] == '/')
        {
            path.erase(path.size() - 1, 1);
        }
        return std::move(path);
    }

    bool CBusiness::generate_thumbnail(const std::string& file_path, std::string& thumbnail_path, int& width, int& height)
    {
        IplImage* src = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_UNCHANGED);
        if (NULL == src)
        {
            TraceLog(LogError, "opencv read image fail, %s", file_path.c_str());
            return NULL;
        }

        if (src->height <= THUMBNAIL_IMAGE_HEIGHT && src->width <= THUMBNAIL_IMAGE_WIDTH)
        {
            width = src->width;
            height = src->height;
            cvReleaseImage(&src);
            thumbnail_path = file_path;
            return true;
        }

        if (4 == src->nChannels)
        {
            IplImage* tmp = cvCreateImage(cvGetSize(src), src->depth, 3);
            cvCvtColor(src, tmp, CV_RGBA2RGB);
            cvReleaseImage(&src);
            src = tmp;
        }

        //获取图片大小
        get_image_size(src->width, src->height, THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT, width, height);

        bool is_success = false;
        IplImage* dst = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
        if (NULL != dst)
        {
            cvResize(src, dst, CV_INTER_AREA);
            int pos = file_path.rfind('.');
            std::string thumbnail = file_path.substr(0, pos);
            thumbnail.append(".thumbnail.png");
            if (cvSaveImage(thumbnail.c_str(), dst))
            {
                thumbnail_path = thumbnail;
                is_success = true;
            }
            else
            {
                TraceLog(LogError, "save thumbnail image fail, %s", file_path.c_str());
            }
            cvReleaseImage(&dst);
        }
        cvReleaseImage(&src);
        return is_success;
    }

    void CBusiness::do_get_phone_file_image(void* params)
    {
        post_task(eTaskGetPhoneFileImage, params);
    }

    void CBusiness::on_get_phone_file_image(void* params)
    {
        PhoneFileInfo* info_ptr = (PhoneFileInfo*)params;
        CDataFactory& factory = theDataFactory::instance();
        std::string storage_path = factory.tmp_storage_path_;
        std::string file_name = character::unicode_to_mbyte(info_ptr->file_name_);
        storage_path.append(file_name);
        bool is_refresh = false;
        std::string phone_file = character::unicode_to_mbyte(info_ptr->file_path_);
        if (!phone_file.empty()) phone_file.append("/");
        phone_file.append(file_name);

        if (nullptr != curr_phone_ptr_ && curr_phone_ptr_->get_file(phone_file, storage_path))
        {
            void* image_ptr = create_image(storage_path, THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
            if (NULL != image_ptr)
            {
                is_refresh = true;
                ui::refresh_thumbnail_image(info_ptr, image_ptr);
            }
        }
        
        if (!is_refresh)
        {
            delete info_ptr;
        }        
    }

    void CBusiness::del_tmp_file()
    {
        std::string path = theDataFactory::instance().tmp_storage_path_;
        std::string tmp = path;
        tmp.append("*");

        WIN32_FIND_DATAA find_data;
        HANDLE find_handle = NULL;
        memset(&find_data, 0, sizeof(WIN32_FIND_DATAA));
        find_handle = FindFirstFileA(tmp.c_str(), &find_data);
        if (INVALID_HANDLE_VALUE == find_handle)
        {
            return;
        }

        do
        {
            if (strcmp(find_data.cFileName, "..") == 0 || strcmp(find_data.cFileName, ".") == 0)
            {
                continue;
            }
            else
            {
                std::string file = path + find_data.cFileName;
                delete_file(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY, file.c_str());
            }
        } while (FindNextFileA(find_handle, &find_data));

        FindClose(find_handle);
    }

    void CBusiness::do_add_phone_sync_file(int index, const std::wstring& phone_sn, const std::wstring& phone_dir, bool is_dir)
    {
        AddDelSyncFileParams* params = new AddDelSyncFileParams(index, phone_sn, phone_dir, is_dir);
        post_task(eTaskAddPhoneSyncDir, params);
    }

    void CBusiness::on_add_phone_sync_dir(void* params)
    {
        AddDelSyncFileParams* info = (AddDelSyncFileParams*)params;
        CDataFactory& factory = theDataFactory::instance();
        std::string phone_file = character::unicode_to_mbyte(info->phone_file_);
        std::string phone_sn = character::unicode_to_mbyte(info->phone_sn_);
        std::string file_id = get_str_md5(phone_file);
        if (db_ptr_->is_sync_file_exist(phone_sn, file_id))
        {
            ui::add_phone_sync_file_result(info->index_, 2);
            return;
        }
        
        if (nullptr != curr_phone_ptr_)
        {
            bool result = db_ptr_->add_phone_sync_file(phone_sn, file_id, phone_file, "", true, info->is_dir_, "");
            ui::add_phone_sync_file_result(info->index_, result);
            if (result)
            {
                SyncFileParam* params = new SyncFileParam(curr_phone_ptr_->serial_no_, phone_file, info->is_dir_, curr_phone_ptr_);
                post_task(eTaskSyncFile, params);
            }
        }
        
        delete info;
    }

    void CBusiness::do_del_phone_sync_file(int index, const std::wstring& phone_sn, const std::wstring& phone_dir)
    {
        AddDelSyncFileParams* params = new AddDelSyncFileParams(index, phone_sn, phone_dir, false);
        post_task(eTaskDelPhoneSyncDir, params);
    }

    void CBusiness::on_del_phone_sync_dir(void* params)
    {
        AddDelSyncFileParams* info = (AddDelSyncFileParams*)params;
        std::string file_id = get_str_md5(character::unicode_to_mbyte(info->phone_file_));
        bool result = db_ptr_->del_phone_sync_file(character::unicode_to_mbyte(info->phone_sn_), file_id);
        ui::del_phone_sync_file_result(info->index_, result);
        delete info;
    }

    void CBusiness::get_phone_sync_file(std::vector<FileInfo>& file_list)
    {
        CDataFactory& factory = theDataFactory::instance();
        for (auto itor = file_list.begin(); itor != file_list.end(); ++itor)
        {
            std::string phone_file = character::unicode_to_mbyte(itor->file_path_);
            if (!phone_file.empty())
            {
                phone_file.append("/");
            }
            phone_file.append(character::unicode_to_mbyte(itor->file_name_));
            std::string file_id = get_str_md5(phone_file);
            if (db_ptr_->is_sync_file_exist(character::unicode_to_mbyte(itor->serial_no_), file_id))
            {
                itor->is_sync_ = true;
            }
        }
    }

    void CBusiness::on_sync_phone_data(void* params)
    {
        CDataFactory& factory = theDataFactory::instance();
        std::vector<SyncInfo> sync_list;
        std::vector<PhoneBaseInfo>* phone_list = (std::vector<PhoneBaseInfo>*)params;
        for (auto itor = phone_list->begin(); itor != phone_list->end(); ++itor)
        {
            std::string serial_no = character::unicode_to_mbyte(itor->serial_no_);
            std::shared_ptr<phone::Phone> phone_ptr = std::make_shared<phone::Phone>(serial_no);
            if (!phone_ptr->init(1024))
            {
                continue;
            }

            sync_list.clear();
            db_ptr_->get_phone_sync_lst(serial_no, sync_list);
            for (auto sync_itor = sync_list.begin(); sync_itor != sync_list.end(); ++sync_itor)
            {
                SyncFileParam* params = new SyncFileParam(serial_no, sync_itor->file_path_, sync_itor->is_dir_, phone_ptr);
                post_task(eTaskSyncFile, params);
            }
        }
        delete phone_list;
    }

    void CBusiness::get_file_time_size(const std::string& path, std::string& time, int& size)
    {
        if (!file::file_exist(path)) return;

        HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE,
            NULL, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (NULL == hFile)
        {
            TraceLog(LogError, "CreateFileA fail:%d path:%s", GetLastError(), path.c_str());
            return;
        }

        FILETIME ct = { 0 };
        FILETIME at = { 0 };
        FILETIME wt = { 0 };
        if (::GetFileTime(hFile, &ct, &at, &wt))
        {
            time = get_file_modify_time(wt);
        }

        DWORD file_size = 0;
        ::GetFileSize(hFile, &file_size);
        size = file_size;

        CloseHandle(hFile);
    }

    void CBusiness::on_sync_file(void* params)
    {
        SyncFileParam* sync_info = (SyncFileParam*)params;
        std::string path = get_sync_store_path(sync_info->serial_no_, sync_info->phone_file_);

        //创建父目录
        int pos = path.rfind('/');
        std::string parent_dir = path.substr(0, pos);
        std::string file_name = path.substr(pos + 1);
        if (!file::file_exist(parent_dir))
        {
            file::create_folder(parent_dir);
        }

        //父目录写入目录数据库
        add_file_path_to_db(sync_info->serial_no_, sync_info->phone_file_);

        std::string root_id = get_str_md5(sync_info->phone_file_);
        if (!file::file_exist(path))//本地是否存在
        {
            sync_file(sync_info->serial_no_, sync_info->phone_file_, sync_info->is_dir_, sync_info->phone_ptr_, true, root_id);
        }
        else if (!sync_info->is_dir_)//文件
        {
            std::string file_id = get_str_md5(sync_info->phone_file_);
            std::string modify_time = db_ptr_->get_sync_file_time(sync_info->serial_no_, file_id);
            std::string phone_time = sync_info->phone_ptr_->get_file_modify_time(sync_info->phone_file_);
            if (modify_time != phone_time)
            {
                sync_file(sync_info->serial_no_, sync_info->phone_file_, sync_info->is_dir_, sync_info->phone_ptr_, true, root_id);
            }
        }
        else//目录，需要逐个文件比较
        {
            std::string modify_time = sync_info->phone_ptr_->get_file_modify_time(sync_info->phone_file_);
            sync_update_db(sync_info->serial_no_, sync_info->phone_file_, modify_time, 0, eFileDir, true, root_id);

            std::deque<std::string> dir_dequeue;
            dir_dequeue.push_back(sync_info->phone_file_);
            while (!dir_dequeue.empty())
            {
                std::string phone_file = dir_dequeue.front();
                dir_dequeue.pop_front();

                std::vector<FileInfo> file_lst;
                if (!sync_info->phone_ptr_->get_all_dir_file(phone_file, file_lst))
                {
                    continue;
                }

                for (auto itor = file_lst.begin(); itor != file_lst.end(); ++itor)
                {
                    phone_file = character::unicode_to_mbyte(itor->file_path_ + L"/" + itor->file_name_);
                    std::string store_path = get_sync_store_path(sync_info->serial_no_, phone_file);
                    if (!file::file_exist(store_path))
                    {
                        sync_file(sync_info->serial_no_, phone_file, eFileDir == itor->file_type_, sync_info->phone_ptr_, false, root_id);
                        continue;
                    }
                    
                    if (eFileDir == itor->file_type_)
                    {
                        dir_dequeue.push_back(phone_file);
                    }

                    std::string file_id = get_str_md5(phone_file);
                    modify_time = db_ptr_->get_sync_file_time(sync_info->serial_no_, file_id);
                    std::string m_time = character::unicode_to_mbyte(itor->modify_time_);
                    if (modify_time == m_time)
                    {
                        continue;
                    }

                    if (eFileDir != itor->file_type_)
                    {
                        sync_file(sync_info->serial_no_, phone_file, false, sync_info->phone_ptr_, false, root_id);
                    }
                    else
                    {
                        sync_update_db(sync_info->serial_no_, phone_file, m_time, 0, eFileDir, false, root_id);
                    }
                }//end for
            }//end while
        }

        delete sync_info;
    }

    void CBusiness::do_match_file(int index, const std::wstring& key)
    {
        std::pair<int, std::wstring>* params = new std::pair<int, std::wstring>(index, key);
        post_task(eTaskMatchFile, params);
    }

    void CBusiness::on_match_file(void* params)
    {
        std::pair<int, std::wstring>* info = (std::pair<int, std::wstring>*)params;
        //转换为小写
        char* p = (char*)info->second.c_str();
        while (*p)
        {
            if (*p >=65 && *p <= 90)
            {
                *p = *p + 32;
            }
            ++p;
        }

        CDataFactory& factory = theDataFactory::instance();
        lock::Lock lock(&factory.file_lst_lock_);
        if (NULL != factory.file_lst_)
        {
            int count = factory.file_lst_->size();
            int index = info->first;
            for (int i = 0; i < count; ++i, ++index)
            {
                if (index >= count)
                {
                    index = 0;
                }

                if (file_name_compare(factory.file_lst_->at(index).file_name_, info->second))
                {
                    ui::location_file(index);
                    break;
                }
            }
        }

        delete info;
    }

    bool CBusiness::file_name_compare(const std::wstring& src, const std::wstring& key)
    {
        if (src.size() < key.size())
        {
            return false;
        }

        const wchar_t* k_ch = key.c_str();
        const wchar_t* s_ch = src.c_str();
        while (*k_ch)
        {
            if (*k_ch != *s_ch)
            {
                if ((*k_ch-32) != *s_ch)
                {
                    return false;
                }
            }
            ++k_ch;
            ++s_ch;
        }

        return true;
    }

    bool CBusiness::get_local_dir_file(const GetDirFileParam& params, int& total, std::vector<FileInfo>& file_lst)
    {
        bool result = false;
        //从数据库获取
        std::string phone_sn = character::unicode_to_mbyte(params.phone_sn_);
        std::string dir = character::unicode_to_mbyte(params.dir_);
        CDataFactory& factory = theDataFactory::instance();
        std::string dir_id = get_str_md5(get_complete_path(dir));
        if (params.need_total_)
        {
            total = db_ptr_->get_dir_file_count(dir_id, phone_sn);
            if (total >= 0)
            {
                result = true;
            }
        }

        if (!params.need_total_ || total > 0)
        {
            result = db_ptr_->get_dir_file(dir_id,
                phone_sn,
                params.start_page_,
                factory.max_num_,
                factory.sort_type_,
                factory.sort_field_,
                file_lst);
        }

        return result;
    }

    bool CBusiness::get_phone_dir_file(const GetDirFileParam& params, int& total, std::vector<FileInfo>& file_lst)
    {
        bool result = false;
        CDataFactory& factory = theDataFactory::instance();
        //删除临时文件
        del_tmp_file();

        if (nullptr != curr_phone_ptr_)
        {
            std::string dir = character::unicode_to_mbyte(params.dir_);

            if (params.need_total_)
            {
                total = curr_phone_ptr_->get_dir_file_count(dir);
                if (total >= 0) result = true;
            }

            if (!params.need_total_ || total > 0)
            {
                result = curr_phone_ptr_->get_dir_file(dir,
                    params.start_page_,
                    factory.max_num_,
                    factory.sort_type_,
                    factory.sort_field_,
                    file_lst);

                if (result)get_phone_sync_file(file_lst);
            }
        }
        return result;
    }

    void CBusiness::set_curr_phone(const std::wstring& phone_sn)
    {
        if (nullptr != curr_phone_ptr_)
        {
            curr_phone_ptr_.reset();
            curr_phone_ptr_ = nullptr;
        }

        if (!phone_sn.empty())
        {
            curr_phone_ptr_ = std::make_shared<phone::Phone>(character::unicode_to_mbyte(phone_sn));
            curr_phone_ptr_->init();
        }
    }

    void CBusiness::parse_devices_data(const std::string& data, std::vector<PhoneBaseInfo>& phone_list)
    {
        const static std::string offset_string = "List of devices attached\r\n";
        int pos = data.find(offset_string);
        if (pos != std::string::npos)
        {
            PhoneBaseInfo base_info;
            std::string tmp = data.substr(pos + offset_string.size());

            std::vector<std::string> devices_list;
            split_string(tmp, '\r', devices_list);
            for (auto itor = devices_list.begin(); itor != devices_list.end(); ++itor)
            {
                if (std::string::npos == itor->find("device product:"))
                {
                    continue;
                }
                if ('\n' == itor->at(0))
                {
                    itor->erase(0, 1);
                }

                std::vector<std::string> info_list;
                split_string(*itor, ' ', info_list);
                if (info_list.size() != 11)
                {
                    continue;
                }
                base_info.serial_no_ = character::mbyte_to_unicode(info_list[0]);
                int n = info_list[10].find(":");
                base_info.device_ = character::mbyte_to_unicode(info_list[10].substr(n + 1));
                n = info_list[9].find(":");
                base_info.model_ = character::mbyte_to_unicode(info_list[9].substr(n + 1));
                n = info_list[8].find(":");
                base_info.product_ = character::mbyte_to_unicode(info_list[8].substr(n + 1));
                phone_list.push_back(base_info);
            }
        }
        else
        {
            TraceLog( LogError, "not find 'List of devices attached', %s", data.c_str());
        }
    }

    void CBusiness::add_sync_dir_file(const std::string& serial_no, const std::string& phone_file, const std::string& root_id)
    {
        std::string root_path = get_sync_store_path(serial_no, phone_file);

        std::deque<std::pair<std::string, std::string>> dir_dequeue;
        dir_dequeue.push_back(std::make_pair(root_path, phone_file));

        while (!dir_dequeue.empty())
        {
            auto dir_info = dir_dequeue.front();
            dir_dequeue.pop_front();

            root_path = dir_info.first;
            if ('/' != root_path[root_path.size() - 1]) root_path.append("/");

            std::string tmp = root_path + "*";

            WIN32_FIND_DATAA find_data = { 0 };
            HANDLE find_handle = FindFirstFileA(tmp.c_str(), &find_data);
            if (INVALID_HANDLE_VALUE == find_handle) continue;

            do
            {
                if (strcmp(find_data.cFileName, "..") == 0 || strcmp(find_data.cFileName, ".") == 0)
                {
                    continue;
                }
                else
                {
                    std::string phone_path = dir_info.second + "/" + find_data.cFileName;
                    eFileType file_type = eFileDir;
                    
                    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        tmp = root_path + find_data.cFileName;
                        dir_dequeue.push_back(std::make_pair(tmp, phone_path));
                    }
                    else
                    {
                        file_type = get_file_type(find_data.cFileName);
                    }
                    
                    std::string modify_time = get_file_modify_time(find_data.ftLastWriteTime);
                    sync_update_db(serial_no, phone_path, modify_time, find_data.nFileSizeLow, file_type, false, root_id);
                }                
            } while (FindNextFileA(find_handle, &find_data));

            FindClose(find_handle);
        }
    }

    std::string CBusiness::get_sync_store_path(const std::string& serial_no, const std::string& phone_file)
    {
        std::string path = theDataFactory::instance().local_storage_path_;
        path.append("/");
        path.append(serial_no);
        path.append("/");
        path.append(phone_file);

        return std::move(path);
    }

    void CBusiness::add_file_path_to_db(const std::string& serial_no, const std::string& phone_file)
    {
        std::string path = serial_no;
        path.append("/");
        path.append(phone_file);

        int pos = path.rfind('/');
        path = path.substr(0, pos);

        std::vector<std::string> folder_list;
        split_string(path, '/', folder_list);

        path = theDataFactory::instance().local_storage_path_;
        int count = folder_list.size();
        for (int i = 0; i < count; ++i)
        {
            std::string tmp = path + "/" + folder_list[i];
            std::string file_id = get_str_md5(tmp);
            if (!db_ptr_->is_file_exist(file_id))
            {
                std::string modify_time;
                int size = 0;
                get_file_time_size(tmp, modify_time, size);
                db_ptr_->add_file(file_id, folder_list[i], path, modify_time, size, eFileDir, serial_no);
            }
            path.append("/");
            path.append(folder_list[i]);            
        }  
    }

    void CBusiness::sync_file(const std::string& serial_no,
        const std::string& phone_file,
        bool is_dir,
        std::shared_ptr<phone::Phone>& phone_ptr,
        bool is_root,
        const std::string& root_id)
    {
        std::string path = get_sync_store_path(serial_no, phone_file);

        //创建父目录
        int pos = path.rfind('/');
        std::string parent_dir = path.substr(0, pos);
        std::string file_name = path.substr(pos + 1);

        if (phone_ptr->get_file(phone_file, path))
        {
            //添加同步列表
            std::string modify_time;
            int size = 0;
            get_file_time_size(path, modify_time, size);
 
            eFileType file_type = eFileDir;
            if (is_dir)//文件
            {
                std::string file_id = get_str_md5(path);
                db_ptr_->del_file(file_id);//删除本地目录，然后再添加
                add_sync_dir_file(serial_no, phone_file, root_id);
            }
            else
            {
                file_type = get_file_type(file_name);
            }
            sync_update_db(serial_no, phone_file, modify_time, size, file_type, is_root, root_id);
        }
    }

    void CBusiness::sync_update_db(const std::string& serial_no,
        const std::string& phone_file, 
        const std::string& modify_time,
        int file_size,
        eFileType file_type,
        bool is_root,
        const std::string& root_id)
    {
        std::string file_id = get_str_md5(phone_file);
        //添加同步表
        if (db_ptr_->is_sync_file_exist(serial_no, file_id))
        {
            db_ptr_->update_sync_file_time(serial_no, file_id, modify_time);
        }
        else
        {
            db_ptr_->add_phone_sync_file(serial_no, file_id, phone_file, modify_time, is_root, eFileDir == file_type, root_id);
        }

        //添加目录表
        std::string store_path = get_sync_store_path(serial_no, phone_file);
        file_id = get_str_md5(store_path);
        if (db_ptr_->is_file_exist(file_id))
        {
            db_ptr_->update_file(file_id, modify_time, file_size);
        }
        else
        {
            int pos = store_path.rfind('/');
            std::string parent_dir = store_path.substr(0, pos);
            std::string file_name = store_path.substr(pos + 1);
            db_ptr_->add_file(file_id, file_name, parent_dir, modify_time, file_size, file_type, serial_no);
        }

        //更新缩略图
        if (eFilePic == file_type)
        {
            std::string thumbnail_path;
            int width = 0;
            int height = 0;
            if (generate_thumbnail(store_path, thumbnail_path, width, height))
            {
                db_ptr_->update_file_thumbnail(file_id, thumbnail_path, width, height);
            }
        }
    }

    void CBusiness::do_get_view_big_image(const std::string& file_name, const std::string& file_path, bool is_phone, int max_width, int max_height, int rotate, const std::string& phone_sn)
    {
        ShowImageParam* params = new ShowImageParam(is_phone, file_name, file_path, max_width, max_height, rotate, phone_sn);
        post_task(eTaskGetViewBigImage, params);
    }

    void CBusiness::on_get_view_big_image(void* params)
    {
        ShowImageParam* pic_param = (ShowImageParam*)params;
        void* image_ptr = get_view_pic(pic_param->pic_name_, pic_param->pic_path_, pic_param->is_phone_, pic_param->max_width_, pic_param->max_height_, pic_param->rotate_, pic_param->phone_sn_);
        ui::refresh_view_big_image(pic_param->pic_name_, image_ptr);
        delete pic_param;
    }

    void* CBusiness::create_image(const std::string& file_path, int max_width, int max_height, int rotate/*=0*/)
    {
        IplImage* src = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_UNCHANGED);
        if (NULL == src)
        {
            TraceLog(LogError, "opencv read image fail, %s", file_path.c_str());
            return NULL;
        }

        if (4 == src->nChannels)
        {
            IplImage* tmp = cvCreateImage(cvGetSize(src), src->depth, 3);
            cvCvtColor(src, tmp, CV_RGBA2RGB);
            cvReleaseImage(&src);
            src = tmp;
        }

        //旋转图片
        src = (IplImage*)rotate_image(src, rotate);
        
        //获取图片大小
        int width = 0;
        int height = 0;
        get_image_size(src->width, src->height, max_width, max_height, width, height);

        IplImage* dst = src;
        //调整图片尺寸
        if (src->height != height && src->width != width)
        {
            dst = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
            if (NULL != dst)
            {
                cvResize(src, dst, CV_INTER_AREA);
            }
            cvReleaseImage(&src);
        }

        return dst;    
    }

    void CBusiness::do_get_view_image_list(const std::string& file_path, bool is_phone)
    {
        std::pair<std::string, bool>* params = new std::pair<std::string, bool>(file_path, is_phone);
        post_task(eTaskGetViewImageList, params);
    }

    void CBusiness::on_get_view_image_list(void* params)
    {
        std::pair<std::string, bool>* pic_info = (std::pair<std::string, bool>*)params;
        std::vector<std::pair<std::string, std::string>>* pic_list = new std::vector<std::pair<std::string, std::string>>;
        if (pic_info->second)//手机
        {
            if (nullptr != curr_phone_ptr_)
            {
                curr_phone_ptr_->get_pic_list(pic_info->first, *pic_list);
            }
        }
        else
        {
            std::string dir_id = get_str_md5(pic_info->first);
            db_ptr_->get_pic_list(dir_id, *pic_list);
        }
        delete pic_info;
        ui::refresh_view_image_list(pic_list);
    }

    void CBusiness::do_get_view_thumbnail(const std::string& file_name, const std::string& file_path, bool is_phone, int max_width, int max_height, int index, const std::string& phone_sn)
    {
        ShowThumbnailParam* params = new ShowThumbnailParam(file_name, file_path, is_phone, max_width, max_height, index, phone_sn);
        post_task(eTaskGetViewThumbnail, params);
    }

    void CBusiness::on_get_view_thumbnail(void* params)
    {
        ShowThumbnailParam* info = (ShowThumbnailParam*)params;
        void* image = get_view_pic(info->pic_name_, info->pic_path_, info->is_phone_, info->max_width_, info->max_height_, 0, info->phone_sn_);
        void* dark_image = NULL;
        if (NULL != image)
        {
            dark_image = image_birghtness_adjust(image, -20);
        }
        ui::refresh_view_thumbnail(info->index_, image, dark_image);
        delete info;
    }

    void* CBusiness::get_view_pic(const std::string& file_name, const std::string& file_path, bool is_phone, int max_width, int max_height, int rotate, const std::string& phone_sn)
    {
        CDataFactory& factory = theDataFactory::instance();
        std::string path;
        if (is_phone)
        {
            path = factory.view_pic_store_path_ + file_name;
            if (!file::file_exist(path) && nullptr != curr_phone_ptr_ && curr_phone_ptr_->serial_no_ == phone_sn)
            {
                std::string phone_file = file_path;
                if (!phone_file.empty())
                {
                    phone_file.append("/");
                    phone_file.append(file_name);
                }

                curr_phone_ptr_->get_file(phone_file, path);
            }
        }
        else
        {
            path = file_path + "/" + file_name;
        }

        void* image_ptr = NULL;
        if (file::file_exist(path))
        {
            image_ptr = create_image(path, max_width, max_height, rotate);
        }
        return image_ptr;
    }

    void*  CBusiness::rotate_image(void* image, int degree)
    {
        if (0 == degree || 360 == degree)
        {
            return image;
        }

        IplImage* img = (IplImage*)image;
        double angle = degree  * CV_PI / 180;
        double a = sin(angle), b = cos(angle);
        int width = img->width;
        int height = img->height;

        //旋转后的新图尺寸  
        int width_rotate = int(height * fabs(a) + width * fabs(b));
        int height_rotate = int(width * fabs(a) + height * fabs(b));
        IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), img->depth, img->nChannels);
        cvZero(img_rotate);
        //保证原图可以任意角度旋转的最小尺寸  
        int tempLength = sqrt((double)width * width + (double)height *height) + 10;
        int tempX = (tempLength + 1) / 2 - width / 2;
        int tempY = (tempLength + 1) / 2 - height / 2;
        IplImage* temp = cvCreateImage(cvSize(tempLength, tempLength), img->depth, img->nChannels);
        cvZero(temp);
        //将原图复制到临时图像tmp中心  
        cvSetImageROI(temp, cvRect(tempX, tempY, width, height));
        cvCopy(img, temp, NULL);
        cvResetImageROI(temp);
        //旋转数组map  
        // [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]  
        // [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]  
        float m[6];
        int w = temp->width;
        int h = temp->height;
        m[0] = b;
        m[1] = a;
        m[3] = -m[1];
        m[4] = m[0];
        // 将旋转中心移至图像中间 
        m[2] = w * 0.5f;
        m[5] = h * 0.5f;
        CvMat M = cvMat(2, 3, CV_32F, m);
        cvGetQuadrangleSubPix(temp, img_rotate, &M);
        cvReleaseImage(&temp);
        cvReleaseImage(&img);

        return img_rotate;
    }

    void* CBusiness::image_birghtness_adjust(void* image, int brightness)
    {
        IplImage* src_img = (IplImage*)image;
        IplImage* dst_img = cvCloneImage(src_img);
        if (NULL == dst_img)
        {
            return NULL;
        }

        float value = brightness;
        float val;
        for (int i = 0; i < 3; i++)//彩色图像需要处理3个通道，灰度图像这里可以删掉
        {
            for (int y = 0; y < src_img->height; y++)
            {
                for (int x = 0; x < src_img->width; x++)
                {
                    val = ((uchar*)(src_img->imageData + src_img->widthStep*y))[x * 3 + i];
                    val += value;
                    //对灰度值的可能溢出进行处理
                    if (val > 255)    val = 255;
                    if (val < 0) val = 0;
                    ((uchar*)(dst_img->imageData + dst_img->widthStep*y))[x * 3 + i] = (uchar)val;
                }
            }
        }

        return dst_img;
    }

    void CBusiness::get_image_size(int src_w, int src_h, int max_w, int max_h, int& dst_w, int& dst_h)
    {
        dst_w = src_w;
        dst_h = src_h;
        if (src_h <= max_h && src_w <= max_w)
        {
            return;
        }

        if ((src_h > max_h && src_w < max_w))
        {
            dst_h = max_h;
            double h1 = src_h;
            double h2 = dst_h;
            dst_w = src_w*(h2 / h1);
        }
        else if (src_h < max_h && src_w > max_w)
        {
            dst_w = max_w;
            double w1 = src_w;
            double w2 = dst_w;
            dst_h = src_h*(w2 / w1);
        }
        else
        {
            double h1 = src_h;
            double w1 = src_w;
            double h2 = max_h;
            double w2 = max_w;
            if ((h1 / w1) > (h2 / w2))
            {
                dst_h = max_h;
                dst_w = src_w*(h2 / h1);
            }
            else
            {
                dst_w = max_w;
                dst_h = src_h*(w2 / w1);
            }

            if (abs(max_w - dst_w) <= 2) dst_w = max_w;

            if (abs(max_h - dst_h) <= 2) dst_h = max_h;
        }
    }

    void CBusiness::do_download_phone_file(const std::string& phone_sn, const std::string& phone_file, const std::string& store_path)
    {
        DownloadFileParam* params = new DownloadFileParam(phone_sn, phone_file, store_path);
        post_task(eTaskDownloadPhoneFile, params);
    }

    void CBusiness::on_download_phone_file(void* params)
    {
        DownloadFileParam* file_info = (DownloadFileParam*)params;
        if (nullptr != curr_phone_ptr_ && file_info->phone_sn_ == curr_phone_ptr_->serial_no_)
        {
            curr_phone_ptr_->get_file(file_info->phone_file_, file_info->store_path_);
        }
        delete file_info;
    }

    void CBusiness::do_delete_file(const std::wstring& file, bool is_phone, const std::wstring& phone_sn)
    {
        DeleteFileParam* params = new DeleteFileParam(file, is_phone, phone_sn);
        post_task(eTaskDeleteFile, params);
    }

    void CBusiness::on_delete_file(void* params)
    {
        DeleteFileParam* del_info = (DeleteFileParam*)params;
        std::string phone_sn = character::unicode_to_mbyte(del_info->phone_sn_);
        std::string file = character::unicode_to_mbyte(del_info->file_);
        if (del_info->is_phone_)//手机
        {
            if (nullptr != curr_phone_ptr_ && phone_sn == curr_phone_ptr_->serial_no_)
            {
                curr_phone_ptr_->delete_file(file);
            }
        }
        else//删除本地
        {
            std::string file_id = get_str_md5(file);
            FileInfo file_info;
            if (db_ptr_->get_file_info(file_id, file_info))
            {
                if (!file_info.thumbnail_path_.empty())
                {
                    delete_file(false, character::unicode_to_mbyte(file_info.thumbnail_path_).c_str());
                }
            }

            delete_file(eFileDir == file_info.file_type_, file.c_str());
            if (eFileDir == file_info.file_type_)
            {
                std::deque<std::string> dir_id_dequeue;
                do 
                {
                    std::vector<FileInfo> file_list;
                    db_ptr_->get_file_list(file_id, file_list);
                    for (auto itor = file_list.begin(); itor != file_list.end(); ++itor)
                    {
                        if (eFileDir == itor->file_type_)
                        {
                            std::string tmp = character::unicode_to_mbyte(itor->file_path_ + L"/" + itor->file_name_);
                            dir_id_dequeue.push_back(get_str_md5(tmp));
                        }
                    }
                    db_ptr_->del_file(file_id);

                    file_id = dir_id_dequeue.front();
                    dir_id_dequeue.pop_front();
                } while (!dir_id_dequeue.empty());
            }
            else db_ptr_->del_file(file_id);
            
            //删除同步目录
            CDataFactory& factory = theDataFactory::instance();
            int i = file.find(factory.local_storage_path_);
            if (i != std::string::npos)
            {
                i = factory.local_storage_path_.size();
                int j = del_info->file_.find('/', i+1);
                if (j != std::string::npos)
                {
                    std::string phone_sn = file.substr(i + 1, j - i - 1);
                    std::string phone_file = file.substr(j + 1);
                    file_id = get_str_md5(phone_file);
                    db_ptr_->del_phone_sync_file(phone_sn, file_id);
                }
            }
        }
        delete del_info;
    }

    void CBusiness::do_rename_file(const std::wstring& file_name, const std::wstring& new_name, const std::wstring& path, bool is_phone)
    {
        RenameFileParam* params = new RenameFileParam(file_name, new_name, path, is_phone);
        post_task(eTaskRenameFile, params);
    }

    void CBusiness::on_rename_file(void* params)
    {
        RenameFileParam* file_info = (RenameFileParam*)params;
        std::string old_name = character::unicode_to_mbyte(file_info->file_name_);
        std::string new_name = character::unicode_to_mbyte(file_info->new_name_);
        std::string path = character::unicode_to_mbyte(file_info->path_);
        bool is_phone = file_info->is_phone_;
        delete file_info;

        std::string old_file = path;
        if (!path.empty()) old_file.append("/");
        std::string new_file = old_file;

        old_file.append(old_name);
        new_file.append(new_name);

        if (file_info->is_phone_ && nullptr != curr_phone_ptr_)
        {
            rename_phone_file(path, old_file, new_file);
            if (path.empty()) path = theDataFactory::instance().local_storage_path_ + "/" + curr_phone_ptr_->serial_no_;
            else path = theDataFactory::instance().local_storage_path_ + "/" + curr_phone_ptr_->serial_no_ + "/" + path;
        }

        rename_local_file(path, old_name, new_name);
    }

    void  CBusiness::rename_phone_file(const std::string& path, const std::string& old_file, const std::string& new_file)
    {
        if (nullptr == curr_phone_ptr_ || !curr_phone_ptr_->rename_file(old_file, new_file)) return;

        std::string phone_sn = curr_phone_ptr_->serial_no_;
        std::string file_path;
        std::string modify_time;
        bool is_root = false;
        bool is_dir = false;
        std::string root_id;
        std::string old_id = get_str_md5(old_file);
        if (!db_ptr_->get_phone_sync_file_info(phone_sn, old_id, file_path, modify_time, is_root, is_dir, root_id)) return;

        std::string new_id = get_str_md5(new_file);
        db_ptr_->update_sync_file_id(phone_sn, old_id, new_id, new_file, modify_time, root_id);

        if (!is_dir) return;

        if (is_root) root_id = new_id;

        std::deque<std::string> path_deque;
        path_deque.push_back(new_file);
        while (!path_deque.empty())
        {
            std::string phone_dir = path_deque.front();
            path_deque.pop_front();

            //获取手机目录文件
            std::vector<FileInfo> file_lst;
            if (!curr_phone_ptr_->get_all_dir_file(phone_dir, file_lst) && !file_lst.empty()) continue;

            int n = phone_dir.find('/', path.size() + 1);
            std::string old_dir = file_path;
            if (n != std::string::npos)
            {
                old_dir.append(phone_dir.substr(n));
            }

            for (auto itor = file_lst.begin(); itor != file_lst.end(); ++itor)
            {
                std::string name = character::unicode_to_mbyte(itor->file_name_);
                std::string new_f = phone_dir + "/" + name;
                std::string old_f = old_dir + "/" + name;
                if (eFileDir == itor->file_type_)
                {
                    path_deque.push_back(new_f);
                }

                modify_time = character::unicode_to_mbyte(itor->modify_time_);
                new_id = get_str_md5(new_f);
                old_id = get_str_md5(old_f);
                db_ptr_->update_sync_file_id(phone_sn, old_id, new_id, new_f, modify_time, root_id);
            }
        }
    }

    void  CBusiness::rename_local_file(const std::string& path, const std::string& old_name, const std::string& new_name)
    {
        std::string old_file = path + "/" + old_name;
        std::string new_file = path + "/" + new_name;
        if (file::file_exist(old_file))
        {
            rename(old_file.c_str(), new_file.c_str());
        }

        std::string old_id = get_str_md5(old_file);
        FileInfo file_info;
        if (!db_ptr_->get_file_info(old_id, file_info)) return;

        std::string new_id = get_str_md5(new_file);
        std::string dir_id = get_str_md5(path);
        std::string serial_no = get_serial_no_by_path(new_file);
        std::string old_thumbnail = character::unicode_to_mbyte(file_info.thumbnail_path_);
        std::string new_thumbnail = old_thumbnail;
        if (eFilePic == file_info.file_type_)
        {
            int pos = old_thumbnail.find(".thumbnail.png");
            if (pos == std::string::npos) new_thumbnail = new_file;
            else
            {
                pos = new_file.rfind('.');
                new_thumbnail = new_file.substr(0, pos);
                new_thumbnail.append(".thumbnail.png");
                rename(old_thumbnail.c_str(), new_thumbnail.c_str());
            }
        }
        db_ptr_->update_file_id(old_id, new_id, new_name, path, dir_id, serial_no, new_thumbnail);

        if (eFileDir != file_info.file_type_) return;

        std::vector<FileInfo> file_lst;
        std::deque<std::string> dir_deque;
        dir_deque.push_back(old_file);
        while (!dir_deque.empty())
        {
            std::string old_dir = dir_deque.front();
            dir_deque.pop_front();

            dir_id = get_str_md5(old_dir);
            file_lst.clear();
            db_ptr_->get_file_list(dir_id, file_lst);
            std::string new_dir = new_file;
            int n = old_dir.find('/', path.size() + 1);
            if (n != std::string::npos)
            {
                new_dir.append(old_dir.substr(n));
            }
            dir_id = get_str_md5(new_dir);

            for (auto itor = file_lst.begin(); itor != file_lst.end(); ++itor)
            {
                std::string file_name = character::unicode_to_mbyte(itor->file_name_);
                old_file = old_dir + "/" + file_name;
                old_id = get_str_md5(old_file);
                if (eFileDir == itor->file_type_)dir_deque.push_back(old_file);

                std::string file = new_dir + "/" + file_name;
                old_thumbnail = character::unicode_to_mbyte(itor->thumbnail_path_);
                new_thumbnail = old_thumbnail;
                if (eFilePic == itor->file_type_)
                {
                    int pos = old_thumbnail.find(".thumbnail.png");
                    if (pos == std::string::npos)
                    {
                        new_thumbnail = file;
                    }
                    else
                    {
                        pos = file.rfind('.');
                        new_thumbnail = file.substr(0, pos);
                        new_thumbnail.append(".thumbnail.png");
                        rename(old_thumbnail.c_str(), new_thumbnail.c_str());
                    }
                }
                
                new_id = get_str_md5(new_dir + "/" + file_name);           
                db_ptr_->update_file_id(old_id, new_id, file_name, new_dir, dir_id, serial_no, new_thumbnail);
            }//end for
        }//end while
    }


    void  CBusiness::delete_file(bool is_dir, const std::string& file)
    {
        if (is_dir) file::delete_dir(file);
        else file::delete_file(file);
    }


}


