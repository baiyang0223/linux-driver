/*************************************************************************
    > File Name: baiyang_second.c
    > Author: baiy
    > Mail: yang01.bai@hobot.cc 
    > Created Time: 2018年07月11日 星期三 09时54分24秒
	> Func: 内核符号导出使用
 ************************************************************************/
 
#include <linux/init.h>
#include <linux/module.h>
#include "baiyang_hello.h"  //depend on baiyang_hello1.ko




static int __init second_init(void)
{
    printk("%s-%d,test driver init,extern_num = %#x\n",__func__,__LINE__,extern_num);
	print_hello();

    return 0;
}
static void __exit second_exit(void)
{
    printk("%s-%d,test driver exit\n",__func__,__LINE__);
	print_bye();
}


		
module_init(second_init);
module_exit(second_exit); //修饰入口和出口函数
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("test driver symbol export");
MODULE_AUTHOR("baiyang <baiyang0223@163.com>");
MODULE_VERSION("1.0.0.0");
