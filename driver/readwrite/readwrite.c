#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>


#include "readwrite.h"


#define DEVICE_NAME			"readwrite"
#define MAJOR_NUM			(1001)
#define SEMA_NUM			(1)
#define USER_BUFF_SIZE		(8)
#define USER_NUM			(2)


typedef struct {
	char *pBuffBase;
	char *pBuffR;
	char *pBuffW;
	bool isWrap;

	struct semaphore readSema;
	struct semaphore writeSema;

	wait_queue_head_t readQue;
	wait_queue_head_t writeQue;

	bool isUsed;
} ST_USER_INFO;


static struct file_operations gFileops;
static ST_USER_INFO gstUserInfo [USER_NUM];
static struct semaphore g_sema; // for open/close

// old info
static ST_USER_INFO gstUserInfo_old;
static char gszBuff [USER_BUFF_SIZE];


static int getPos (char *p, ST_USER_INFO *pUserInfo)
{
	return (int)(p - pUserInfo->pBuffBase);
}

// 内部バッファ末尾まで何byteあるか
static int getRemainSize (char *p, ST_USER_INFO *pUserInfo)
{
	return (int)(USER_BUFF_SIZE - getPos(p, pUserInfo));
}

static int getReadableSize (ST_USER_INFO *pUserInfo)
{
	if ((pUserInfo->pBuffW - pUserInfo->pBuffR) == 0) {
		return (int)(!pUserInfo->isWrap ? 0 : USER_BUFF_SIZE);

	} else if ((pUserInfo->pBuffW - pUserInfo->pBuffR) > 0) {
		return (int)(pUserInfo->pBuffW - pUserInfo->pBuffR);

	} else {
		// pBuffWの位置が一周している場合
		return (int)(USER_BUFF_SIZE - (pUserInfo->pBuffR - pUserInfo->pBuffW));
	}
}

static int getWritableSize (ST_USER_INFO *pUserInfo)
{
	if ((pUserInfo->pBuffW - pUserInfo->pBuffR) == 0) {
		return (int)(!pUserInfo->isWrap ? USER_BUFF_SIZE : 0);

	} else if ((pUserInfo->pBuffW - pUserInfo->pBuffR) > 0) {
		return (int)(USER_BUFF_SIZE - (pUserInfo->pBuffW - pUserInfo->pBuffR));

	} else {
		// pBuffWの位置が一周している場合
		return (int)(pUserInfo->pBuffR - pUserInfo->pBuffW);
	}
}

/*
 * open()
 */
static int openDev (struct inode* inode, struct file* filp)
{
	int i = 0;

	printk (KERN_INFO "%s : call open()\n", DEVICE_NAME);

	down (&g_sema);

	for (i = 0; i < USER_NUM; i ++) {
		if (!gstUserInfo[i].isUsed) {
			break;
		}
	}

	if (i == USER_NUM) {
		// not found 
		up (&g_sema);
		return -EBUSY;
	}

	// alloc
	gstUserInfo[i].pBuffBase = NULL; //TODO
	gstUserInfo[i].pBuffR = NULL; //TODO
	gstUserInfo[i].pBuffW = NULL; //TODO
	gstUserInfo[i].isWrap = false;
	sema_init (&gstUserInfo[i].readSema, SEMA_NUM);
	sema_init (&gstUserInfo[i].writeSema, SEMA_NUM);
	init_waitqueue_head (&gstUserInfo[i].readQue);
	init_waitqueue_head (&gstUserInfo[i].writeQue);
	gstUserInfo[i].isUsed = true;

	filp->private_data = (void*)&gstUserInfo[i];

	up (&g_sema);

	return 0;
}

/*
 * close()
 */
