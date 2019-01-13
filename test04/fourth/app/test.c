/*************************************************************************
    > File Name: test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2019年01月13日 星期日 19时34分17秒
	> Func: 
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>


#define DEVICE_NAME "/dev/testioctl"

#define MAGIC_NUM 'G'
#define TEST_01 _IO(MAGIC_NUM,1)
#define TEST_02 _IOR(MAGIC_NUM,2)
#define TEST_03 _IOW(MAGIC_NUM,3)
#define TEST_04 _IORW(MAGIC_NUM,4)
#define TEST_MAX _IO(MAGIC_NUM,5)




int main(int argc, char * argv[])
{
	int opt;
	int val;
	int ret;
	char * buf = malloc(GLOBALMEM_SIZE);
	assert(buf != NULL);
	int fd = open(DEVICE_NAME,O_RDWR);
	assert(fd > 0);

	ret = ioctl(fd,TEST_01);
	printf("Ioctl test01 ret %d\n", ret);
	ret = ioctl(fd,TEST_02);
	printf("Ioctl test02 ret %d\n", ret);
	ret = ioctl(fd,TEST_03);
	printf("Ioctl test03 ret %d\n", ret);
	ret = ioctl(fd,TEST_04);
	printf("Ioctl test04 ret %d\n", ret);


	close(fd);

	return 0;
}
