#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/jiffies.h>



static int proc_test_show(struct seq_file *m, void *v) {
	seq_printf(m, "now test. %llu\n", get_jiffies_64());
	return 0;
}

static int proc_test_open(struct inode *inode, struct  file *file) {
	return single_open(file, proc_test_show, NULL);
}

static const struct file_operations proc_test_fops = {
	.owner = THIS_MODULE,
	.open = proc_test_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init proc_test_init(void) {
	proc_create("proc_test", 0, NULL, &proc_test_fops);
	return 0;
}

static void __exit proc_test_exit(void) {
	remove_proc_entry("proc_test", NULL);
}

MODULE_LICENSE("GPL");
module_init(proc_test_init);
module_exit(proc_test_exit);
