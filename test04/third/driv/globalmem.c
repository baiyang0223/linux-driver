/*
 * a simple char device driver: globalmem without mutex
 *
 * Copyright (C) 2014 Barry Song  (baohua@kernel.org)
 *
 * Licensed under GPLv2 or later.
 */

#define pr_fmt(fmt) "GLOBAL_MEM_DEBUG: " fmt

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define GLOBALMEM_SIZE	0x1000
#define MEM_CLEAR 0x1


static int globalmem_major = 0;
module_param(globalmem_major, int, S_IRUGO);
MODULE_PARM_DESC(globalmem_major,"globalmem_major  num");


struct globalmem_dev {
	struct cdev cdev;
	struct class * cls;
	struct device * dev;
	unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;


static int globalmem_open(struct inode *inode, struct file *filp)
{
	filp->private_data = globalmem_devp;
	return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd,
			    unsigned long arg)
{
	struct globalmem_dev *dev = filp->private_data;

	switch (cmd) {
	case MEM_CLEAR:
		memset(dev->mem, 0, GLOBALMEM_SIZE);
		printk(KERN_INFO "globalmem is set to zero\n");
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static ssize_t globalmem_read(struct file *filp, char __user * buf, size_t size,
			      loff_t * ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_to_user(buf, dev->mem + p, count)) {
		ret = -EFAULT;
	} else {
		*ppos += count;
		ret = count;

		printk(KERN_INFO "read %u bytes(s) from %lu\n", count, p);
	}

	return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user * buf,
			       size_t size, loff_t * ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_from_user(dev->mem + p, buf, count))
		ret = -EFAULT;
	else {
		*ppos += count;
		ret = count;

		printk(KERN_INFO "written %u bytes(s) from %lu\n", count, p);
	}

	return ret;
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
	loff_t ret = 0;
	switch (orig) {
	case 0:
		if (offset < 0) {
			ret = -EINVAL;
			break;
		}
		if ((unsigned int)offset > GLOBALMEM_SIZE) {
			ret = -EINVAL;
			break;
		}
		filp->f_pos = (unsigned int)offset;
		ret = filp->f_pos;
		break;
	case 1:
		if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
			ret = -EINVAL;
			break;
		}
		if ((filp->f_pos + offset) < 0) {
			ret = -EINVAL;
			break;
		}
		filp->f_pos += offset;
		ret = filp->f_pos;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	.llseek = globalmem_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.unlocked_ioctl = globalmem_ioctl,
	.open = globalmem_open,
	.release = globalmem_release,
};




static int globalmem_setup_cdev(struct globalmem_dev *pdev, int index)
{
	int err = 0;
	dev_t devno = MKDEV(globalmem_major, index);
	
		
	pdev->dev = device_create(pdev->class, NULL, devno, NULL, "globalmem%d", index);
	if (IS_ERR(pdev->dev))
	{
		ret = PTR_ERR(classdev);
		printk(KERN_ERR "device create error %d\n", ret);
		return ret;
	}

	cdev_init(&pdev->cdev, &globalmem_fops);
	pdev->cdev.owner = THIS_MODULE;
	err = cdev_add(&pdev->cdev, devno, 1);
	if (err){
		device_destroy(pdev->class, devno);
		pr_err("Error %d adding globalmem%d\n", err, index);
		return err;
	}

	return 0;
}


static void globalmem_unset_cdev(struct globalmem_dev *pdev, int index)
{
	int err = 0;
	dev_t devno = MKDEV(globalmem_major, index);
	
	device_destroy(pdev->class, devno);
	cdev_del();
	


	return;
}


static int __init globalmem_init(void)
{
	int ret;
	dev_t devno;

	globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	if (!globalmem_devp) {
		pr_err("alloc globalmem_devp failed\n");
		return -ENOMEM;
	}

	if (globalmem_major)
		devno = = MKDEV(globalmem_major, 0);
		ret = register_chrdev_region(devno, 1, "globalmem");
	else {
		ret = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		globalmem_major = MAJOR(devno);
	}
	if (ret < 0){
		pr_err("reg/alloc char dev failed\n");
		goto fail_reg_chrdev
	}
	
	globalmem_devp->cls = class_create(THIS_MODULE,"globalmem");
	if(globalmem_devp->cls == NULL){
		pr_err("class create failed\n");
		ret = PTR_ERR(globalmem_devp->cls);
		goto fail_cls:		
	}
	
	ret = globalmem_setup_cdev(globalmem_devp, 0);
	if(ret < 0){
		pr_err("globalmem_setup_cdev failed\n");
		goto fail_cdev:	
	}
	return 0;

fail_cdev:
	class_destroy(globalmem_devp->cls);
fail_cls:
	unregister_chrdev_region(devno, 1);
fail_reg_chrdev:
	kfree(globalmem_devp);
	return ret;
}


static void __exit globalmem_exit(void)
{

	class_destroy(globalmem_devp->cls);
	cdev_del(&globalmem_devp->cdev);
	kfree(globalmem_devp);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
}


module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("This is a test driver");
MODULE_VERSION("1.0.0.0");
