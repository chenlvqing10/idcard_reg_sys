#include "socketthr.h"


SocketThr::~SocketThr(){
    emit sigErr("线程退出");
}

void SocketThr::setIP(const QString &ipstr){
    this->ipstr = ipstr;
}

void SocketThr::exitthr(){
    isRun = false;
}

void SocketThr::loadlib(QLibrary *lib)
{
    //加载动态库生成图片
     QLibrary *lib_wlt = NULL;
     QLibrary *lib_usb_1_0 = NULL;
     //写清楚库的路径，如果放在当前工程的目录下，路径为./libhello.so
     #ifdef __FCU2302__ARM64
         lib_wlt = new QLibrary("/opt/Qt/5.15.1/gcc_64/lib/libwlt.so");
         lib_usb_1_0 = new QLibrary("/opt/Qt/5.15.1/gcc_64/lib/libusb-1.0.so");
         lib = new QLibrary("/opt/Qt/5.15.1/gcc_64/lib/libSynReader64.so");
     #else

     #endif
     //加载动态库
     lib_wlt->load();
     if (!lib_wlt->isLoaded())
     {
         printf("load libwlt.so failed!\n");
     }
     else
     {
         printf("load lib_wlt.so successed!\n");
      }

     lib_usb_1_0->load();
    if (!lib_usb_1_0->isLoaded())
    {
        printf("load libusb_1_0.so failed!\n");
    }
    else
    {
        printf("load libusb_1_0.so successed!\n");
     }

    lib->load();
   if (!lib->isLoaded())
   {
       printf("load libSynReader.so failed!\n");
       qDebug()<< "error string:" << lib->errorString() << Qt::endl;
   }
   else
   {
       printf("load libSynReader.so successed!\n");

   }

   /* save some images not only one image so need to get the bmp files' numbers in this directory*/
   /* when using,need to change it for case*/
   const char* path = "/home/forlinx/chenlvqing/qt/QtCLient/image";
   int filenumer = getBmpCount(path);
   char filename[100];
   if(flag_same == false)
        sprintf(filename,"/home/forlinx/chenlvqing/qt/QtCLient/image/idcard%d.bmp",filenumer + 1);
   else {
        //sprintf(filename,"/home/forlinx/chenlvqing/qt/QtCLient/image/idcard%d.bmp",filenumer);
       //卸载库
       lib->unload();
       lib_wlt->unload();
       lib_usb_1_0->unload();
       //emit sigShowimage(filename);
       return;
   }

   qDebug()<< "filename:" << filename<< Qt::endl;

   //定义函数指针
   typedef void (*Fun)();
   //resolve得到库中函数地址
   Fun fun_WltToBmp = (Fun)lib->resolve("saveWlt2Bmp");
   if (fun_WltToBmp)
   {
       int result = saveWlt2Bmp(idcard_bitmap.data(),  filename);//解码照片
       if (result == 1) {
           printf("saveWlt2Bmp success\r\n");
       } else {
           printf("saveWlt2Bmp(id_bitmap) = %d\r\n", result);
       }
   }
    qDebug()<< ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << Qt::endl;

   //卸载库
   lib->unload();
   lib_wlt->unload();
   lib_usb_1_0->unload();

    emit sigShowimage(filename);

}

int SocketThr::getBmpCount(const char *path)
{
    QDir *dir = new QDir(path);
    dir->setFilter(QDir::Files);
    QStringList filter;
    filter<<"*.bmp";
    dir->setNameFilters(filter);
    QFileInfoList list=dir->entryInfoList();
    QFileInfo fileInfo;
    QStringList filelist;
    foreach (fileInfo, list)
        {
            filelist << fileInfo.fileName();
        }
    int count = filelist.count();

    return count;
}

