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
    //��ʼ��
    bool init();

    void set_local_storage_path(const std::string& path);

    std::vector<PhoneBaseInfo> get_phone_list();

    void reset_phone_list(const std::vector<PhoneBaseInfo>& phone_list);
public:
    std::string                 program_path_;  //����·��
    std::string                 tmp_storage_path_;//��ʱ�洢·��
    std::string                 view_pic_store_path_;//��ʾͼƬ�洢·��
    std::string                 local_storage_path_;//���ش洢·��
    int                         max_num_;       //һҳ�����ʾ�ļ�����
    int                         max_num_date_;  //������ͼ�����ʾ����
    eSortType                   sort_type_;     //��������
    eSortField                  sort_field_;    //�����ֶ�

    lock::Mutex                 file_lst_lock_; //�ļ��б���
    std::vector<FileInfo>*      file_lst_;      //�ļ��б�
    std::string                 save_as_path_;  //���Ϊ·��
private:
    
    std::vector<PhoneBaseInfo>  phone_list_;  //�ֻ��б�
    lock::Mutex                 phone_list_lock_;

};

typedef comm::Singleton<CDataFactory> theDataFactory;
#endif

