#ifndef BUSINESS_INTERFACE_H
#define BUSINESS_INTERFACE_H

#include <string>
#include <thread_pool.h>
#include <log.h>
#include <catch_dump.h>
#include <singleton.h>
#include "Phone.h"
#include "DB.h"
#include "../data/Common.h"

namespace business
{
    class CBusiness :public Log::ModuleLog
    {
    private:
        enum eTaskType
        {
            eTaskGetDevices,            //��ȡ�豸�б�
            eTaskGetAllFile,            //��ȡ�����ļ�
            eTaskGetDirFile,            //��ȡĿ¼���ļ�
            eTaskRefreshLocalDir,       //ˢ�±���Ŀ¼
            eTaskGetPhoneFileImage,     //��ȡ�ֻ��ļ�ͼƬ
            eTaskAddPhoneSyncDir,       //����ֻ�ͬ��Ŀ¼
            eTaskDelPhoneSyncDir,       //ɾ���ֻ�ͬ��Ŀ¼
            eTaskSyncPhoneData,         //ͬ���ֻ�����
            eTaskSyncFile,              //ͬ��Ŀ¼
            eTaskMatchFile,             //ƥ���ļ�
            eTaskGetViewBigImage,       //��ȡ�鿴��ͼ
            eTaskGetViewImageList,      //��ȡ�鿴ͼƬ�б�
            eTaskGetViewThumbnail,      //��ȡ�鿴����ͼ
            eTaskDownloadPhoneFile,     //�����ֻ��ļ�
            eTaskDeleteFile,            //ɾ���ļ�
            eTaskRenameFile,            //�������ļ�
        };

        struct TaskParam
        {
            TaskParam() :task_type_(0), params_(NULL), object_(NULL){}
            TaskParam(int task_type, void* params, void* object) :task_type_(task_type), params_(params), object_(object){}

            int task_type_;
            void* params_;
            void* object_;
        };

        struct GetAllFileParam
        {
            GetAllFileParam(){}
            GetAllFileParam(int file_type, int start_page, const std::wstring& phone_sn, bool is_local, bool need_total) :
                file_type_(file_type), start_page_(start_page), phone_sn_(phone_sn), is_local_(is_local), need_total_(need_total){}
            int          file_type_;
            int          start_page_;
            std::wstring phone_sn_;
            bool         is_local_;
            bool         need_total_;
        };

        struct GetDirFileParam
        {
            GetDirFileParam(){}
            GetDirFileParam(const std::wstring& dir, int start_page, const std::wstring& phone_sn, bool is_local, bool need_total) :
                dir_(dir), start_page_(start_page), phone_sn_(phone_sn), is_local_(is_local), need_total_(need_total){}
            std::wstring dir_;
            int          start_page_;
            std::wstring phone_sn_;
            bool         is_local_;
            bool         need_total_;
        };

        struct AddDelSyncFileParams
        {
            AddDelSyncFileParams(int index, const std::wstring& phone_sn, const std::wstring& phone_file, bool is_dir) :
                index_(index), phone_sn_(phone_sn), phone_file_(phone_file), is_dir_(is_dir){};

            int          index_;
            std::wstring phone_sn_;
            std::wstring phone_file_;
            bool         is_dir_;
        };

        struct SyncFileParam
        {
            SyncFileParam(const std::string& serial_no,
                const std::string& phone_file,
                bool is_dir,
                const std::shared_ptr<phone::Phone>& phone_ptr) :serial_no_(serial_no), phone_file_(phone_file), is_dir_(is_dir), phone_ptr_(phone_ptr)
            {

            }
            std::string serial_no_;
            std::string phone_file_;
            bool        is_dir_;
            std::shared_ptr<phone::Phone> phone_ptr_;
        };

        struct ShowImageParam
        {
            ShowImageParam(bool is_phone, const std::string& pic_name, const std::string& pic_path, int max_width, int max_height, int rotate, const std::string& phone_sn) :
                is_phone_(is_phone), pic_name_(pic_name), pic_path_(pic_path), max_width_(max_width), max_height_(max_height), rotate_(rotate), phone_sn_(phone_sn){}

            bool is_phone_;
            std::string pic_name_;
            std::string pic_path_;
            int         max_width_;
            int         max_height_;
            int         rotate_;
            std::string phone_sn_;
        };

        struct ShowThumbnailParam
        {
            ShowThumbnailParam(const std::string& pic_name, const std::string pic_path, bool is_phone, int max_width, int max_height, int index, const std::string& phone_sn) :
                pic_name_(pic_name), pic_path_(pic_path), is_phone_(is_phone), max_width_(max_width), max_height_(max_height), index_(index), phone_sn_(phone_sn){}
            std::string pic_name_;
            std::string pic_path_;
            bool is_phone_;
            int max_width_;
            int max_height_;
            int index_;
            std::string phone_sn_;
        };

        struct DownloadFileParam 
        {
            DownloadFileParam(const std::string& phone_sn, const std::string& phone_file, const std::string& store_path):
                phone_sn_(phone_sn), phone_file_(phone_file), store_path_(store_path){}
            std::string phone_sn_;
            std::string phone_file_;
            std::string store_path_;
        };

        struct DeleteFileParam 
        {
            DeleteFileParam(const std::wstring& file, bool is_phone, const std::wstring& phone_sn):
                file_(file), is_phone_(is_phone), phone_sn_(phone_sn){}
            std::wstring file_;
            bool         is_phone_;
            std::wstring phone_sn_;
        };

        struct RenameFileParam 
        {
            RenameFileParam(const std::wstring& file_name, const std::wstring& new_name, const std::wstring& path, bool is_phone) :
                file_name_(file_name), new_name_(new_name), path_(path), is_phone_(is_phone){}
            std::wstring file_name_;
            std::wstring new_name_;
            std::wstring path_;
            bool is_phone_;
        };

