#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

struct usb_whitelist {
    __u16 vendor_id;
    __u16 product_id;
};

static struct usb_whitelist whitelist[] = {
    {0x0781, 0x5567}, // SanDisk Cruzer Blade
    {0x046D, 0xC534}, // Logitech USB Receiver
    {0x0BDA, 0x5689}, // Realtek Webcam
    {0x0000, 0x0000}  // Kết thúc danh sách
};

static int usb_guard_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_device *dev = interface_to_usbdev(interface);
    
    printk(KERN_INFO "USBGuard: Thiết bị USB hợp lệ kết nối.\n");
    printk(KERN_INFO "USBGuard: Vendor ID: %04X, Product ID: %04X\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
    printk(KERN_INFO "USBGuard: Vendor Name: %s, Product Name: %s\n", dev->manufacturer ? dev->manufacturer : "Unknown",
                                                         dev->product ? dev->product : "Unknown");
    return 0;
}

static void usb_guard_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "USBGuard: Thiết bị USB đã ngắt kết nối.\n");
}

#define USB_GUARD_DEVICE(vendor, product) \
    .match_flags = USB_DEVICE_ID_MATCH_DEVICE, \
    .idVendor = (vendor), \
    .idProduct = (product)

static struct usb_device_id usb_guard_table[] = {
    { USB_GUARD_DEVICE(0x0781, 0x5567) }, // SanDisk
    { USB_GUARD_DEVICE(0x046D, 0xC534) }, // Logitech
    { USB_GUARD_DEVICE(0x0BDA, 0x5689) }, // Realtek
    {} /* kết thúc bảng */
};
MODULE_DEVICE_TABLE(usb, usb_guard_table);

static struct usb_driver usb_guard_driver = {
    .name = "usb_guard_driver",
    .id_table = usb_guard_table,
    .probe = usb_guard_probe,
    .disconnect = usb_guard_disconnect,
};

static int __init usb_guard_init(void)
{
    int result;
    printk(KERN_INFO "USBGuard: Khởi động USB Guard driver (phiên bản nâng cao).\n");
    result = usb_register(&usb_guard_driver);
    if (result)
        printk(KERN_ERR "USBGuard: usb_register failed. Error number %d\n", result);
    return result;
}

static void __exit usb_guard_exit(void)
{
    printk(KERN_INFO "USBGuard: Gỡ bỏ USB Guard driver.\n");
    usb_deregister(&usb_guard_driver);
}

module_init(usb_guard_init);
module_exit(usb_guard_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hzduongviphz2");
MODULE_DESCRIPTION("USB Guard Driver nâng cao kiểm soát thiết bị USB.");
