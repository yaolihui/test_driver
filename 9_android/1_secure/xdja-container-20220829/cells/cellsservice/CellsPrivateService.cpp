#define LOG_TAG "CELLSSERVICE"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include "CellsPrivateService.h"
#include "../cells/cellnet.h"

#include <binder/IServiceManager.h>
#include <gui/ISurfaceComposer.h>
#include <utils/String16.h>

#include <android/os/IPowerManager.h>
#include <powermanager/PowerManager.h>

namespace android {

#define SYSTEMPRIVATE_LOGV(x, ...) ALOGV("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGD(x, ...) ALOGD("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGI(x, ...) ALOGI("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGW(x, ...) ALOGW("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGE(x, ...) ALOGE("[CellsPrivate] " x, ##__VA_ARGS__)

#define WAKE_REASON_POWER_BUTTON 1

CellsPrivateService::CellsPrivateService()
{
}

CellsPrivateService::~CellsPrivateService()
{
}

int CellsPrivateService::isInCellstar()
{
    return mtar_pthread_t;
}

void CellsPrivateService::setCellstaring()
{
    mtar_pthread_t = 1;
}

void CellsPrivateService::setCellstared()
{
    mtar_pthread_t = 0;
}

void CellsPrivateService::startCellstar()
{

}

status_t CellsPrivateService::setProperty(const String16& name,const String16& value)
{
    SYSTEMPRIVATE_LOGD("SETPROPERTY arg %s %s", String8(name).string(), String8(value).string());
    status_t result = property_set(String8(name).string(), String8(value).string());
    SYSTEMPRIVATE_LOGD("SETPROPERTY result = %d", result);
    return result;
}

status_t CellsPrivateService::startCellsVM(const String16& name)
{
    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc start %s",String8(name).string());
    SYSTEMPRIVATE_LOGD("STARTCELLSVM cmd = %s", cmd);
    system(cmd);
    return NO_ERROR;
}

status_t CellsPrivateService::stopCellsVM(const String16& name)
{
    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc stop %s",String8(name).string());
    SYSTEMPRIVATE_LOGD("STOPCELLSVM cmd = %s", cmd);
    system(cmd);
    return NO_ERROR;
}

status_t CellsPrivateService::cellsSwitchVM(const String16& name)
{
    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc switch %s",String8(name).string());
    SYSTEMPRIVATE_LOGD("CELLSSWITCHVM cmd = %s", cmd);
    system(cmd);
    property_set("persist.sys.active", String8(name).string());
    return NO_ERROR;
}

status_t CellsPrivateService::cellsSwitchHOST(const String16& /*name*/)
{
    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc switch host");
    SYSTEMPRIVATE_LOGD("CELLSSWITCHHOST cmd = %s", cmd);
    system(cmd);
    property_set("persist.sys.active", "");
    return NO_ERROR;
}

static void* gotosleep(void* /*o*/)
{
    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::os::IPowerManager> mPowerManager = 
            android::interface_cast<android::os::IPowerManager>(sm->checkService(android::String16("power")));
        if(mPowerManager != NULL){
            mPowerManager->goToSleep(long(ns2ms(systemTime())),android::os::IPowerManager::GO_TO_SLEEP_REASON_POWER_BUTTON,0);
        }
    }

    ALOGD("BACK SYSTEM go to sleep...");

    return (void*)0;
};

static void create_gotosleep_pthread(void)
{
    int ret;
    pthread_t daemon_thread;
    
    /* Start listening for connections in a new thread */
    ret = pthread_create(&daemon_thread, NULL,gotosleep, NULL);
    if (ret) {
        ALOGE("create_gotosleep_pthread err: %s", strerror(errno));
    }
};

status_t CellsPrivateService::switchCellsVM(const String16& name)
{
    int i = 0;
    char curname[PROPERTY_VALUE_MAX] = {0};

    property_get("ro.boot.vm.name", curname, "");

    ALOGD("switchCellsVM: curname=%s , name=%s", curname, String8(name).string());

    if(strcmp(curname, String8(name).string()) == 0){
        return 0;
    }

    if(strcmp(curname, "") == 0 && strcmp(String8(name).string(), "host") == 0){
        return 0;
    }

    if(strcmp(String8(name).string(), "host") != 0){
        if(strcmp(curname, "") == 0){
            if(isVMSystemReady(name) == 0)
                return 0;
        }

        android::sp<android::IServiceManager> sm = android::initdefaultServiceManager();
        android::sp<android::ICellsPrivateService> mCellsPrivateService = 
        android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
        if(mCellsPrivateService != NULL && mCellsPrivateService->isVMSystemReady(name) == 0){
            return 0;
        }
    }

    if(strcmp(curname, "") == 0){
        exitHost(android::String16("host"));
    }else{
        exitCell(android::String16(curname));
    }

    if(strcmp(String8(name).string(), "host") == 0)
    {
        {
            android::sp<android::IServiceManager> sm = android::initdefaultServiceManager();
            android::sp<android::ICellsPrivateService> mCellsPrivateService = 
            android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
            if(mCellsPrivateService != NULL){
                mCellsPrivateService->cellsSwitchHOST(android::String16("host"));
            }
        }
    
        {
            android::sp<android::IServiceManager> sm = android::initdefaultServiceManager();
            android::sp<android::ICellsPrivateService> mCellsPrivateService = 
            android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
            if(mCellsPrivateService != NULL){
                mCellsPrivateService->enterHost(android::String16("host"));
            }
        }
    }
    else
    {
        {
            android::sp<android::IServiceManager> sm = android::initdefaultServiceManager();
            android::sp<android::ICellsPrivateService> mCellsPrivateService = 
            android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
            if(mCellsPrivateService != NULL){
                mCellsPrivateService->cellsSwitchVM(name);
            }
        }
    
        {
            sscanf(String8(name).string(), "cell%d",&i);
            android::sp<android::IServiceManager> sm = android::OtherServiceManager(i);
            android::sp<android::ICellsPrivateService> mCellsPrivateService = 
                android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
            if(mCellsPrivateService != NULL){
                mCellsPrivateService->enterCell(name);
            }else{
                SYSTEMPRIVATE_LOGD("OtherServiceManager = 0");
            }
        }
    }

    {
        create_gotosleep_pthread();
    }

    SYSTEMPRIVATE_LOGD("SWITCHCELLSVM result = %d", 0);
    return NO_ERROR;
}

