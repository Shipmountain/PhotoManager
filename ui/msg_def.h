#ifndef MSG_DEF_H
#define MSG_DEF_H

enum EWindowMsg
{
    EMsgReloadData = WM_USER + 1000,    //重新加载数据
    EMsgExchangeDataMode,               //切换数据模式
    EMsgRefreshResult,                  //刷新结果
    EMsgRefreshThumbnail,               //刷新缩略图
    EMsgRefreshList,                    //刷新列表
    EMsgRefreshDateList,                //刷新时间列表
    EMsgOpenFile,                       //打开文件
    EMsgPathForward,                    //前进
    EMsgPathBack,                       //后退
    EMsgRefresh,                        //刷新
    EMsgGotoPath,                       //跳转到指定路径
    EMsgShowDirFolderMenu,              //显示目录下的文件夹菜单
    EMsgInitPageItem,                   //初始化分页项
    EMsgGotoPage,                       //跳转到指定页
    EMsgShowSettingWin,                 //显示设置窗口
    EMsgModuleOptionChange,             //选项模块改变
    EMsgRefreshThumbnailImage,          //刷新缩略图图片
    EMsgShowPhoneListMenu,              //显示手机列表菜单
    EMsgRefreshPhoneDevice,             //刷新手机设备
    EMsgShowPic,                        //显示图片
    EMsgShowMainRightMenu,              //显示主界面右键菜单
    EMsgAddPhoneSyncFile,               //添加手机同步文件
    EMsgDelPhoneSyncFile,               //取消手机同步文件
    EMsgAddPhoneSyncFileResult,         //添加手机同步文件结果
    EMsgDelPhoneSyncFileResult,         //取消手机同步文件结果
    EMsgLocationFile,                   //定位文件
    EMsgRefreshViewBigImage,            //显示图片
    EMsgRefreshViewImageList,           //显示图片列表
    EMsgRefreshViewThumbnail,           //刷新显示缩略图
    EMsgDeleteFile,                     //删除文件
    EMsgRenameFile,                     //重命名文件
};

struct ViewThumbnailParam 
{
    ViewThumbnailParam(int index, void* image, void* dark_image):
        index_(index), image_(image), dark_image_(dark_image){}

    int     index_;
    void*   image_;
    void*   dark_image_;
};

#endif //MSG_DEF_H


