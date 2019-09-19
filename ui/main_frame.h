#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP
#include "../../DuiLib/Utils/WinImplBase.h"
#include <map>
#include <memory>
#include <log.h>
#include "../data/DataFactory.h"
#include <lock.h>
using namespace DuiLib;

class MainFrame : public WindowImplBase, Log::ModuleLog
{
private:
    enum eMainTimer
    {
        eTimerGetDevices,    //获取设备列表定时器
        eTimerFileSearch,    //文件搜索定时器
    };

    enum eListField
    {
        eFieldName,
        eFieldModifyTime,
        eFieldSize,
        eFieldPath,
    };

    enum eImageMode
    {
        eImageBig,
        eImageMiddle,
        eImageSmall,
    };

    struct HistoryDirInfo 
    {
        HistoryDirInfo(){};
        HistoryDirInfo(const std::wstring& dir_name) :dir_name_(dir_name), total_page_(1), curr_page_(1), page_pos_(1){}
        std::wstring dir_name_;      //目录名
        int          total_page_;    //总页数
        int          curr_page_;     //当前页
        int          page_pos_;      //当前页的位置
    };
   
public:

	MainFrame();
	~MainFrame();

    std::wstring get_curr_path();
public:

	LPCTSTR GetWindowClassName() const;	
	virtual void InitWindow();
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	LPCTSTR GetResourceID() const;
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam, LPARAM lParam, bool& bHandled);
    virtual LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:	

	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	void OnItemSelect(TNotifyUI& msg);
	virtual void OnClick(TNotifyUI& msg);
	void OnSelectChange(TNotifyUI& msg);
    static void add_path_menu_item(CMenuUI*  pMenu, void* params);
    static void path_menu_item_click(CControlUI* ctrl_ptr, void* params);
    static void add_phone_menu_item(CMenuUI*  pMenu, void* params);
    static void phone_menu_item_click(CControlUI* ctrl_ptr, void* params);
    static void main_menu_modify_fun(CMenuUI*  pMenu, void* params);
    static void main_menu_item_click(CControlUI* ctrl_ptr, void* params);

    void OnRButtonDown(TNotifyUI& msg);
    void OnMenuItemClick(TNotifyUI& msg);
    void OnItemDBClick(TNotifyUI& msg);
private:
    void on_msg_reload_data();
    void on_msg_exchange_data_mode();
    void on_msg_refresh_phone_device();
    void on_msg_refresh_result(WPARAM wParam);
    void on_msg_refresh_list();
    void on_msg_refresh_date_list();
    void on_msg_refresh_thumbnail();
    void on_msg_mouse_lbuttondown(WPARAM& wParam, LPARAM& lParam);
    void on_msg_mouse_hover(WPARAM& wParam, LPARAM& lParam);
    void on_msg_mouse_move(WPARAM& wParam, LPARAM& lParam);
    void on_msg_mouse_dbclick(WPARAM& wParam, LPARAM& lParam);
    void on_msg_open_file(WPARAM wParam);
    void on_msg_path_forward();
    void on_msg_path_back();
    void on_msg_goto_path();
    void on_msg_refresh();
    void on_msg_show_dir_folder_menu(WPARAM wParam, LPARAM lParam);
    void on_msg_init_page_item(WPARAM wParam, LPARAM lParam);
    void on_msg_goto_page(WPARAM wParam, LPARAM lParam);
    void on_msg_show_setting_win();
    void on_msg_device_change(WPARAM wParam, LPARAM lParam);
    void on_msg_module_option_change();
    void on_msg_refresh_thumbnail_image(WPARAM wParam, LPARAM lParam);
    void on_msg_show_phone_list_menu();
    void on_msg_mouse_rbuttondown(WPARAM wParam, LPARAM lParam);
    void on_msg_show_main_right_menu(WPARAM wParam, LPARAM lParam);
    void on_msg_add_phone_sync_file(WPARAM wParam);
    void on_msg_del_phone_sync_file(WPARAM wParam);
    void on_msg_add_phone_sync_file_result(WPARAM wParam, LPARAM lParam);
    void on_msg_del_phone_sync_file_result(WPARAM wParam, LPARAM lParam);
    void on_msg_location_file(WPARAM wParam);
    void on_msg_delete_file(WPARAM wParam);
    void on_msg_rename_file(WPARAM wParam);

    bool on_thumbnail_file_name_edt(void* param);
    bool on_list_file_name_edt(void* param);
    std::wstring get_file_pic_path(const FileInfo& file_info, eImageMode image_mode);
    void thumbnail_selected(CControlUI* ctrl_ptr, bool selected);
    void thumbnail_mouse_in(CControlUI* ctrl_ptr, bool mouse_in);
    CControlUI* get_thumbnail_by_mouse(POINT& pt);
    void open_dir(const std::wstring& file_name);
    void open_pic(const FileInfo& file_info);
    void reset_curr_path_by_dir_lst();
    void visible_forward_back_btn();
    void add_path_folder();
    bool on_path_folder_click(void* param);
    bool on_path_folder_id_click(void* param);
    void set_path_folder_id_image(CButtonUI* btn_ptr, int is_drop);
    void on_page_back();
    void on_page_forward();
    void remove_thumbnail_lst_image();
    void show_loading(bool visible);
    void reset_page_item(int total_page, int curr_page, int page_pos);
    std::wstring get_thumbnail_image_id(const std::wstring& image_name, int width, int height);
    void change_ctrl_sync_state(int index, const FileInfo& file_info);
    bool change_file_name_check(const std::wstring& new_name);
    std::wstring rename_file_in_file_list(int index, const std::wstring& new_name);
