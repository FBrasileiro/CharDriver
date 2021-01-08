#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Felipe Brasileiro");
MODULE_DESCRIPTION("Simple character driver with read and write functionality");
MODULE_LICENSE("GPL");

#define DEVICE_NAME "char_driver"
#define BUFFER_SIZE 1024

struct my_device_d
{
    struct cdev cdev;
    char buffer[BUFFER_SIZE];
};
static struct my_device_d devs;
static struct class *dev_class;
static int isDeviceOpen = 0;
dev_t dev;

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
    printk(KERN_INFO "[*] %s: Device has been opened\n", DEVICE_NAME);
    isDeviceOpen++;
    return 0;
}

static int m_release(struct inode *pinode, struct file *pfile)
{
    pfile->private_data = NULL;
    isDeviceOpen--;
    printk(KERN_INFO "[*] %s: Device has been closed\n", DEVICE_NAME);
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
    printk(KERN_INFO "[*] %s: Device has been read\n", DEVICE_NAME);
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
    memset(my_device->buffer, '\0', BUFFER_SIZE);
    if (copy_from_user(my_device->buffer + *offset, buffer, len))
        return -EFAULT;
    printk(KERN_INFO "[*] %s: Device has been written\n", DEVICE_NAME);
    *offset += len;
    return len;
}

static int __init m_init(void)
{
    int error;
    printk(KERN_INFO "[+] %s: Device has been loaded...\n", DEVICE_NAME);
    error = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (error != 0)
    {
        printk(KERN_ALERT "[!] %s: Failed to allocate major\n", DEVICE_NAME);
        return error;
    }
    pr_info("Major: %d  Minor: %d\n", MAJOR(dev), MINOR(dev));
    cdev_init(&devs.cdev, &fops);
    if (cdev_add(&devs.cdev, dev, 1))
    {
        pr_alert("[!] %s: Failed to add the device to the system\n", DEVICE_NAME);
        goto remove;
    }
    if ((dev_class = class_create(THIS_MODULE, "m_class")) == NULL)
    {
        pr_alert("[!] %s: Failed to create struct class\n", DEVICE_NAME);
        goto remove;
    }
    if (device_create(dev_class, NULL, dev, NULL, DEVICE_NAME) == NULL)
    {
        pr_alert("[!] %s: Failed to create device\n", DEVICE_NAME);
        goto remove_device;
    }
    pr_info("[+] %s: Device has been added successfully\n", DEVICE_NAME);
    return 0;

remove_device:
    class_destroy(dev_class);

remove:
    unregister_chrdev_region(dev, 1);
    return -1;
}

static void __exit m_exit(void)
{
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&devs.cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "[-] %s: Device has been unloaded...\n", DEVICE_NAME);
}

module_init(m_init);
module_exit(m_exit);