static int closeDev (struct inode *inode, struct file *filp)
{
	ST_USER_INFO *pUserInfo = NULL;

	printk (KERN_INFO "%s : call close()\n", DEVICE_NAME);

	down (&g_sema);

	if (filp->private_data) {
		pUserInfo = (ST_USER_INFO*)filp->private_data;

		pUserInfo->pBuffBase = NULL;
		pUserInfo->pBuffR = NULL;
		pUserInfo->pBuffW = NULL;
		pUserInfo->isWrap = false;
		sema_init (&(pUserInfo->readSema), SEMA_NUM);
		sema_init (&(pUserInfo->writeSema), SEMA_NUM);
		init_waitqueue_head (&(pUserInfo->readQue));
		init_waitqueue_head (&(pUserInfo->writeQue));
		pUserInfo->isUsed = false;
	}

	filp->private_data = NULL;

	up (&g_sema);

	return 0;
}

/*
 * read()
 */
static ssize_t readDev (struct file* filp, char* buf, size_t count, loff_t* pos)
{
	int nCopylen = 0;
	int nCopylenInner = 0;
	int nCopylenInnerWrap = 0;
	ST_USER_INFO *pUserInfo = NULL;

	printk (KERN_INFO "%s : call read()\n", DEVICE_NAME);

//TODO
pUserInfo = &gstUserInfo_old;

//	if (down_interruptible (&gSema)) {
//		// signal interrupt
//		printk (KERN_NOTICE "%s : down_interruptible() interrupt occurs.\n", DEVICE_NAME);
//		return -ERESTARTSYS;
//	}
	if (down_trylock (&(pUserInfo->readSema)) != 0) {
		printk (KERN_NOTICE "%s : read semaphore is not available.\n", DEVICE_NAME);
		return -EBUSY;
	}


	// 内部バッファに読み取るものが無い場合はブロックする
	while (getReadableSize(pUserInfo) == 0) {

		// ノンブロックでopenしていたらすぐ戻る
		if (filp->f_flags & O_NONBLOCK) {
			printk (KERN_INFO "%s : read() is return. (O_NONBLOCK)\n", DEVICE_NAME);
			up (&(pUserInfo->readSema));
			return -EAGAIN;
		}

		printk (KERN_INFO "%s : read() is blocking.\n", DEVICE_NAME);

		//
		// プロセスを待ち状態に
		// wake_up_interruptible()により待ちが解除された場合、
		// 第2引数式が0(TRUE)である場合0を返す。
		// 0を返さなかった場合はシグナルによる割り込みです。
		//
		if (wait_event_interruptible (pUserInfo->readQue, (getReadableSize(pUserInfo) > 0)) != 0) {
			// signal interrupt
			printk (KERN_NOTICE "%s : wait_event_interruptible() interrupt occurs.\n", DEVICE_NAME);
			up (&(pUserInfo->readSema));
			return -ERESTARTSYS;
		}
	}


	// ユーザ側読むサイズ指定チェック
	nCopylen = count > getReadableSize(pUserInfo) ? getReadableSize(pUserInfo) : count;
	// 内部バッファ配列末尾までに読み込めるサイズチェック
	nCopylenInner = nCopylen > getRemainSize(pUserInfo->pBuffR, pUserInfo) ? getRemainSize(pUserInfo->pBuffR, pUserInfo) : nCopylen;
	// バッファ一周したら正の数になる
	nCopylenInnerWrap = nCopylen - getRemainSize(pUserInfo->pBuffR, pUserInfo);


	// ユーザ側に渡す
	if (copy_to_user (buf, pUserInfo->pBuffR, nCopylenInner)) {
		printk (KERN_ERR "%s : copy_to_user()\n", DEVICE_NAME);
		up (&(pUserInfo->readSema));
		return -EFAULT;
	}
	printk (KERN_INFO "%s : copy_to_user() is success. [%d]bytes\n", DEVICE_NAME, nCopylenInner);

	// ユーザが読み取った部分は内部バッファから削除
	memset (pUserInfo->pBuffR, 0x00, nCopylenInner);

	*pos += nCopylenInner;
	buf += nCopylenInner;


	//
	// pBuffR 位置の更新
	//
	if (nCopylenInnerWrap == 0) {

		// 丁度一周した
		pUserInfo->pBuffR = pUserInfo->pBuffBase;
		pUserInfo->isWrap = false;

	} else if (nCopylenInnerWrap > 0) {

		// 一周した
		pUserInfo->pBuffR = pUserInfo->pBuffBase;
		pUserInfo->isWrap = false;

		// ユーザ側に渡す
		if (copy_to_user (buf, pUserInfo->pBuffR, nCopylenInnerWrap)) {
			printk (KERN_ERR "%s : copy_to_user()\n", DEVICE_NAME);
			up (&(pUserInfo->readSema));
			return -EFAULT;
		}	
		printk (KERN_INFO "%s : copy_to_user() is success. (wrap) [%d]bytes\n", DEVICE_NAME, nCopylenInnerWrap);

		// ユーザが読み取った部分は内部バッファから削除
		memset (pUserInfo->pBuffR, 0x00, nCopylenInnerWrap);

		*pos += nCopylenInnerWrap;
		pUserInfo->pBuffR += nCopylenInnerWrap;

	} else {
		pUserInfo->pBuffR += nCopylenInner;
	}


	up (&(pUserInfo->readSema));

	// write()を実行可能状態に
	if (getWritableSize(pUserInfo) > 0) {
		wake_up_interruptible (&(pUserInfo->writeQue));
	}

	return nCopylen;
}

