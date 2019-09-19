#include "DataFactory.h"
#include <system.h>
#include "../ui/ui_interface.h"
#include <file.h>

CDataFactory::CDataFactory():ModuleLog("DataFactory")
, max_num_(200)
, max_num_date_(50)
, sort_type_(eSortAse)
, sort_field_(eSortByName)
{
    
}

CDataFactory::~CDataFactory()
{
}

bool CDataFactory::init()
{
    if (!sys::get_module_path(program_path_))
    {
        return false;
    }

    tmp_storage_path_ = program_path_;
    tmp_storage_path_.append("tmp\\");
    string_replace(tmp_storage_path_, '\\', '/');
    if (!file::file_exist(tmp_storage_path_))
    {
        return file::create_folder(tmp_storage_path_);
    }

    view_pic_store_path_ = program_path_;
    view_pic_store_path_.append("PicView\\");
    string_replace(view_pic_store_path_, '\\', '/');
    if (!file::file_exist(view_pic_store_path_))
    {
        return file::create_folder(view_pic_store_path_);
    }
    
    return true;
}

void CDataFactory::set_local_storage_path(const std::string& path)
{
    local_storage_path_ = path;
    if (local_storage_path_.empty())
    {
        local_storage_path_ = program_path_;
        local_storage_path_.append("LocalFile");
    }

    if (!file::file_exist(local_storage_path_))
    {
        file::create_folder(local_storage_path_);
    }

    string_replace(local_storage_path_, '\\', '/');
    if ('/' == local_storage_path_[local_storage_path_.size() - 1])
    {
        auto itor = local_storage_path_.rbegin();
        local_storage_path_.erase(local_storage_path_.size() - 1, 1);
    }
}

std::vector<PhoneBaseInfo> CDataFactory::get_phone_list()
{
    lock::Lock locl(&phone_list_lock_);
    return phone_list_;
}

void CDataFactory::reset_phone_list(const std::vector<PhoneBaseInfo>& phone_list)
{
    lock::Lock locl(&phone_list_lock_);
    phone_list_ = phone_list;
}


