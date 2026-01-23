#include "asm-generic/errno-base.h"
#include "asm-generic/memory_model.h"
#include "asm/current.h"
#include "asm/io.h"
#include "asm/page_64.h"
#include "asm/page_64_types.h"
#include "asm/page_types.h"
#include "asm/pgtable_64_types.h"
#include "linux/gfp.h"
#include "linux/mm.h"
#include "linux/mmdebug.h"
#include "linux/page_ref.h"
#include "linux/pfn.h"
#include "linux/sched.h"
#include "linux/slab.h"
#include "linux/smp.h"
#include "linux/spinlock.h"
#include "linux/types.h"
#include "linux/wait.h"
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/pagemap.h>
#include <asm/cacheflush.h>
#include "asm-generic/getorder.h"
#include "linux/slub_def.h"
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/ktime.h>

#include "testkmisc.h"

#define MINOR_BASE			(0)
#define MINOR_COUNT			(255)
#define MODULE_NAME			"testkmisc"

enum thread_req_state {
	REQ_STATE_NEW = 0,
	REQ_STATE_SUBMITTED,
	REQ_STATE_COMPLETED,
	REQ_STATE_FAILED,
	REQ_STATE_ABORTED
};

struct testkmisc_cdev {
	int major;
	struct cdev cdev;
	dev_t cdevno;
	struct mutex lock;

	enum thread_req_state req_state;
	wait_queue_head_t req_waitq;

	struct task_struct *kthread;
	wait_queue_head_t kthread_waitq;
	unsigned int kthread_sched;

} *s_cdev;

static struct page **s_pages = NULL;
static unsigned int s_pages_nr = 0;

#define MAX_HELD_PAGES (1024)
static int s_held_cnt;
static struct page *s_held_pages[MAX_HELD_PAGES];


static void unmap_user_buf(void)
{
	int i;

	if (!s_pages || !s_pages_nr)
		return;

	for (i = 0; i < s_pages_nr; i++) {
		if (s_pages[i]) {
//			set_page_dirty_lock(s_pages[i]); // if writed : feel like it's meaningless because it's a user page and not a file.
			put_page(s_pages[i]);
			pr_info ("idx%d refcnt:%d flags:%#lx(%pGp)\n", i, page_ref_count(s_pages[i]), s_pages[i]->flags, &s_pages[i]->flags);
		}
	}
	pr_info ("unmap %d pages\n", i);

	kfree(s_pages);
	s_pages = NULL;
	s_pages_nr = 0;
}

static int map_user_buf(const char __user *buf, size_t len, bool write)
{
	unsigned int pages_nr = (((unsigned long)buf + len + PAGE_SIZE - 1) -
				 ((unsigned long)buf & PAGE_MASK)) >> PAGE_SHIFT;
	int i;
	int rv;

	s_pages_nr = 0;

	if (pages_nr == 0)
		return -EINVAL;

	s_pages = kcalloc(pages_nr, sizeof(struct page *), GFP_KERNEL);
	if (!s_pages) {
		pr_err("pages OOM.\n");
		rv = -ENOMEM;
		goto err;
	}

	pr_info ("pages_nr %d\n", pages_nr);

	rv = get_user_pages_fast((unsigned long)buf, pages_nr, FOLL_WRITE, s_pages);
	if (rv < 0) {
		pr_err("err: get_user_pages_fast rv %d\n", rv);
		goto err_gup;
	}
	if (rv != pages_nr) {
		pr_err("err: unable to pin down all %d user pages, %d.\n", pages_nr, rv);
		s_pages_nr = rv;
		rv = -EFAULT;
		goto err_gup_nr;
	}
	s_pages_nr = pages_nr;

	pr_info("pin down %d user pages\n", rv);

	for (i = 0; i < pages_nr; i++) {
		pr_info ("idx%d refcnt:%d flags:%#lx(%pGp)\n", i, page_ref_count(s_pages[i]), s_pages[i]->flags, &s_pages[i]->flags);
	}

	for (i = 1; i < pages_nr; i++) {
		if (s_pages[i - 1] == s_pages[i]) {
			pr_err("err: duplicate pages, %d, %d.\n", i - 1, i);
			rv = -EFAULT;
			goto err_gup_nr;
		}
	}

	return 0;

err_gup_nr:
	unmap_user_buf();
err_gup:
	kfree(s_pages);
err:
	return rv;
}