/*
 * write()
 */
static ssize_t writeDev (struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	int nCopylen = 0;
	int nCopylenInner = 0;
	int nCopylenInnerWrap = 0;
	ST_USER_INFO *pUserInfo = NULL;

	printk (KERN_INFO "%s : call write()\n", DEVICE_NAME);

//TODO
pUserInfo = &gstUserInfo_old;

//	if (down_interruptible (&gSema)) {
//		// signal interrupt
//		printk (KERN_NOTICE "%s : down_interruptible() interrupt occurs.\n", DEVICE_NAME);
//		return -ERESTARTSYS;
//	}
	if (down_trylock (&(pUserInfo->writeSema)) != 0) {
		printk (KERN_NOTICE "%s : write semaphore is not available.\n", DEVICE_NAME);
		return -EBUSY;
	}


	// 内部バッファに書き込む空き領域が無い場合はブロックする
	while (getWritableSize(pUserInfo) == 0) {

		// ノンブロックでopenしていたらすぐ戻る
		if (filp->f_flags & O_NONBLOCK) {
			printk (KERN_INFO "%s : write() is return. (O_NONBLOCK)\n", DEVICE_NAME);
			up (&(pUserInfo->writeSema));
			return -EAGAIN;
		}

		printk( KERN_INFO "%s : write() is blocking.\n", DEVICE_NAME );

		// プロセスを待ち状態に
		if (wait_event_interruptible (pUserInfo->writeQue, (getWritableSize(pUserInfo) > 0)) != 0) {
			// シグナルによって割り込まれた
			printk (KERN_NOTICE "%s : wait_event_interruptible() interrupt occurs.\n", DEVICE_NAME);
			up (&(pUserInfo->writeSema));
			return -ERESTARTSYS;
		}
	}


	// ユーザ側書き込むサイズチェック
	nCopylen = count > getWritableSize(pUserInfo) ? getWritableSize(pUserInfo) : count;
	// 内部バッファ配列末尾までに書き込めるサイズチェック
	nCopylenInner = nCopylen > getRemainSize(pUserInfo->pBuffW, pUserInfo) ? getRemainSize(pUserInfo->pBuffW, pUserInfo) : nCopylen;
	// バッファ一周したら正の数になる
	nCopylenInnerWrap = nCopylen - getRemainSize(pUserInfo->pBuffW, pUserInfo);


	// ユーザ側から受け取る
	if (copy_from_user (pUserInfo->pBuffW, buf, nCopylenInner)) {
		printk (KERN_ERR "%s : copy_from_user()\n", DEVICE_NAME);
		up (&(pUserInfo->writeSema));
		return -EFAULT;
	}
	printk (KERN_INFO "%s : copy_from_user() is success. [%d]bytes\n", DEVICE_NAME, nCopylenInner);

	*pos += nCopylenInner;
	buf += nCopylenInner;


	//
	// pBuffW 位置の更新
	//
	if (nCopylenInnerWrap == 0) {

		// 丁度一周した
		pUserInfo->pBuffW = pUserInfo->pBuffBase;
		pUserInfo->isWrap = true;

	} else if (nCopylenInnerWrap > 0) {

		// 一周した
		pUserInfo->pBuffW = pUserInfo->pBuffBase;
		pUserInfo->isWrap = true;

		// ユーザ側から受け取る
		if (copy_from_user (pUserInfo->pBuffW, buf, nCopylenInnerWrap)) {
			printk (KERN_ERR "%s : copy_from_user()\n", DEVICE_NAME);
			up (&(pUserInfo->writeSema));
			return -EFAULT;
		}
		printk (KERN_INFO "%s : copy_from_user() is success. (wrap) [%d]bytes\n", DEVICE_NAME, nCopylenInnerWrap);

		*pos += nCopylenInnerWrap;
		pUserInfo->pBuffW += nCopylenInnerWrap;

	} else {
		pUserInfo->pBuffW += nCopylenInner;
	}


	up (&(pUserInfo->writeSema));

	// read()を実行可能状態に
	if (getReadableSize(pUserInfo) > 0) {
		wake_up_interruptible (&(pUserInfo->readQue));
	}

	return nCopylen;
}

