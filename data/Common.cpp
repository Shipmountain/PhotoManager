#include "Common.h"
#include <md5.h>
#include <system.h>
#include <time.h>

static std::map<std::string, eFileType> type_map =
{
    //apk
    { "APK", eFileApk },
    //APP
    { "APP", eFileApps },
    //CAD
    { "DWG", eFileCAD },
    { "DWT", eFileCAD },
    //DOC
    { "DOC", eFileDoc },
    { "DOCX", eFileDoc },
    { "WPS", eFileDoc },
    { "WPT", eFileDoc },
    //EPUB
    { "EPUB", eFileEpub },
    //EXE
    { "EXE", eFileExe },
    //GENERAL
    { "H", eFileGeneral },
    { "C", eFileGeneral },
    { "HPP", eFileGeneral },
    { "CPP", eFileGeneral },
    { "DLL", eFileGeneral },
    { "LIB", eFileGeneral },
    { "LOG", eFileGeneral },
    { "INI", eFileGeneral },
    //PIC
    { "PNG", eFilePic },
    { "JPG", eFilePic },
    { "BMP", eFilePic },
    { "JPEG", eFilePic },
    { "GIF", eFilePic },
    //IPA
    { "IPA", eFileIpa },
    //KEY
    { "KEY", eFileKey},
    //MUSIC
    { "MP3", eFileMusic },
    { "WMA", eFileMusic },
    { "WAV", eFileMusic },
    { "MIDI", eFileMusic },
    { "AIF", eFileMusic },
    { "AIFC", eFileMusic },
    { "AIFF", eFileMusic },
    { "CMF", eFileMusic },
    { "VOC", eFileMusic },
    //numbers
    { "NUMBERS", eFileNumbers },
    //PAGES
    { "PAGES", eFilePages },
    //PDF
    { "PDF", eFilePdf },
    //ppt
    { "PPTX", eFilePpt },
    { "PPT", eFilePpt },
    { "DPS", eFilePpt },
    { "DPT", eFilePpt },
    { "WPP", eFilePpt },
    //RAR
    { "RAR", eFileRar },
    { "ZIP", eFileRar },
    { "Z", eFileRar },
    { "TAR", eFileRar },
    { "7Z", eFileRar },
    { "GZ", eFileRar },
    { "JAR", eFileRar },
    { "ISO", eFileRar },
    //种子文件
    { "TORRENT", eFileTorrent },
    //文本文件
    { "TXT", eFileTxt },
    //视频
    { "AVI", eFileVideo },
    { "RMVB", eFileVideo },
    { "RM", eFileVideo },
    { "ASF", eFileVideo },
    { "ASP", eFileVideo },
    { "DIVX", eFileVideo },
    { "MPG", eFileVideo },
    { "MPEG", eFileVideo },
    { "MPE", eFileVideo },
    { "WMV", eFileVideo },
    { "MP4", eFileVideo },
    { "MKV", eFileVideo },
    { "VOB", eFileVideo },
    { "3GP", eFileVideo },
    //VSD文件（visio文件）
    { "VSD", eFileVsd },
    { "VSS", eFileVsd },
    { "VST", eFileVsd },
    { "VDX", eFileVsd },
    { "VSX", eFileVsd },
    { "VTX", eFileVsd },
    //xls文件
    { "XLS", eFileXls },
    { "XLSX", eFileXls },
    { "ET", eFileXls },
    { "ETT", eFileXls },
    
};

std::wstring get_now_time_w()
{
    time_t t;
    struct tm * lt;
    time(&t);
    lt = localtime(&t);
    wchar_t tmp[50] = { 0 };
    wsprintf(tmp, L"%d-%02d-%02d %02d:%02d:%02d",
        lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
        lt->tm_hour, lt->tm_min, lt->tm_sec);
    return std::wstring(tmp);
}
std::wstring get_now_time_ws()
{
    time_t t;
    struct tm * lt;
    time(&t);
    lt = localtime(&t);
    wchar_t tmp[50] = { 0 };
    wsprintf(tmp, L"%02d:%02d:%02d",
        lt->tm_hour, lt->tm_min, lt->tm_sec);
    return std::wstring(tmp);
}

std::string get_now_time()
{
    time_t t;
    struct tm * lt;
    time(&t);
    lt = localtime(&t);
    char tmp[50] = { 0 };
    sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d",
        lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
        lt->tm_hour, lt->tm_min, lt->tm_sec);
    return std::string(tmp);
}

std::wstring get_now_date_w()
{
    time_t t;
    struct tm * lt;
    time(&t);
    lt = localtime(&t);
    wchar_t tmp[50] = { 0 };
    wsprintf(tmp, L"%d-%02d-%02d",
        lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
    return std::wstring(tmp);
}


std::string get_file_name(const std::string& file_path)
{
    std::string path = file_path;
    char* p = (char*)path.c_str();
    while (*p)
    {
        if (*p == '/')
        {
            *p = '\\';
        }
        ++p;
    }

    int pos = path.rfind('\\');
    if (pos == std::string::npos)
    {
        return "";
    }

    std::string file = path.substr(pos + 1, path.size() - pos - 1);
    return file;
}

std::string get_string_field(char** data_set, int index)
{
    std::string result;
    if (NULL != data_set[index])
    {
        result = data_set[index];
    }
    return result;
}

int         get_int_field(char** data_set, int index)
{
    int result = 0;
    if (NULL != data_set[index])
    {
        result = atoi(data_set[index]);
    }
    return result;
}

void split_string(const std::string& str, const char& separator, std::vector<std::string>& out_lst)
{
    if (str.empty())
    {
        return;
    }

    int n = str.find(separator);
    int offset = 0;
    std::string tmp;
    while (n != std::string::npos)
    {
        out_lst.push_back(str.substr(offset, n - offset));
        offset = n + 1;
        n = str.find(separator, offset);
    }

    if (offset < str.size())
    {
        out_lst.push_back(str.substr(offset, str.size() - offset));
    }
}

void split_wstring(const std::wstring& str, const wchar_t& separator, std::vector<std::wstring>& out_lst)
{
    if (str.empty())
    {
        return;
    }

    int n = str.find(separator);
    int offset = 0;
    std::wstring tmp;
    while (n != std::wstring::npos)
    {
        out_lst.push_back(str.substr(offset, n - offset));
        offset = n + 1;
        n = str.find(separator, offset);
    }

    if (offset < str.size())
    {
        out_lst.push_back(str.substr(offset, str.size() - offset));
    }
}

std::string get_str_md5(const std::string& str)
{
    std::string md5;
    md5.resize(32, '\0');
    TransTo32ByteMd5((char*)str.c_str(), str.size(), (char*)md5.data());
    return std::move(md5);
}

void string_replace(std::string& str, const char& src_ch, const char& dst_ch)
{
    char* p = (char*)str.data();
    while (*p)
    {
        if (src_ch == *p)
        {
            *p = dst_ch;
        }
        ++p;
    }
}

eFileType get_file_type(const std::string& file_name)
{
    eFileType file_type = eFileOther;
    int n = file_name.find_last_of('.');
    if (n != std::string::npos)
    {
        std::string suffix = file_name.substr(n+1);
        char* p = (char*)suffix.data();
        while (*p)
        {
            if (*p >= 'a' && *p <= 'z')
            {
                *p = *p - 32;
            }
            ++p;
        }

        auto itor = type_map.find(suffix);
        if (itor != type_map.end())
        {
            return itor->second;
        }
    }
    else
    {
        file_type = eFileOther;
    }

    return file_type;
}


