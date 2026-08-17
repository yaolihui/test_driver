#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
 
typedef struct{
	int integer;
	char string[12];
}RECORD;
 
#define NRECORDS (256 + 1) // 4096 + 16
 
int main()
{
	RECORD record, *mapped;
	int i, f, pagesize;
	FILE *fp;
	pagesize = sysconf(_SC_PAGESIZE);
	printf("sizeof(RECORD)=%ld, pagesize=%d\n", sizeof(RECORD), pagesize);
	
	fp = fopen("records.dat", "w+");
	for( i = 0; i < NRECORDS; i++)
	{
		record.integer = i;
		sprintf(record.string, "[RECORD-%d]", i);
		fwrite(&record, sizeof(record), 1, fp);
	}
	fclose(fp);
	
	fp = fopen("records.dat", "r+");
	fseek(fp, 43 * sizeof(record), SEEK_SET);
	fread(&record, sizeof(record), 1, fp);
	
	record.integer = 143;
	sprintf(record.string, "[RECORD-%d]", record.integer);
	fseek(fp, 43 * sizeof(record), SEEK_SET);
	fwrite(&record, sizeof(record), 1, fp);
	fclose(fp);
	
	f = open("records.dat", O_RDWR);
	mapped = (RECORD*)mmap(NULL, (NRECORDS + 10) * sizeof(record), 
	                        PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);
	printf("[0]: integer=%d, %s\n", mapped[0].integer, mapped[0].string);

	off_t size = lseek(f, 0, SEEK_END);
	printf("f:[%d], filesize:%ld\n", f, size);
	//open是系统调用，返回文件描述符。fopen是库函数，返回指针。                        
	mapped[43].integer = 343;
	sprintf(mapped[43].string, "[RECORD-%d]", mapped[43].integer);
	
	//以下追加数据无效	
	mapped[100].integer = 100;
	sprintf(mapped[100].string, "[record-%d]", mapped[100].integer);

	msync((void *) mapped, NRECORDS * sizeof(record), MS_ASYNC);
	munmap((void *)mapped, NRECORDS * sizeof(record));
	//close(f);

	//f = open("records.dat", O_RDWR);
        mapped = (RECORD*)mmap(mapped, NRECORDS * sizeof(record),
                                PROT_READ | PROT_WRITE, MAP_SHARED, f, pagesize/*必须整数倍*/);
	printf("[43]: integer=%d, string=%s\n", mapped[0].integer, mapped[0].string);
	munmap((void *)mapped, NRECORDS * sizeof(record));
	close(f);
	return 0;	
}
