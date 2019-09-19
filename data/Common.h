#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

#define PROGRAM_CONF_FILE "PhoneHelper.dat"

#define PROGRAM_MAIN_WIN_NAMEA            "手机助手"
#define PROGRAM_MAIN_WIN_NAMEW            L"手机助手"
#define PROGRAM_UNIQUE_GUID "PHONE_HELPER_PROGRAM_UNIQUE_GUID"

#define THUMBNAIL_IMAGE_WIDTH 114
#define THUMBNAIL_IMAGE_HEIGHT 84

//文件类型
enum eFileType
{
    eFileApk,       //APK文件（android安装包文件）
    eFileApps,      //APPS文件（苹果系统可执行文件）
    eFileCAD,       //CAD文件
    eFileDoc,       //文档
    eFileEpub,      //电子书文件
    eFileExe,       //exe可执行文件
    eFileDir,       //目录
    eFileGeneral,   //通用文件
    eFilePic,       //图片
    eFileIpa,       //IPA文件（苹果安装文件）
    eFileKey,       //KEY文件（苹果演示文件，相当于windows PowerPoint）
    eFileMusic,     //音乐
    eFileNumbers,   //苹果numbers文件（相当于windows excel）
    eFileOther,     //其他
    eFilePages,     //苹果pages文件（相当于windows doc）
    eFilePdf,       //PDF文件
    eFilePpt,       //PPT文件
    eFileRar,       //压缩文件
    eFileTorrent,   //种子文件
    eFileTxt,       //文本文件
    eFileVideo,     //视频
    eFileVsd,       //VSD文件（visio文件）
    eFileXls,       //XLS文件
};

//排序类型
enum eSortType
{
    eSortAse,   //升序
    eSortDesc,  //降序
};

//排序字段
enum eSortField
{
    eSortByName,    //通过文件名排序
    eSortByTime,    //通过修改时间排序
    eSortBySize,    //通过文件大小排序
};

//手机基本信息
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

//文件信息
struct FileInfo
{
    FileInfo() :file_size_(0), file_type_(eFileGeneral), thumbnail_width_(0), thumbnail_height_(0), is_phone_(false), is_sync_(false), is_del_(false){}
    std::wstring file_name_;
    std::wstring file_path_;
    std::wstring modify_time_;
    std::wstring serial_no_;     //手机序列号
    int          file_size_;     //文件大小（kb）
    eFileType    file_type_;
    std::wstring thumbnail_path_;
    int          thumbnail_width_;
    int          thumbnail_height_;
    bool         is_phone_;          //是否手机文件
    bool         is_sync_;           //是否同步
    bool         is_del_;            //是否删除(界面删除操作时，置true)
};

//结果
struct Result
{
    Result(int total, int count, bool is_success, bool is_local, int curr_page, bool is_dir) :
        total_(total), count_(count), is_success_(is_success), is_local_(is_local), curr_page_(curr_page), is_dir_(is_dir){}
    Result() :total_(-1), count_(0), is_success_(false), is_local_(false), curr_page_(1){}

    int          total_;         //记录的总条数
    int          count_;         //记录条数
    bool         is_success_;    //查找是否成功
    bool         is_local_;      //是否本机搜索
    int          curr_page_;     //当前页
    bool         is_dir_;        //是否通过路径查找（通过路径查找，通过文件类型查找）
    std::wstring dir_;           //通过路径查找时的路径
    int          file_type_;     //通过文件类型查找时的文件类型
};

//获取手机文件
struct PhoneFileInfo
{
    PhoneFileInfo() :ctrl_ptr_(NULL){};
    PhoneFileInfo(const std::wstring& file_name, std::wstring& file_path, const std::wstring& phone_sn, void* ctrl_ptr) :
        file_name_(file_name), file_path_(file_path), phone_sn_(phone_sn), ctrl_ptr_(ctrl_ptr){}
    std::wstring file_name_; //文件名
    std::wstring file_path_; //文件路径
    std::wstring phone_sn_;  //手机序列号
    void*        ctrl_ptr_;  //控件指针
};

//同步信息
struct SyncInfo
{
    std::string file_path_;
    std::string modify_time_;
    bool        is_dir_;
};

//显示照片参数
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

//数据库字段获取
std::string get_string_field(char** data_set, int index);
int         get_int_field(char** data_set, int index);

//分隔字符串
void split_string(const std::string& str, const char& separator, std::vector<std::string>& out_lst);

void split_wstring(const std::wstring& str, const wchar_t& separator, std::vector<std::wstring>& out_lst);

//获取MD5值
std::string get_str_md5(const std::string& str);

//字符串替换
void string_replace(std::string& str, const char& src_ch, const char& dst_ch);

//获取文件类型
eFileType get_file_type(const std::string& file_name);



#endif  //COMMON_H

