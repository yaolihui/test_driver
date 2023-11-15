#define LOG_TAG "CELLSSTART"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <binder/BinderService.h>
#include <android-base/properties.h>
#include "ICellsPrivateService.h"

using namespace android;

int main(int /*argc*/, char** /*argv*/)
{
	property_set("persist.sys.ui.exit", "1");
	property_set("persist.sys.iw.wlan", "cell1");

	const sp<IServiceManager> sm = defaultServiceManager();
	sp<ICellsPrivateService> pCellsPrivateService = NULL;
	if (sm != NULL)
	{
		sp<IBinder> binder = sm->checkService(String16("CellsPrivateService"));
        while(binder == NULL){
            sleep(1);

            binder = sm->checkService(String16("CellsPrivateService"));
        }

        pCellsPrivateService = interface_cast<ICellsPrivateService>(binder);
        if(pCellsPrivateService == NULL){
            ALOGE("could not get service CellsPrivateService");
            return 0;
        }
	}

	sleep(1);

	if(android::base::GetProperty("persist.sys.cell1.init", "0") == std::string("0"))
	{
		ALOGD("Start cell1");
		pCellsPrivateService->startCellsVM(android::String16("cell1"));
		ALOGD("Started cell1");
	}

	property_set("persist.sys.active", "cell1");

	if(android::base::GetProperty("persist.sys.cell2.init", "0") == std::string("0"))
	{
		ALOGD("Start cell2");
		pCellsPrivateService->startCellsVM(android::String16("cell2"));
		ALOGD("Started cell2");
	}

	while(android::base::GetProperty("persist.sys.cell1.init", "0") == std::string("0"))
	{
		sleep(3);
	}

	ALOGD("Switch cell1");
	pCellsPrivateService->switchCellsVM(android::String16("cell1"));
	ALOGD("Switched cell1");

    return 0;
}