status_t CellsPrivateService::enterHost(const String16& /*name*/)
{
    {
        property_set("persist.sys.exit", "0");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::os::IPowerManager> mPowerManager = 
            android::interface_cast<android::os::IPowerManager>(sm->checkService(android::String16("power")));
        if(mPowerManager != NULL){
            mPowerManager->wakeUp(long(ns2ms(systemTime())),WAKE_REASON_POWER_BUTTON,
                            android::String16("enter_self"),android::String16("CellsPrivateService"));
        }else{
            SYSTEMPRIVATE_LOGD("mPowerManager = 0");
        }
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->enterSelf();
        }
    }

    {
        property_set("ctl.restart", "adbd");
    }

    {
        //property_set("ctl.restart", "vendor.qcrild");
    }

    {
        //startCellstar();
    }

    SYSTEMPRIVATE_LOGD("ENTERHOST result = %d", 0);
    return NO_ERROR;
}

status_t CellsPrivateService::exitHost(const String16& /*name*/)
{
    {
        property_set("persist.sys.exit", "1");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->exitSelf();
        }
    }

    {
        property_set("ctl.stop", "adbd");
    }

    {
        //property_set("ctl.stop", "vendor.qcrild");
    }

    SYSTEMPRIVATE_LOGD("EXITHOST result = %d", 0);
    return NO_ERROR;
}

static void write_vm_exit(bool bexit){
    int vmfd = open("/.cell",O_WRONLY);
    if(vmfd>=0){
        if(bexit)
            write(vmfd,"1",strlen("1"));
        else
            write(vmfd,"0",strlen("0"));
        close(vmfd);
    }
}

status_t CellsPrivateService::enterCell(const String16& /*name*/)
{
    {
        write_vm_exit(false);
        property_set("persist.sys.exit", "0");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::os::IPowerManager> mPowerManager = 
            android::interface_cast<android::os::IPowerManager>(sm->checkService(android::String16("power")));
        if(mPowerManager != NULL){
            mPowerManager->wakeUp(long(ns2ms(systemTime())),WAKE_REASON_POWER_BUTTON,
                            android::String16("enter_self"),android::String16("CellsPrivateService"));
        }else{
            SYSTEMPRIVATE_LOGD("mPowerManager = 0");
        }
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->enterSelf();
        }
    }

    {
        property_set("ctl.restart", "adbd");
    }

    {
        //property_set("ctl.restart", "vendor.qcrild");
    }

    SYSTEMPRIVATE_LOGD("ENTERCELL result = %d", 0);
    return NO_ERROR;
}

status_t CellsPrivateService::exitCell(const String16& /*name*/)
{
    {
        write_vm_exit(true);
        property_set("persist.sys.exit", "1");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->exitSelf();
        }
    }

    {
        property_set("ctl.stop", "adbd");
    }

    {
        //property_set("ctl.stop", "vendor.qcrild");
    }

    SYSTEMPRIVATE_LOGD("EXITCELL result = %d", 0);
    return NO_ERROR;
}

status_t CellsPrivateService::uploadCellsVM(const String16& /*name*/)
{
    return NO_ERROR;
}

status_t CellsPrivateService::downloadCellsVM(const String16& /*name*/)
{
    return NO_ERROR;
}

status_t CellsPrivateService::untarCellsVM(const String16& /*name*/)
{
    return NO_ERROR;
}

status_t CellsPrivateService::tarCellsVM(const String16& /*name*/)
{
    return NO_ERROR;
}

status_t CellsPrivateService::sendCellsVM(const String16& /*path*/, const String16& /*address*/)
{
    return NO_ERROR;
}

status_t CellsPrivateService::vmSystemReady(const String16& name)
{
    char pname[PATH_MAX] = {0};
    int i = 0;

    sprintf(pname, "persist.sys.%s.init",  String8(name).string());
    property_set(pname, "1");

    property_set("ctl.restart", "adbd");

    sscanf(String8(name).string(), "cell%d",&i);
    starttether(i);

    //chown("/dev/sg1", 1000, 1000);

    SYSTEMPRIVATE_LOGD("SYSTEMREADY name = %s", String8(name).string());
    return NO_ERROR;
}

status_t CellsPrivateService::isVMSystemReady(const String16& name)
{
    char pname[PATH_MAX] = {0};
    char value[PROPERTY_VALUE_MAX] = {0};

    status_t ret = 0;
    if(strcmp(String8(name).string(), "host") == 0){
        ret = 1;
    }

    sprintf(pname, "persist.sys.%s.init",  String8(name).string());
    property_get(pname, value, "0");
    if(strcmp(String8(value).string(), "1") == 0){
        ret = 1;
    }

    SYSTEMPRIVATE_LOGD("ISVMSYSTEMREADY name = %s ret = %d", String8(name).string(), ret);
    return ret;
}

};
