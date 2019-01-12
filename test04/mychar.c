/*************************************************************************
    > File Name: mychar.c
    > Author: baiy
    > Mail: yang01.bai@hobot.cc 
    > Created Time: 2018-07-27-16:56:21
	> Func: 
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>


#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/pgtable.h>
#include <asm/io.h>


//文件操作相关结构体：
//struct file :描述一个已打开的文件 linux/fs.h
//struct file_operations: vfs与设备的接口函数，linux/fs.h
//struct cdev: 描述一个字符设备
//struct inode: 描述一个文件节点


struct class * class;
struct device * device;
static int major = 0;
module_param(major,int,0644);
MODULE_PARM_DESC(major,"major num");

static int minor = 0;
module_param(minor,int,0644);
MODULE_PARM_DESC(minor,"minor num");
#define BUF_SIZE 1024

#define GLOBALMEM_MAGIC 'g'
#define GLOBALMEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)


//记录读写位置
static loff_t roffset = 0;
static loff_t woffset = 0;


struct globalmem_t{
    char * shmem;
    struct cdev cdev;
};
struct globalmem_t global;

void dump_filp(const struct file * filp)
{
    //unsigned long count = atomic_long_read(&filp->f_count);
    unsigned long count = 0;
    printk("%s - %d: dump_file info:%p",__func__,__LINE__, filp);
    printk("\tfop=%p,private=%p,pos=%p-0x%llx, f_inode=%p\n",filp->f_op,filp->private_data,&filp->f_pos,filp->f_pos,filp->f_inode);
    printk("\tflags=0x%x,mode = 0x%lx,count=0x%lx,version=0x%lx\n",filp->f_flags,(unsigned long)filp->f_mode,(unsigned long)count,(unsigned long)filp->f_version);
    return;
}

void dump_inode(const struct inode * inode)
{

    return;
}

int globalmem_open (struct inode *inode, struct file *filp)
{
    struct globalmem_t * pglobal = (struct globalmem_t *)container_of(inode->i_cdev,struct globalmem_t,cdev); //获取
    filp->private_data = pglobal;


    printk("%s-%d inode = %p,cdev = %p\n",__func__,__LINE__,inode, inode->i_cdev);
    dump_filp(filp);
    dump_inode(inode);

    return 0;
}

ssize_t globalmem_read (struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    struct globalmem_t * pglobal = filp->private_data;
    
    loff_t p = *offset;
    printk("%s-%d:count = %#lx,offset=%p-%#llx\n",__func__, __LINE__, count,offset,*offset);
    if(p >= BUF_SIZE){
        printk(KERN_ERR"%s-%d failed: current pos is over \n",__func__, __LINE__);
        return 0;
    }

    if(count > BUF_SIZE - p){
        count = BUF_SIZE - p;
    }
    if(copy_to_user(buf,pglobal->shmem + p,count)){ //success return 0
        printk(KERN_ERR"%s-%d failed: read count %#lx ,copy to user failed\n",__func__, __LINE__, count);
        return -EFAULT;    
    } else {
        *offset += count;
        roffset = *offset;
        printk("%s-%d:read count = %#lx success\n",__func__, __LINE__,count);
    }
    return count;
}

ssize_t globalmem_write (struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    struct globalmem_t * pglobal = filp->private_data;
    
    loff_t p = *offset;
    printk("%s-%d:count = %#lx,offset=%p-%#llx\n",__func__, __LINE__, count,offset,*offset);
    if(p >= BUF_SIZE){
        printk(KERN_ERR"%s-%d failed: current pos is over \n",__func__, __LINE__);
        return 0;
    }

    if(count > BUF_SIZE - p){
        count = BUF_SIZE - p;
    }
    if(copy_from_user(pglobal->shmem + p,buf,count)){ //success return 0
        printk(KERN_ERR"%s-%d failed: write count %#lx ,copy to user failed\n",__func__, __LINE__, count);
        return -EFAULT;    
    } else {
        *offset += count;
        woffset = *offset;
        printk("%s-%d:write count = %#lx success\n",__func__, __LINE__,count);
    }
    return count;
}

int globalmem_release (struct inode *inode, struct file *filp)
{

    return 0;
}

loff_t globalmem_llseek (struct file * filp, loff_t offset, int orig)
{
    loff_t ret = 0;

    printk("%s-%d:offset = %#llx,offset=%p-%#llx\n",__func__, __LINE__,offset,&filp->f_pos, filp->f_pos);
    switch(orig){
        case 0: //SEEK_SET
            if(offset < 0 || offset > BUF_SIZE  ) {
                printk("%s-%d FAILED: INVAILD OFFSET\n",__func__, __LINE__ );
                return -EINVAL;
            }
            filp->f_pos = offset;
            ret = filp->f_pos;
            break;
        case 1: //SEEK_CUR
            if((offset + filp->f_pos > BUF_SIZE  ) || (offset + filp->f_pos < 0  ) ) {
                printk("%s-%d FAILED: INVAILD OFFSET\n",__func__, __LINE__ );
                return -EINVAL;
            }
            filp->f_pos += offset;
            ret = filp->f_pos;
            break;
        case 2: //SEEK_EMD
            if( offset > 0  ) {
                printk("%s-%d FAILED: INVAILD OFFSET\n",__func__, __LINE__ );
                return -EINVAL;
            }
            filp->f_pos = BUF_SIZE + offset;
            ret = filp->f_pos;

            break;
        default:
            printk("%s-%d FAILED: not support cmd\n",__func__, __LINE__ );
            break;
    }


    return ret;
}

long globalmem_unlocked_ioctl (struct file *filp, unsigned int cmd, unsigned long args)
{
    struct inode * pinode = filp->f_inode;
    struct globalmem_t * pglobal = filp->private_data;
    printk("%s-%d\n",__func__, __LINE__);

    switch(cmd){
        case GLOBALMEM_CLEAR:
            memset(pglobal->shmem, 0 ,BUF_SIZE);
            filp->f_pos = 0;
            break;

        default:
            break;
    }
    return 0;
}

long globalmem_compat_ioctl (struct file *filp, unsigned int cmd, unsigned long args)
{
    struct inode * pinode = filp->f_inode;
    struct globalmem_t * pglobal = filp->private_data;
    printk("%s-%d\n",__func__, __LINE__);

    switch(cmd){
        case GLOBALMEM_CLEAR:
            memset(pglobal->shmem, 0 ,BUF_SIZE);
            filp->f_pos = 0;
            break;

        default:
            break;
    }
    return 0;

}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = globalmem_open,
    .release = globalmem_release,
    .read = globalmem_read,
    .write = globalmem_write,
    .llseek = globalmem_llseek,
    .unlocked_ioctl = globalmem_unlocked_ioctl,  //test ioctl 
    .compat_ioctl = globalmem_compat_ioctl,
};

    

static int __init mychar_init(void)
{
    int ret = 0;
    dev_t devno;


    global.shmem =  kzalloc(BUF_SIZE, GFP_KERNEL);
    if(!global.shmem){
        printk(KERN_ERR"failed region chrdev\n");
        return -ENOMEM;
    }
    //1.注册字符驱动
    if(major){
        devno = MKDEV(major,0);
        ret = register_chrdev_region(devno, 1,"globalmem");
    }else{
        ret = alloc_chrdev_region(&devno,0, 1,"globalmem");
        major = MAJOR(devno);
    }
    if(ret < 0){
        printk(KERN_ERR"failed region chrdev\n");
        goto failed_register;
    }

    //2.注册字符驱动信息
    cdev_init(&global.cdev,&fops); // fs/char_dev.c 
    global.cdev.owner=THIS_MODULE;

    ret = cdev_add(&global.cdev,devno,1);
    if(ret < 0){
        printk(KERN_ERR"failed cdev add\n");
        goto failed_cdev;
    }
    
    //3.注册字符驱动设备节点
    class = class_create(THIS_MODULE,"globalmem");
    if(!class){
        printk(KERN_ERR"failed class create\n");
        goto failed_class;
    }
    device = device_create(class, NULL, devno,NULL,"globalmem"); //mknod /dev/globalmem c xxx 0
    if(!device){
        printk(KERN_ERR"failed device create\n");
        goto failed_device;
    }
    
    printk("mychar_init:%s,cdev=%p,global=%p\n",__FUNCTION__,&global.cdev, &global);
    
    return 0;//成功返回0，失败返回错误码

    device_destroy(class,devno);
failed_device:
    class_destroy(class);
failed_class:
    cdev_del(&global.cdev);
failed_cdev:
    unregister_chrdev_region(devno,1);
failed_register:
    kfree(global.shmem);
    return -EINVAL;
}


static void __exit mychar_exit(void)
{
    dev_t devno;
    devno = MKDEV(major,0);
    device_destroy(class,devno);
    class_destroy(class);
    cdev_del(&global.cdev);
    unregister_chrdev_region(devno,1);
    kfree(global.shmem);
    printk("mychar_exit:%s\n",__FUNCTION__);
    return;
}

module_init(mychar_init);
module_exit(mychar_exit); //修饰入口和出口函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>"); //作者信息，选填
MODULE_DESCRIPTION("This is a test driver"); //描述信息，选填
MODULE_VERSION("1.0.0.0");	//版本描述，选填
//modinfo 可查看这些信息

