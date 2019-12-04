
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/leds.h>

#include <linux/fs.h>
#include <linux/pwm.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>

#include <linux/timer.h>
#include <asm/uaccess.h>

#define TIME_OUT 4
#define DEV_NAME "int_gpio"
#define GPIO_NUM 230

#if 1
static int major;
static struct class *cls;
struct pwm_device *pwm=0;
static unsigned int irqno;
static struct fasync_struct *signal_async;
static char press_flag = 0;
static char count = 0;
static struct timer_list timer;

irqreturn_t hand_irq_event(int irq, void* dev)
{
	disable_irq_nosync(irq);
	printk("hand irq\n");
	//kill_fasync (&signal_async, SIGIO, POLL_IN);
	if (!count) {
		count++;
		add_timer(&timer);
	}
	enable_irq(irq);
	return IRQ_HANDLED;
}


static ssize_t led_get(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	return sprintf(buf, "%s\n", "enable led");
}

static ssize_t led_set(struct device* dev,
		struct device_attribute* attr, const char* buf, size_t len)
{
	printk("set buf %s\n", buf);
	if (pwm&&buf[0]=='1')
		pwm_enable(pwm);
	else if (pwm&&buf[0]=='0')
		pwm_disable(pwm);

	return len;
}

static DEVICE_ATTR(pwm_ledxx, S_IWUSR|S_IRUSR, led_get, led_set);

static int int_gpio_open(struct inode *inode, struct file *file) 
{
	return 0;
}

static int int_gpio_fasync (int fd, struct file *filp, int on)
{
	return fasync_helper(fd, filp, on, &signal_async);
}

static int int_gpio_release(struct inode *inode, struct file *file)
{
	int_gpio_fasync(-1, file, 0);
	return 0;
}

static ssize_t int_gpio_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	copy_to_user(buf, &press_flag, 1);
	press_flag = 0;
	return 1;
}

struct file_operations pwm_led_ops={
	.owner  = THIS_MODULE,
	.open   = int_gpio_open,
	.read   = int_gpio_read,
	.release= int_gpio_release,
	.fasync = int_gpio_fasync,
};
#endif

void timer_fn(unsigned long i)
{
	printk("timer out\n");
	if (!gpio_get_value(GPIO_NUM) && count<TIME_OUT) {
		count ++;
		mod_timer(&timer, jiffies+HZ);
	} else {
		if (count<TIME_OUT)
			press_flag = 1;
		else
			press_flag = 2;
		count = 0;
		kill_fasync (&signal_async, SIGIO, POLL_IN);
	}
	//timer.expires = jiffies+HZ;
	//add_timer(&timer);
}

static int __init pwm_trig_init(void)
{
	struct device *mydev;
	printk("__init\n");
	major = register_chrdev(0, DEV_NAME, &pwm_led_ops);
	cls = class_create(THIS_MODULE, DEV_NAME"s");
	mydev = device_create(cls, 0, MKDEV(major,0), NULL, "%s_%d", DEV_NAME, 1);

	sysfs_create_file(&(mydev->kobj), &dev_attr_pwm_ledxx.attr);

	irqno = gpio_to_irq(GPIO_NUM);
	printk("irq num:%d \n", irqno);
	request_irq(irqno, hand_irq_event, IRQF_TRIGGER_FALLING, "eint_keydown", NULL);

	init_timer(&timer);
	timer.function = timer_fn;
	timer.expires = jiffies+HZ;

	printk("success exit \n");
	return 0;
	
//err:
	printk("err exit \n");
	device_destroy(cls, MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major, DEV_NAME);
	return 0;
}

static void __exit pwm_trig_exit(void)
{
	printk("__exit\n");
	free_irq(irqno, NULL);
	device_destroy(cls, MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major, "pwm_led");
}

module_init(pwm_trig_init);
module_exit(pwm_trig_exit);

MODULE_AUTHOR("p.Chen <18768590968@163.com>");
MODULE_DESCRIPTION("pwm trigger");
MODULE_LICENSE("GPL");

