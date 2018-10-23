/*************************************************************************
    > File Name: testapp.c
    > Author: baiy
    > Mail: yang01.bai@hobot.cc 
    > Created Time: 2018-07-31-20:22:49
   > Func: 
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <errno.h>


#define GLOBALMEM_MAGIC 'g'
#define GLOBALMEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)


char tmpbuf[1024] = {0};

int main(int argc, char * argv[])
{

    int opt;
    int count = 0;
    int ret = 0;

    int fd = open("/dev/globalmem", O_RDWR );
    printf("fd is %d\n",fd);
    assert(fd > 0);

    while(1){
        printf("current file offset is %#llx,start to test:\n",lseek(fd,0,SEEK_CUR));
        printf("input r/w/c/i to continue:");
        opt = getc(stdin);
        printf("input count:");
        scanf("%d",&count);
        printf("current opt is %c, count = %d\n",opt,count);
        switch(opt){
            case 'r':
                ret = read(fd,tmpbuf,count);
                break;
            case 'w':
                ret = write(fd,tmpbuf,count);
                break;
            case 'c':
                ret = lseek(fd,count,SEEK_SET);
                break;
            case 'i':
                if(count == 0) 
                    ret = ioctl(fd, GLOBALMEM_CLEAR, NULL);
                else
                    ret = -EINVAL;
            default:
                printf("not support cmd\n");
                break;
        }
        printf("opt return %d\n",ret);

        while(getc(stdin) != '\n')continue;
    }
    

    return 0;
}

