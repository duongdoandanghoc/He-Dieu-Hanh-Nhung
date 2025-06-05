
/**
* asgn1.c - Full Virtual RAM Disk Kernel Module
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#define MYDEV_NAME "asgn1"
#define MYIOC_TYPE 'k'
#define SET_NPROC_OP 1
#define TEM_SET_NPROC _IOW(MYIOC_TYPE, SET_NPROC_OP, int)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nhom 1");
MODULE_DESCRIPTION("Virtual RAM Disk Kernel Module");

typedef struct page_node_rec {
   struct list_head list;
   struct page *page;
} page_node;

struct asgn1_dev_t {
   dev_t dev;
   struct cdev *cdev;
   struct list_head mem_list;
   int num_pages;
   size_t data_size;
   atomic_t nprocs;
   atomic_t max_nprocs;
   struct class *class;
   struct device *device;
};

static struct asgn1_dev_t asgn1_device;
static struct proc_dir_entry *asgn1_proc;
static int asgn1_major = 0;
static int asgn1_minor = 0;
static int asgn1_dev_count = 1;

static void free_memory_pages(void) {
   page_node *curr, *tmp;
   list_for_each_entry_safe(curr, tmp, &asgn1_device.mem_list, list) {
       if (curr->page)
           __free_page(curr->page);
       list_del(&curr->list);
       kfree(curr);
   }
   asgn1_device.num_pages = 0;
   asgn1_device.data_size = 0;
}

static int asgn1_open(struct inode *inode, struct file *filp) {
   if (atomic_read(&asgn1_device.nprocs) >= atomic_read(&asgn1_device.max_nprocs))
       return -EBUSY;
   atomic_inc(&asgn1_device.nprocs);
   if (filp->f_flags & O_WRONLY)
       free_memory_pages();
   return 0;
}

static int asgn1_release(struct inode *inode, struct file *filp) {
   atomic_dec(&asgn1_device.nprocs);
   return 0;
}

static ssize_t asgn1_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
   size_t size_read = 0, offset, to_read;
   int page_no = *f_pos / PAGE_SIZE, cur_page = 0;
   page_node *curr;

   if (*f_pos >= asgn1_device.data_size) return 0;
   list_for_each_entry(curr, &asgn1_device.mem_list, list) {
       if (cur_page++ < page_no) continue;
       offset = *f_pos % PAGE_SIZE;
       to_read = min((size_t)(PAGE_SIZE - offset), count - size_read);
       if (*f_pos + to_read > asgn1_device.data_size)
           to_read = asgn1_device.data_size - *f_pos;

       if (copy_to_user(buf + size_read, page_address(curr->page) + offset, to_read))
           return -EFAULT;
       *f_pos += to_read;
       size_read += to_read;
       if (size_read >= count) break;
   }
   return size_read;
}

static ssize_t asgn1_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
   size_t size_written = 0, offset, to_write;
   int page_no = *f_pos / PAGE_SIZE, cur_page = 0;
   int needed_pages = (*f_pos + count + PAGE_SIZE - 1) / PAGE_SIZE;
   page_node *curr;

   while (asgn1_device.num_pages < needed_pages) {
       curr = kmalloc(sizeof(page_node), GFP_KERNEL);
       if (!curr) return -ENOMEM;
       curr->page = alloc_page(GFP_KERNEL);
       if (!curr->page) return -ENOMEM;
       list_add_tail(&curr->list, &asgn1_device.mem_list);
       asgn1_device.num_pages++;
   }

   list_for_each_entry(curr, &asgn1_device.mem_list, list) {
       if (cur_page++ < page_no) continue;
       offset = *f_pos % PAGE_SIZE;
       to_write = min((size_t)(PAGE_SIZE - offset), count - size_written);
       if (copy_from_user(page_address(curr->page) + offset, buf + size_written, to_write))
           return -EFAULT;
       *f_pos += to_write;
       size_written += to_write;
       if (size_written >= count) break;
   }

   asgn1_device.data_size = max(asgn1_device.data_size, (size_t)(*f_pos));
   return size_written;
}

static loff_t asgn1_lseek(struct file *file, loff_t offset, int whence) {
   loff_t new_pos;
   size_t buffer_size = asgn1_device.num_pages * PAGE_SIZE;

   switch (whence) {
   case SEEK_SET: new_pos = offset; break;
   case SEEK_CUR: new_pos = file->f_pos + offset; break;
   case SEEK_END: new_pos = buffer_size + offset; break;
   default: return -EINVAL;
   }
   new_pos = clamp_val(new_pos, 0, buffer_size);
   file->f_pos = new_pos;
   return new_pos;
}

static long asgn1_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
   int val;
   if (_IOC_TYPE(cmd) != MYIOC_TYPE) return -EINVAL;
   if (_IOC_NR(cmd) == SET_NPROC_OP) {
       if (copy_from_user(&val, (int __user *)arg, sizeof(int))) return -EFAULT;
       if (val < 1) return -EINVAL;
       atomic_set(&asgn1_device.max_nprocs, val);
       return 0;
   }
   return -ENOTTY;
}

static int asgn1_mmap(struct file *filp, struct vm_area_struct *vma) {
   page_node *curr;
   unsigned long start = vma->vm_start;
   unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
   size_t len = vma->vm_end - vma->vm_start;
   int i = 0;

   if (len > asgn1_device.num_pages * PAGE_SIZE)
       return -EINVAL;

   list_for_each_entry(curr, &asgn1_device.mem_list, list) {
       if ((i * PAGE_SIZE) >= offset && (i * PAGE_SIZE) < offset + len) {
           unsigned long pfn = page_to_pfn(curr->page);
           if (remap_pfn_range(vma, start + i * PAGE_SIZE, pfn, PAGE_SIZE, vma->vm_page_prot))
               return -EAGAIN;
       }
       i++;
   }
   return 0;
}

static int proc_show(struct seq_file *m, void *v) {
   seq_printf(m, "Major: %d\nMinor: %d\nPages: %d\nData size: %zu\nNprocs: %d\nMax nprocs: %d\n",
              asgn1_major, asgn1_minor, asgn1_device.num_pages, asgn1_device.data_size,
              atomic_read(&asgn1_device.nprocs), atomic_read(&asgn1_device.max_nprocs));
   return 0;
}

static int proc_open(struct inode *inode, struct file *file) {
   return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_fops = {
   .proc_open = proc_open,
   .proc_read = seq_read,
   .proc_lseek = seq_lseek,
   .proc_release = single_release
};

static struct file_operations asgn1_fops = {
   .owner = THIS_MODULE,
   .open = asgn1_open,
   .release = asgn1_release,
   .read = asgn1_read,
   .write = asgn1_write,
   .llseek = asgn1_lseek,
   .unlocked_ioctl = asgn1_ioctl,
   .mmap = asgn1_mmap
};

static int __init asgn1_init(void) {
   int result;
   atomic_set(&asgn1_device.nprocs, 0);
   atomic_set(&asgn1_device.max_nprocs, 1);
   INIT_LIST_HEAD(&asgn1_device.mem_list);

   result = alloc_chrdev_region(&asgn1_device.dev, 0, 1, MYDEV_NAME);
   if (result < 0) return result;
   asgn1_major = MAJOR(asgn1_device.dev);

   asgn1_device.cdev = cdev_alloc();
   cdev_init(asgn1_device.cdev, &asgn1_fops);
   cdev_add(asgn1_device.cdev, asgn1_device.dev, 1);

   asgn1_device.class = class_create(MYDEV_NAME);
   asgn1_device.device = device_create(asgn1_device.class, NULL, asgn1_device.dev, NULL, MYDEV_NAME);

   asgn1_proc = proc_create(MYDEV_NAME, 0, NULL, &proc_fops);
   return 0;
}

static void __exit asgn1_exit(void) {
   device_destroy(asgn1_device.class, asgn1_device.dev);
   class_destroy(asgn1_device.class);
   cdev_del(asgn1_device.cdev);
   unregister_chrdev_region(asgn1_device.dev, 1);
   remove_proc_entry(MYDEV_NAME, NULL);
   free_memory_pages();
}

module_init(asgn1_init);
module_exit(asgn1_exit);
