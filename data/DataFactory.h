#ifndef DATA_FACTORY_H
#define DATA_FACTORY_H
#include <vector>
#include <singleton.h>
#include <log.h>
#include <lock.h>
#include "Common.h"


class CDataFactory :public Log::ModuleLog
{
public:
	CDataFactory();
	~CDataFactory();

public:
    //初始化
    bool init();

    void set_local_storage_path(const std::string& path);

    std::vector<PhoneBaseInfo> get_phone_list();

    void reset_phone_list(const std::vector<PhoneBaseInfo>& phone_list);
public:
    std::string                 program_path_;  //程序路径
    std::string                 tmp_storage_path_;//临时存储路径
    std::string                 view_pic_store_path_;//显示图片存储路径
    std::string                 local_storage_path_;//本地存储路径
    int                         max_num_;       //一页最大显示文件个数
    int                         max_num_date_;  //日期视图最大显示个数
    eSortType                   sort_type_;     //排序类型
    eSortField                  sort_field_;    //排序字段

    lock::Mutex                 file_lst_lock_; //文件列表锁
    std::vector<FileInfo>*      file_lst_;      //文件列表
    std::string                 save_as_path_;  //另存为路径
private:
    
    std::vector<PhoneBaseInfo>  phone_list_;  //手机列表
    lock::Mutex                 phone_list_lock_;

};

typedef comm::Singleton<CDataFactory> theDataFactory;
#endif

