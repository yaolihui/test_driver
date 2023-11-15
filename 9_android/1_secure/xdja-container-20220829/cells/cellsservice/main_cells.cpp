#define LOG_TAG "CellsService"

#include <fcntl.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <binder/BinderService.h>
#include <CellsPrivateService.h>

#include "../cells/cellnet.h"

using namespace android;

static int get_vm_index()
{
	int index = 0;
	char value[PROPERTY_VALUE_MAX] = {0};
	property_get("ro.boot.vm.name", value, "");
	sscanf(value, "cell%d", &index);

	ALOGD("VM Index = %d", index);
	return index;
}

int main(int /*argc*/, char** /*argv*/)
{
	ALOGI("GuiExt service start...");

	char value[PROPERTY_VALUE_MAX];
	property_get("ro.boot.vm", value, "1");
	if((strcmp(value, "0") == 0)){
		//property_set("persist.sys.exit", "0");
		//property_set("persist.sys.active", "");

		//property_set("ctl.stop", "vendor.qcrild");
		//property_set("ctl.stop", "vendor.qcrild2");
		//property_set("ctl.stop", "vendor.qcrild3");
		//property_set("ctl.stop", "vendor.adsprpcd");
		//property_set("ctl.stop", "vendor.sensors");

		property_set("ctl.start", "cellsstart");
	}else{
		//property_set("ctl.stop", "vendor.qcrild");
		//property_set("ctl.stop", "vendor.qcrild2");
		//property_set("ctl.stop", "vendor.qcrild3");
		//property_set("ctl.stop", "vendor.adsprpcd");
		//property_set("ctl.stop", "vendor.sensors");
		//property_set("ctl.stop", "adbd");
		//property_set("persist.sys.exit", "1");
		
		int i = get_vm_index();
		if(i == 1){
			property_set("persist.sys.cells.netagent", "cell1");
		}else if (i == 2){
			property_set("persist.sys.cells.netagent", "cell2");
		}
	}

	CellsPrivateService::publishAndJoinThreadPool(true);

	ProcessState::self()->setThreadPoolMaxThreadCount(4);

	ALOGD("Cells service exit...");
    return 0;
}