/*
 * poll() / select()
 */
static unsigned int pollDev (struct file *filp, poll_table *wait)
{
	unsigned int nRtn = 0;
	ST_USER_INFO *pUserInfo = NULL;

	printk (KERN_INFO "%s : call poll() / select()\n", DEVICE_NAME);

//TODO
pUserInfo = &gstUserInfo_old;

	poll_wait(filp, &(pUserInfo->readQue), wait);
	poll_wait(filp, &(pUserInfo->writeQue), wait);

	if (getReadableSize(pUserInfo) > 0) {
		nRtn |= (POLLIN | POLLRDNORM);
	}

	if (getWritableSize(pUserInfo) > 0) {
		nRtn |= (POLLOUT | POLLWRNORM);
	}

	return nRtn;
}

/*
 * モジュール初期処理
 */
static int initMod (void)
{
	printk (KERN_INFO "driver loaded\n");

gstUserInfo_old.pBuffBase = &gszBuff[0];
gstUserInfo_old.pBuffR = &gszBuff[0];
gstUserInfo_old.pBuffW = &gszBuff[0];
gstUserInfo_old.isWrap = false;
sema_init (&gstUserInfo_old.readSema, SEMA_NUM);
sema_init (&gstUserInfo_old.writeSema, SEMA_NUM);
init_waitqueue_head (&gstUserInfo_old.readQue);
init_waitqueue_head (&gstUserInfo_old.writeQue);
gstUserInfo_old.isUsed = true;


	sema_init (&g_sema, SEMA_NUM);

	// file_operations
	gFileops.owner = THIS_MODULE;
	gFileops.open = openDev;
	gFileops.read = readDev;
	gFileops.write = writeDev;
	gFileops.release = closeDev;
	gFileops.poll = pollDev;

	if (register_chrdev (MAJOR_NUM, DEVICE_NAME, &gFileops)) {
		printk (KERN_ERR "register_chrdev()\n");
		return -EBUSY;
	}

	return 0;
}

/*
 * モジュール開放
 */
static void cleanMod (void)
{
	printk (KERN_INFO "driver unloaded\n");

	unregister_chrdev (MAJOR_NUM, DEVICE_NAME);

	return;
}

MODULE_LICENSE ("GPL");

module_init (initMod);
module_exit (cleanMod);