static void put_pages(bool show)
{
	struct page * page;
	pgoff_t index;
	if (show) {
		pr_info("%s\n", __func__);
		pr_info("heldcnt %d\n", s_held_cnt);
	}
	for (index = 0; index < s_held_cnt; index++) {
		page = s_held_pages[index];
		put_page(s_held_pages[index]);
		if (show)
			pr_info ("idx%lu %px refcnt:%d flags:%#lx(%pGp)\n", index, page, page_ref_count(page), page->flags, &page->flags);
	}
}

static int find_pages(struct file *file, size_t count, loff_t pos)
{
	pgoff_t index, start_index, end_index;
	struct page *page;
	struct address_space *mapping = file->f_mapping;

	pr_info("%s\n", __func__);

	start_index = pos >> PAGE_SHIFT;
	end_index = (pos + count - 1) >> PAGE_SHIFT;
	if (end_index - start_index + 1 > MAX_HELD_PAGES) {
		pr_info("MAX_HELD_PAGES over\n");
		return -EINVAL;
	}

	s_held_cnt = 0;
	for (index = start_index; index <= end_index; index++) {
		page = find_get_page(mapping, index);
		if (page == NULL) {
			pr_debug("find_get_page null\n");
			page = find_or_create_page(mapping, index, GFP_KERNEL);
			if (page == NULL) {
				pr_debug("find_or_create_page null\n");
				put_pages(true);
				return -ENOMEM;
			}
			unlock_page(page); // find_or_create_page -> locked
		}
		s_held_pages[s_held_cnt++] = page;
		pr_info ("idx%lu %px refcnt:%d flags:%#lx(%pGp)\n", index, page, page_ref_count(page), page->flags, &page->flags);
		pr_info ("  mapping:%px index:%lu\n", page->mapping, page->index);
//		dump_page(page, "testtest");
	}

	pr_info("heldcnt %d\n", s_held_cnt);
	return 0;
}

