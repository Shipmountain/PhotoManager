#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

#define PROGRAM_CONF_FILE "PhoneHelper.dat"

#define PROGRAM_MAIN_WIN_NAMEA            "�ֻ�����"
#define PROGRAM_MAIN_WIN_NAMEW            L"�ֻ�����"
#define PROGRAM_UNIQUE_GUID "PHONE_HELPER_PROGRAM_UNIQUE_GUID"

#define THUMBNAIL_IMAGE_WIDTH 114
#define THUMBNAIL_IMAGE_HEIGHT 84

//�ļ�����
enum eFileType
{
    eFileApk,       //APK�ļ���android��װ���ļ���
    eFileApps,      //APPS�ļ���ƻ��ϵͳ��ִ���ļ���
    eFileCAD,       //CAD�ļ�
    eFileDoc,       //�ĵ�
    eFileEpub,      //�������ļ�
    eFileExe,       //exe��ִ���ļ�
    eFileDir,       //Ŀ¼
    eFileGeneral,   //ͨ���ļ�
    eFilePic,       //ͼƬ
    eFileIpa,       //IPA�ļ���ƻ����װ�ļ���
    eFileKey,       //KEY�ļ���ƻ����ʾ�ļ����൱��windows PowerPoint��
    eFileMusic,     //����
    eFileNumbers,   //ƻ��numbers�ļ����൱��windows excel��
    eFileOther,     //����
    eFilePages,     //ƻ��pages�ļ����൱��windows doc��
    eFilePdf,       //PDF�ļ�
    eFilePpt,       //PPT�ļ�
    eFileRar,       //ѹ���ļ�
    eFileTorrent,   //�����ļ�
    eFileTxt,       //�ı��ļ�
    eFileVideo,     //��Ƶ
    eFileVsd,       //VSD�ļ���visio�ļ���
    eFileXls,       //XLS�ļ�
};

//��������
enum eSortType
{
    eSortAse,   //����
    eSortDesc,  //����
};

//�����ֶ�
enum eSortField
{
    eSortByName,    //ͨ���ļ�������
    eSortByTime,    //ͨ���޸�ʱ������
    eSortBySize,    //ͨ���ļ���С����
};

//�ֻ�������Ϣ
struct PhoneBaseInfo
{
    PhoneBaseInfo(){};
    PhoneBaseInfo(const std::wstring& serial_no,
        const std::wstring& product,
        const std::wstring& model,
        const std::wstring& device) :serial_no_(serial_no), product_(product), model_(model), device_(device){}
    std::wstring serial_no_;
    std::wstring product_;
    std::wstring model_;
    std::wstring device_;
};

//�ļ���Ϣ
struct FileInfo
{
    FileInfo() :file_size_(0), file_type_(eFileGeneral), thumbnail_width_(0), thumbnail_height_(0), is_phone_(false), is_sync_(false), is_del_(false){}
    std::wstring file_name_;
    std::wstring file_path_;
    std::wstring modify_time_;
    std::wstring serial_no_;     //�ֻ����к�
    int          file_size_;     //�ļ���С��kb��
    eFileType    file_type_;
    std::wstring thumbnail_path_;
    int          thumbnail_width_;
    int          thumbnail_height_;
    bool         is_phone_;          //�Ƿ��ֻ��ļ�
    bool         is_sync_;           //�Ƿ�ͬ��
    bool         is_del_;            //�Ƿ�ɾ��(����ɾ������ʱ����true)
};

//���
struct Result
{
    Result(int total, int count, bool is_success, bool is_local, int curr_page, bool is_dir) :
        total_(total), count_(count), is_success_(is_success), is_local_(is_local), curr_page_(curr_page), is_dir_(is_dir){}
    Result() :total_(-1), count_(0), is_success_(false), is_local_(false), curr_page_(1){}

    int          total_;         //��¼��������
    int          count_;         //��¼����
    bool         is_success_;    //�����Ƿ�ɹ�
    bool         is_local_;      //�Ƿ񱾻�����
    int          curr_page_;     //��ǰҳ
    bool         is_dir_;        //�Ƿ�ͨ��·�����ң�ͨ��·�����ң�ͨ���ļ����Ͳ��ң�
    std::wstring dir_;           //ͨ��·������ʱ��·��
    int          file_type_;     //ͨ���ļ����Ͳ���ʱ���ļ�����
};

//��ȡ�ֻ��ļ�
struct PhoneFileInfo
{
    PhoneFileInfo() :ctrl_ptr_(NULL){};
    PhoneFileInfo(const std::wstring& file_name, std::wstring& file_path, const std::wstring& phone_sn, void* ctrl_ptr) :
        file_name_(file_name), file_path_(file_path), phone_sn_(phone_sn), ctrl_ptr_(ctrl_ptr){}
    std::wstring file_name_; //�ļ���
    std::wstring file_path_; //�ļ�·��
    std::wstring phone_sn_;  //�ֻ����к�
    void*        ctrl_ptr_;  //�ؼ�ָ��
};

//ͬ����Ϣ
struct SyncInfo
{
    std::string file_path_;
    std::string modify_time_;
    bool        is_dir_;
};

//��ʾ��Ƭ����
struct ShowPicParam
{
    ShowPicParam(bool is_phone_pic, const std::wstring& pic_name, const std::wstring& pic_path, const std::wstring& phone_sn) :
        is_phone_pic_(is_phone_pic), pic_name_(pic_name), pic_path_(pic_path), phone_sn_(phone_sn){}

    bool is_phone_pic_;
    std::wstring pic_name_;
    std::wstring pic_path_;
    std::wstring phone_sn_;
};

std::wstring get_now_time_w();
std::wstring get_now_time_ws();
std::string get_now_time();
std::wstring get_now_date_w();
std::string get_file_name(const std::string& file_path);

//���ݿ��ֶλ�ȡ
std::string get_string_field(char** data_set, int index);
int         get_int_field(char** data_set, int index);

//�ָ��ַ���
void split_string(const std::string& str, const char& separator, std::vector<std::string>& out_lst);

void split_wstring(const std::wstring& str, const wchar_t& separator, std::vector<std::wstring>& out_lst);

//��ȡMD5ֵ
std::string get_str_md5(const std::string& str);

//�ַ����滻
void string_replace(std::string& str, const char& src_ch, const char& dst_ch);

//��ȡ�ļ�����
eFileType get_file_type(const std::string& file_name);



#endif  //COMMON_H

