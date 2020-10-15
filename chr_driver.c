#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Felipe Brasileiro");
MODULE_DESCRIPTION("Simple character driver with read and write functionality");
MODULE_LICENSE("MIT");

#define DEVICE_NAME "m_device"
#define m_MAJOR 245 // change the major if it's already in use
#define BUFFER_SIZE 1024

struct my_device_d
{
    struct cdev cdev;
    char buffer[BUFFER_SIZE];
};
struct my_device_d devs;

static int isDeviceOpen = 0;

static int m_open(struct inode *pinode, struct file *pfile);
static int m_release(struct inode *pinode, struct file *pfile);
static ssize_t m_read(struct file *pfile, char __user *buffer, size_t len, loff_t *offset);
static ssize_t m_write(struct file *pfile, const char __user *buffer, size_t len, loff_t *offset);

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = m_open,
    .read = m_read,
    .write = m_write,
    .release = m_release,
};

static int m_open(struct inode *pinode, struct file *pfile)
{
    struct my_device_d *my_device = container_of(pinode->i_cdev, struct my_device_d, cdev);
    if (isDeviceOpen)
    {
        return -EBUSY;
    }
    pfile->private_data = my_device;
    printk(KERN_INFO "[*] My_device: Device has been opened\n");
    isDeviceOpen++;
    return 0;
}

static int m_release(struct inode *pinode, struct file *pfile)
{
    pfile->private_data = NULL;
    isDeviceOpen--;
    printk(KERN_INFO "[*] My_device: Device has been closed\n");
    return 0;
}

static ssize_t m_read(struct file *pfile, char __user *buffer, size_t u_len, loff_t *offset)
{
    struct my_device_d *my_device = (struct my_device_d *)pfile->private_data;
    ssize_t len = BUFFER_SIZE - *offset;
    if (len <= 0)
        return 0;
    if (len >= u_len)
        len = u_len;
    if (copy_to_user(buffer, my_device->buffer + *offset, len))
    {
        return -EFAULT;
    }
    printk(KERN_INFO "[*] My_device: Device has been read\n");
    *offset += len;
    return 0;
}

static ssize_t m_write(struct file *pfile, const char __user *buffer, size_t u_len, loff_t *offset)
{
    struct my_device_d *my_device = (struct my_device_d *)pfile->private_data;
    ssize_t len = BUFFER_SIZE - *offset;
    if (len <= 0)
        return 0;
    if (len >= u_len)
        len = u_len;
    if (copy_from_user(my_device->buffer + *offset, buffer, len))
        return -EFAULT;
    printk(KERN_INFO "[*] My_device: Device has been written\n");
    *offset += len;
    return len;
}

int m_init(void)
{
    int error;
    printk(KERN_INFO "[+] My_device: Device has been loaded...\n");
    error = register_chrdev_region(MKDEV(m_MAJOR, 0), 1, DEVICE_NAME);
    if (error != 0)
    {
        printk(KERN_ALERT "[!] My_device: Failed to allocate major\n");
        return error;
    }
    cdev_init(&devs.cdev, &fops);
    cdev_add(&devs.cdev, MKDEV(m_MAJOR, 0), 1);
    return 0;
}

void m_exit(void)
{
    cdev_del(&devs.cdev);
    unregister_chrdev_region(MKDEV(m_MAJOR, 0), 1);
    printk(KERN_INFO "[-] My_device: Device has been unloaded...\n");
}

module_init(m_init);
module_exit(m_exit);