static void vma_dump (int lv)
{
	char *pathname = NULL;
	char *dent = NULL;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	int i = 0;
	if (mm) {
		down_read(&mm->mmap_sem);
		pr_info("pgd %lu\n", mm->pgd->pgd);
		pr_info("mm_pgtables_bytes %lu\n", mm_pgtables_bytes(mm));
		pr_info("vm_area count %d\n", mm->map_count);

		vma = mm->mmap;
		if (vma) {
			for (i = 0; i < mm->map_count; i++) {
				if (vma->vm_file) {
					pathname = kmalloc(PATH_MAX, GFP_ATOMIC);
					if (pathname) {
						dent = d_path(&vma->vm_file->f_path, pathname, PATH_MAX);
					}
				}

				if (dent) {
					pr_info("%lx-%lx %pGv off:%lx ino:%lu %s\n", vma->vm_start, vma->vm_end, &vma->vm_flags, vma->vm_pgoff << PAGE_SHIFT , vma->vm_file->f_inode->i_ino ,dent);
					if (lv > 0) {
						find_pages(vma->vm_file, vma->vm_end - vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
						put_pages(false);
					}

				} else if (vma->vm_start <= mm->brk && vma->vm_end >= mm->start_brk) {
					// fs/proc/task_mmu.c show_map_vma
					pr_info("%lx-%lx %pGv [heap]\n", vma->vm_start, vma->vm_end, &vma->vm_flags);
				} else if (vma->vm_start <= mm->start_stack && vma->vm_end >= mm->start_stack) {
					// fs/proc/task_mmu.c show_map_vma
					pr_info("%lx-%lx %pGv [stack]\n", vma->vm_start, vma->vm_end, &vma->vm_flags);
				} else {
					pr_info("%lx-%lx %pGv\n", vma->vm_start, vma->vm_end, &vma->vm_flags);
				}
				if (pathname) {
					kfree(pathname);
					pathname = NULL;
				}
				dent = NULL;

				pr_info("  vm_page_prot %ld\n", (unsigned long)pgprot_val(vma->vm_page_prot));

				vma = vma->vm_next;
			}
		}

		up_read(&mm->mmap_sem);
	}
}

static int fileread (const char* path)
{
	int rv = 0;
	struct file *file;
	struct kstat *stat;
	mm_segment_t fs;
	loff_t pos = 0;
	char *buf;

	file = filp_open(path, O_RDWR, 0600);
	if (IS_ERR(file)) {
		pr_err("filp_open failure\n");
		return PTR_ERR(file);
	}
	if (!(file->f_mode & FMODE_CAN_READ)) {
		pr_err("alloc_device: cache file not readable\n");
		rv = -EINVAL;
		goto err_close;
	}
	if (!(file->f_mode & FMODE_CAN_WRITE)) {
		pr_err("alloc_device: cache file not writeable\n");
		rv = -EINVAL;
		goto err_close;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	stat = (struct kstat *) kmalloc(sizeof(struct kstat), GFP_KERNEL);
	if (!stat) {
		pr_err("kmalloc failure: kstat\n");
		rv = -ENOMEM;
		goto err_close2;
	}
	vfs_stat(path, stat);
	pr_info("stat size %lld\n", stat->size);

	buf = kmalloc(stat->size, GFP_KERNEL);
	if (!buf) {
		pr_err("kmalloc failure: buf\n");
		rv = -ENOMEM;
		goto err_free_stat;
	}

	pr_info("kernel_read\n");
	rv = kernel_read(file, buf, stat->size, &pos) ;
	if (rv != stat->size) {
		pr_err("kernel_read failure: rv %d\n", rv);
		rv = -EFAULT;
		goto err_free_buf;
	}
	rv = 0;
	pr_info("[%s]\n", buf);

	set_fs(fs);

	filp_close(file, NULL);
	kfree(stat);
	kfree(buf);
	return rv;

err_free_buf:
	kfree(buf);
err_free_stat:
	kfree(stat);
err_close2:
	set_fs(fs);
err_close:
	filp_close(file, NULL);
	return rv;
}

static int pagecache (const char* path)
{
	int rv = 0;
	struct file *file;
	struct kstat *stat;
	mm_segment_t fs;

	file = filp_open(path, O_RDWR, 0600);
	if (IS_ERR(file)) {
		pr_err("filp_open failure\n");
		return PTR_ERR(file);
	}
	if (!(file->f_mode & FMODE_CAN_READ)) {
		pr_err("alloc_device: cache file not readable\n");
		rv = -EINVAL;
		goto err_close;
	}
	if (!(file->f_mode & FMODE_CAN_WRITE)) {
		pr_err("alloc_device: cache file not writeable\n");
		rv = -EINVAL;
		goto err_close;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	stat = (struct kstat *) kmalloc(sizeof(struct kstat), GFP_KERNEL);
	if (!stat) {
		pr_err("kmalloc failure: kstat\n");
		rv = -ENOMEM;
		goto err_close2;
	}
	vfs_stat(path, stat);
	pr_info("stat size %lld\n", stat->size);

	rv = find_pages(file, stat->size, 0);
	put_pages(true);

	filp_close(file, NULL);
	kfree(stat);
	set_fs(fs);
	return rv;

err_close2:
	set_fs(fs);
err_close:
	filp_close(file, NULL);
	return rv;
}

static void vaddr (int size)
{
	void *pk = NULL;
	void *pv = NULL;
	void *gpage = NULL;
	struct page *page = NULL;
	struct page *tmp = NULL;

	pr_info("PAGE_SIZE                %d\n", (int)PAGE_SIZE);
	pr_info("PAGE_SHIFT               %d\n", PAGE_SHIFT);
	pr_info("PAGE_MASK                %lx\n", PAGE_MASK);
	pr_info("\n");
	pr_info("__PHYSICAL_START         %x\n", __PHYSICAL_START);
	pr_info("__START_KERNEL           %lx\n", __START_KERNEL);
	pr_info("__START_KERNEL_map       %lx\n", __START_KERNEL_map); // kernel text mapping, from phys 0
	pr_info("PAGE_OFFSET              %lx\n", PAGE_OFFSET); // offset to start of virtual address, from phys 0 (direct mapping of all phys), kmalloc space
	pr_info("VMALLOC_START            %lx\n", VMALLOC_START); // vmalloc/ioremap space
	pr_info("VMALLOC_END              %lx\n", VMALLOC_END);
	pr_info("__PHYSICAL_MASK_SHIFT    %d\n", __PHYSICAL_MASK_SHIFT);
	pr_info("__VIRTUAL_MASK_SHIFT     %d\n", __VIRTUAL_MASK_SHIFT);
	pr_info("vmemmap                  %px\n", vmemmap); // virtual memory map
	pr_info("VMEMMAP_START            %lx\n", VMEMMAP_START);
	pr_info("sizeof(struct page)      %ld\n", sizeof(struct page));
	pr_info("\n");

	pk = kmalloc(size, GFP_KERNEL);
	if (pk) {
		pr_info("kmalloc                  %px\n", pk);
		pr_info("p - PAGE_OFFSET          %lx\n", (unsigned long)pk - PAGE_OFFSET);
		pr_info("virt_to_phys             %llx\n", virt_to_phys(pk));
		pr_info("phys_to_virt             %px\n", phys_to_virt(virt_to_phys(pk)));

		pr_info("__pa(p) >> PAGE_SHIFT    %lx\n", __pa(pk) >> PAGE_SHIFT);
		pr_info("(__pa(p) >> PAGE_SHIFT) * sizeof(struct page) = %lx\n", (__pa(pk) >> PAGE_SHIFT) * sizeof(struct page));
		tmp = virt_to_page(pk);
		pr_info("virt_to_page             %px\n", tmp);
		pr_info("page_to_pfn              %lx\n", page_to_pfn(tmp));
		pr_info("PFN_PHYS                 %llx\n", PFN_PHYS(page_to_pfn(tmp)));

		pr_info("page - vmemmap           %lx\n", tmp - vmemmap); // between struct page*
		pr_info("(page - vmemmap) << PAGE_SHIFT = %lx\n",(tmp - vmemmap) << PAGE_SHIFT);
		pr_info("((page - vmemmap) << PAGE_SHIFT) + PAGE_OFFSET = %lx\n",((tmp - vmemmap) << PAGE_SHIFT) + PAGE_OFFSET);
		pr_info("page_to_virt             %px\n", page_to_virt(tmp));

		tmp = virt_to_head_page(pk);
		if (tmp->slab_cache) {
			pr_info("slab_cache               %px\n", tmp->slab_cache);
			if (tmp->slab_cache->name) {
				pr_info("slab_cache name          %s\n", tmp->slab_cache->name);
			}
		}

		pr_info("virt_addr_valid          %d\n", __virt_addr_valid((unsigned long)pk));
		pr_info("is_vmalloc_addr          %d\n", is_vmalloc_addr(pk));
		pr_info("\n");
	} else {
		pr_err("kmalloc failure\n");
	}

	pv = vmalloc(size);
	if (pv) {
		pr_info("vmalloc                  %px\n", pv);
		pr_info("vmalloc_to_pfn           %lx\n", vmalloc_to_pfn(pv));
		pr_info("vmalloc_to_page          %px\n", vmalloc_to_page(pv));
		pr_info("page_to_pfn              %lx\n", page_to_pfn(vmalloc_to_page(pv)));
		pr_info("PFN_PHYS                 %llx\n", PFN_PHYS(page_to_pfn(vmalloc_to_page(pv))));
		pr_info("virt_addr_valid          %d\n", __virt_addr_valid((unsigned long)pv));
		pr_info("is_vmalloc_addr          %d\n", is_vmalloc_addr(pv));
		pr_info("\n");
	} else {
		pr_err("vmalloc failure\n");
	}

	page = alloc_page(GFP_KERNEL);
	if (page) {
		pr_info("alloc_page               %px\n", page);
		pr_info("page_address             %px\n", page_address(page));
		pr_info("page_to_virt             %px\n", page_to_virt(page));
		pr_info("page_to_pfn              %lx\n", page_to_pfn(page));
		pr_info("PFN_PHYS                 %llx\n", PFN_PHYS(page_to_pfn(page)));
		pr_info("virt_addr_valid          %d\n", __virt_addr_valid((unsigned long)page_to_virt(page)));
		pr_info("is_vmalloc_addr          %d\n", is_vmalloc_addr(page_to_virt(page)));
		pr_info("\n");
	} else {
		pr_err("alloc_page failure\n");
	}

	gpage = (void*)__get_free_pages(GFP_KERNEL, get_order(size));
	if (gpage) {
		pr_info("get_free_pages           %px order %d\n", gpage, get_order(size));
		pr_info("virt_to_phys             %llx\n", virt_to_phys(gpage));
		pr_info("phys_to_virt             %px\n", phys_to_virt(virt_to_phys(gpage)));
		pr_info("virt_to_page             %px\n", virt_to_page(gpage));
		pr_info("page_to_pfn              %lx\n", page_to_pfn(virt_to_page(gpage)));
		pr_info("PFN_PHYS                 %llx\n", PFN_PHYS(page_to_pfn(virt_to_page(gpage))));
		pr_info("virt_addr_valid          %d\n", __virt_addr_valid((unsigned long)gpage));
		pr_info("is_vmalloc_addr          %d\n", is_vmalloc_addr(gpage));
		pr_info("\n");
	} else {
		pr_err("get_free_pages failure\n");
	}


	if (pk)
		kfree(pk);
	if (pv)
		vfree(pv);
	if (page)
		__free_pages(page, 0);
	if (gpage)
		free_pages((unsigned long)gpage, get_order(size));
}

static int open(struct inode *inode, struct file *file)
{
	struct testkmisc_cdev *cdev;
	int cpu;

	cdev = container_of(inode->i_cdev, struct testkmisc_cdev, cdev);
	if (cdev == NULL) {
		pr_err("container_of\n");
		return -EFAULT;
	}

	cpu = get_cpu();
	pr_debug("%s smp%d\n", __func__, cpu);
	put_cpu();
	file->private_data = cdev;

	return 0;
}

static int close(struct inode *inode, struct file *file)
{
	struct testkmisc_cdev *cdev = (struct testkmisc_cdev *)file->private_data;
	int cpu = get_cpu();
	pr_debug("%s smp%d\n", __func__, cpu);
	put_cpu();

	return 0;
}

#if 0
static ssize_t read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	ssize_t rv = 0;
	struct testkmisc_cdev *cdev = file->private_data;

	int cpu = get_cpu();
	pr_debug("%s smp%d\n", __func__, cpu);
	put_cpu();

	pr_debug("len = %lu, off = %llu\n", len, *off);

	mutex_lock(&cdev->lock);

	rv = map_user_buf(buf, len, 0);
	unmap_user_buf();

	mutex_unlock(&cdev->lock);

	return rv;
}

static ssize_t write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	ssize_t rv = 0;
	struct testkmisc_cdev *cdev = file->private_data;

	int cpu = get_cpu();
	pr_debug("%s smp%d\n", __func__, cpu);
	put_cpu();

	pr_debug("len = %lu, off = %llu\n", len, *off);

	mutex_lock(&cdev->lock);

	rv = map_user_buf(buf, len, 1);
	unmap_user_buf();

	mutex_unlock(&cdev->lock);

	return rv;
}
#endif

static int thread_func(void *data)
{
	struct testkmisc_cdev *cdev = data;
	int cpu = 0;

	while (!kthread_should_stop()) {
		cpu = get_cpu();
		pr_info("%s wait_event_interruptible begin smp%d", __func__, cpu);
		put_cpu();

		wait_event_interruptible(cdev->kthread_waitq, cdev->kthread_sched);

		cpu = get_cpu();
		pr_info("%s wait_event_interruptible end smp%d", __func__, cpu);
		put_cpu();

		cdev->kthread_sched = 0;

//		pr_info("%s\n", __func__);
		usleep_range(10, 11);

		cdev->req_state = REQ_STATE_COMPLETED;
		wake_up_interruptible(&cdev->req_waitq);
		pr_info("%s wake_up_interruptible", __func__);
		
		schedule();
	}

	return 0;
}

static long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	ssize_t rv = 0;
	struct testkmisc_cdev *cdev = file->private_data;
	struct ioctl_testkmisc_gup itg ;
	char path[PATH_MAX];
	int iarg = 0;

	int cpu = get_cpu();
	pr_debug("%s smp%d\n", __func__, cpu);
	put_cpu();

	mutex_lock(&cdev->lock);

	switch (cmd) {
	case IOCTL_TESTKMISC_GUP:
		rv = copy_from_user(
				&itg,
				(struct testkmisc_ioctl __user *)arg,
				sizeof(struct ioctl_testkmisc_gup)
			);
		if (rv == 0) {
			rv = map_user_buf((const char*)itg.buf, itg.len, 1);
			unmap_user_buf();
		}
		break;

	case IOCTL_TESTKMISC_VMADUMP:
		rv = copy_from_user( &iarg, (int __user *)arg, sizeof(int));
		if (rv == 0) {
			vma_dump(iarg);
		}
		break;

	case IOCTL_TESTKMISC_FILEREAD:
		memset (path, 0,sizeof(path));
		rv = copy_from_user(path, (char __user *)arg, strlen((char __user *)arg));
		if (rv == 0) {
			pr_info("read path[%s]\n", path);
			rv = fileread(path);
		}
		break;

	case IOCTL_TESTKMISC_PAGECACHE:
		memset (path, 0,sizeof(path));
		rv = copy_from_user(path, (char __user *)arg, strlen((char __user *)arg));
		if (rv == 0) {
			pr_info("pagecache path[%s]\n", path);
			rv = pagecache(path);
		}
		break;

	case IOCTL_TESTKMISC_VADDR:
		rv = copy_from_user( &iarg, (int __user *)arg, sizeof(int));
		if (rv == 0) {
			vaddr(iarg);
		}
		break;

	case IOCTL_TESTKMISC_KTHREAD_RUN: {
		ktime_t start_time, end_time;
		s64 time_diff_ns;
		start_time = ktime_get();

		pr_info("*** kthread run - begin ***");
		cdev->req_state = REQ_STATE_SUBMITTED;

		cdev->kthread_sched = 1;
		wake_up_interruptible(&cdev->kthread_waitq);
		pr_info("%s wake_up_interruptible", __func__);

		pr_info("%s wait_event_interruptible begin state[%d]", __func__, cdev->req_state);
		wait_event_interruptible(cdev->req_waitq, cdev->req_state != REQ_STATE_SUBMITTED);
		pr_info("%s wait_event_interruptible end state[%d]", __func__, cdev->req_state);

		end_time = ktime_get();
		time_diff_ns = ktime_to_ns(ktime_sub(end_time, start_time));
		pr_info("*** kthread run - end *** %lld ns\n", time_diff_ns);

		}
		break;

	default:
		pr_err("Unsupported operation\n");
		rv = -EINVAL;
		break;
	}

	mutex_unlock(&cdev->lock);

	return rv;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = open,
	.release = close,
//	.read = read,
//	.write = write,
	.unlocked_ioctl = ioctl,
};