private:
    std::vector<HistoryDirInfo> history_dir_lst_;//目录访问历史列表
    std::wstring                curr_path_;//当前路径
    std::vector<std::wstring>   path_folder_lst_;//路径文件夹列表
    int                         curr_pos_;//当前路径在目录访问历史中的位置

    std::map<CControlUI*, CControlUI*> thumbnail_map_;//缩略图列表 first:lay second:image control
    CControlUI*                 curr_thumbnail_;//当前选中的缩略图
    CControlUI*                 last_move_thumbnail_;//上次鼠标移动范围内的缩略图

    bool                        need_total_numbers_;//是否需要数据的总条数
    wchar_t                     file_match_key_[10];//文件匹配关键字
    int                         match_key_len_;
    CRITICAL_SECTION            file_match_key_lock_;

    std::wstring                file_name_;
private:
    CHorizontalLayoutUI*            lay_title_ptr_;
    COptionUI*                      opt_local_ptr_;
    CControlUI*                     ctrl_local_ptr_;
    COptionUI*                      opt_phone_ptr_;
    CControlUI*                     ctrl_phone_ptr_;
    CButtonUI*                      btn_phone_name_ptr_;
    CButtonUI*                      btn_sync_ptr_;
    CButtonUI*                      btn_setting_ptr_;
    CButtonUI*                      minbtn_ptr_;
    CButtonUI*                      maxbtn_ptr_;
    CButtonUI*                      restorebtn_ptr_;
    CButtonUI*                      closebtn_ptr_;

    CVerticalLayoutUI*              lay_left_ptr_;
    CHorizontalLayoutUI*            lay_left_file_list_ptr_;
    CControlUI*                     ctrl_left_file_list_ptr_;
    COptionUI*                      opt_left_file_list_ptr_;
    CHorizontalLayoutUI*            lay_left_image_ptr_;
    CControlUI*                     ctrl_left_image_ptr_;
    COptionUI*                      opt_left_image_ptr_;
    CHorizontalLayoutUI*            lay_left_vedio_ptr_;
    CControlUI*                     ctrl_left_vedio_ptr_;
    COptionUI*                      opt_left_vedio_ptr_;
    CHorizontalLayoutUI*            lay_left_music_ptr_;
    CControlUI*                     ctrl_left_music_ptr_;
    COptionUI*                      opt_left_music_ptr_;
    CHorizontalLayoutUI*            lay_left_doc_ptr_;
    CControlUI*                     ctrl_left_doc_ptr_;
    COptionUI*                      opt_left_doc_ptr_;
    CHorizontalLayoutUI*            lay_left_other_ptr_;
    CControlUI*                     ctrl_left_other_ptr_;
    COptionUI*                      opt_left_other_ptr_;

    CVerticalLayoutUI*              lay_right_ptr_;
    CButtonUI*                      btn_right_back_ptr_;
    CButtonUI*                      btn_right_forward_ptr_;
    CButtonUI*                      btn_right_flush_ptr_;
    CHorizontalLayoutUI*            lay_right_dir_path_ptr_;
    COptionUI*                      opt_right_switch_list_ptr_;
    COptionUI*                      opt_right_switch_thumbnail_ptr_;
    COptionUI*                      opt_right_switch_date_ptr_;

    CVerticalLayoutUI*              lay_right_thumbnail_ptr_;
    CTileLayoutUI*                  lay_right_thumbnail_content_ptr_;

    CListUI*                        lst_right_list_ptr_;

    CVerticalLayoutUI*              lay_right_date_ptr_;
    CVerticalLayoutUI*              lay_right_date_content_ptr_;

    CLabelUI*                       lbl_right_item_count_ptr_;
    CHorizontalLayoutUI*            lay_right_page_ptr_;
    CButtonUI*                      btn_page_forward_ptr_;
    COptionUI*                      opt_page_1_ptr_;
    COptionUI*                      opt_page_2_ptr_;
    COptionUI*                      opt_page_3_ptr_;
    COptionUI*                      opt_page_4_ptr_;
    COptionUI*                      opt_page_5_ptr_;
    COptionUI*                      opt_page_6_ptr_;
    CButtonUI*                      btn_page_back_ptr_;
    CLabelUI*                       lbl_page_total_ptr_;
    CGifUI*                         gif_loading_ptr_;
};

#endif // MAINFRAME_HPP