    public:
        CBusiness();
        ~CBusiness();

    public:
        //��ʼ��
        bool init();

        //����
        void uninit();

        //��ȡ�豸�б�
        void do_get_device_lst();

        //��ȡ�����ļ�
        void do_get_all_file(int file_type, int start_page, const std::wstring& phone_sn, bool is_local, bool need_total);
        //��ȡĿ¼�������ļ�
        void do_get_dir_file(const std::wstring& dir, int start_page, const std::wstring& phone_sn, bool is_local, bool need_total);
        //ˢ��Ŀ¼
        void do_refresh_local_dir(const std::wstring& dir);
        //��ȡ�ֻ��ļ�ͼƬ
        void do_get_phone_file_image(void* params);
        //����ֻ�ͬ���ļ�
        void do_add_phone_sync_file(int index, const std::wstring& phone_sn, const std::wstring& phone_dir, bool is_dir);
        //ɾ���ֻ�ͬ���ļ�
        void do_del_phone_sync_file(int index, const std::wstring& phone_sn, const std::wstring& phone_dir);
        //ƥ���ļ�
        void do_match_file(int index, const std::wstring& key);
        //���õ�ǰ�ֻ�
        void set_curr_phone(const std::wstring& phone_sn);
        //��ȡ�鿴��ͼ
        void do_get_view_big_image(const std::string& file_name, const std::string& file_path, bool is_phone, int max_width, int max_height, int rotate, const std::string& phone_sn);
        //��ȡ�鿴ͼƬ�б�
        void do_get_view_image_list(const std::string& file_path, bool is_phone);
        //��ȡ�鿴����ͼ
        void do_get_view_thumbnail(const std::string& file_name, const std::string& file_path, bool is_phone, int max_width, int max_height, int index, const std::string& phone_sn);
        //�����ֻ��ļ�
        void do_download_phone_file(const std::string& phone_sn, const std::string& phone_file, const std::string& store_path);
        //ɾ���ļ�
        void do_delete_file(const std::wstring& file, bool is_phone, const std::wstring& phone_sn);
        //�������ļ�
        void do_rename_file(const std::wstring& file_name, const std::wstring& new_name, const std::wstring& path, bool is_phone);
private:

        static void thread_pool_fun(TaskParam& params);
        void post_task(int task_type, void* params);

        void on_get_device_lst();
        void on_get_all_file(void* params);
        void on_get_dir_file(void* params);
        void on_refresh_local_dir(void* params);
        void on_get_phone_file_image(void* params);
        void on_add_phone_sync_dir(void* params);
        void on_del_phone_sync_dir(void* params);
        void on_sync_phone_data(void* params);
        void on_sync_file(void* params);
        void on_match_file(void* params);
        void on_get_view_big_image(void* params);
        void on_get_view_image_list(void* params);
        void on_get_view_thumbnail(void* params);
        void on_download_phone_file(void* params);
        void on_delete_file(void* params);
        void on_rename_file(void* params);

        //����Ψһ��У��
        bool program_unique_check();

        std::string get_serial_no_by_path(const std::string& path);
        std::string get_file_modify_time(const FILETIME& ft);
        void get_file_time_size(const std::string& path, std::string& time, int& size);
        std::string get_complete_path(const std::string& dir);
        bool generate_thumbnail(const std::string& file_path, std::string& thumbnail_path, int& width, int& height);
        void del_tmp_file();
        void get_phone_sync_file(std::vector<FileInfo>& file_list);
        bool file_name_compare(const std::wstring& src, const std::wstring& key);
        bool get_local_dir_file(const GetDirFileParam& params, int& total, std::vector<FileInfo>& file_lst);
        bool get_phone_dir_file(const GetDirFileParam& params, int& total, std::vector<FileInfo>& file_lst);
        void parse_devices_data(const std::string& data, std::vector<PhoneBaseInfo>& phone_list);
        void add_sync_dir_file(const std::string& serial_no, const std::string& phone_file, const std::string& root_id);
        std::string get_sync_store_path(const std::string& serial_no, const std::string& phone_file);

        void add_file_path_to_db(const std::string& serial_no, const std::string& phone_file);
        void sync_file(const std::string& serial_no,
            const std::string& phone_file,
            bool is_dir,
            std::shared_ptr<phone::Phone>& phone_ptr,
            bool is_root,
            const std::string& root_id);
        void sync_update_db(const std::string& serial_no,
            const std::string& phone_file,
            const std::string& modify_time,
            int file_size,
            eFileType file_type,
            bool is_root,
            const std::string& root_id);

        void* create_image(const std::string& file_path, int max_width, int max_height, int rotate = 0);
        void* get_view_pic(const std::string& file_name, const std::string& file_path, bool is_phone, int max_width, int max_height, int rotate, const std::string& phone_sn);
        void* rotate_image(void* image, int degree);
        void* image_birghtness_adjust(void* image, int brightness);
        void  get_image_size(int src_w, int src_h, int max_w, int max_h, int& dst_w, int& dst_h);
        void  rename_phone_file(const std::string& path, const std::string& old_file, const std::string& new_file);
        void  rename_local_file(const std::string& path, const std::string& old_name, const std::string& new_name);
        void  delete_file(bool is_dir, const std::string& file);
    private:
        HANDLE                          unique_mutex_;
        thread::ThreadPool<TaskParam>   thread_pool_;
        std::shared_ptr<phone::Phone>   curr_phone_ptr_;
        std::shared_ptr<db::DB>         db_ptr_;
    };
}

typedef comm::Singleton<business::CBusiness> theBusiness;

#endif