static int _init(void)
{
	int rv = 0;
	dev_t dev;
	struct testkmisc_cdev *cdev = NULL;

	pr_info("%s\n", __func__);

	cdev = kmalloc(sizeof(struct testkmisc_cdev), GFP_KERNEL);
	if (!cdev) {
		rv = -ENOMEM;
		goto err;
	}
	memset (cdev, 0, sizeof(struct testkmisc_cdev));

	rv = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_COUNT, MODULE_NAME);
	if (rv != 0) {
		rv = -EINVAL;
		pr_err("alloc_chrdev_region\n");
		goto err_alloc_cdev;
	}

	cdev->major = MAJOR(dev);
	pr_info("major %d\n", cdev->major);

	// setup cdev_reg
	cdev->cdevno = MKDEV(cdev->major, 0);
	cdev->cdev.owner = THIS_MODULE;
	cdev_init(&cdev->cdev, &fops);
	rv = cdev_add(&cdev->cdev, cdev->cdevno, 1);
	if (rv != 0) {
		rv = -EINVAL;
		pr_err("cdev_add\n");
		goto err_add_cdev;
	}

	mutex_init(&cdev->lock);
	init_waitqueue_head(&cdev->req_waitq);

	init_waitqueue_head(&cdev->kthread_waitq);
	cdev->kthread = kthread_create(thread_func, cdev, MODULE_NAME"-thread");
	wake_up_process(cdev->kthread);

	pr_info("%s done -- cdev 0x%px\n", __func__, cdev);

	s_cdev = cdev;

	return rv;

err_add_cdev:
	unregister_chrdev_region(dev, MINOR_COUNT);
err_alloc_cdev:
	kfree(cdev);
err:
	return rv;
}

static void _finaliz(void)
{
	struct testkmisc_cdev *cdev = s_cdev;

	pr_info("%s\n", __func__);

	if (!cdev)
		return;

	pr_info("cdev 0x%px\n", cdev);

	cdev->kthread_sched = 1;
	wake_up_interruptible(&cdev->kthread_waitq);
	kthread_stop(cdev->kthread);

	cdev_del(&cdev->cdev);
	unregister_chrdev_region(MKDEV(cdev->major, MINOR_BASE), MINOR_COUNT);
	kfree(cdev);
}

static int mod_init(void)
{
	return _init();
}

static void mod_exit(void)
{
	_finaliz();
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
