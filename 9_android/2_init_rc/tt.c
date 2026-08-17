#include <stdio.h>
#include <cutils/log.h>

//extern int sleep();
int main(void)
{
    while(1) {
        ALOGI("~~~~~~~~~~~~~ hello world \n");
        printf("------------ hello world!!\n");
        sleep(1);
    }
	return 0;
};