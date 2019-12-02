
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


#define DEV_NAME "int_gpio"
#define GPIO_NUM 229

#if 1
static int major;
static struct class *cls;
struct pwm_device *pwm=0;
static unsigned int irqno;

static ssize_t led_get(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	return sprintf(buf, "%s\n", "enable led");
}

static ssize_t led_set(struct device* dev,
		struct device_attribute* attr, char* buf, size_t len)
{
	printk("set buf %s\n", buf);
	if (pwm&&buf[0]=='1')
		pwm_enable(pwm);
	else if (pwm&&buf[0]=='0')
		pwm_disable(pwm);

	return len;
}

static DEVICE_ATTR(pwm_ledxx, S_IWUSR|S_IRUSR, led_get, led_set);

struct file_operations pwm_led_ops={
	.owner  = THIS_MODULE,
};
#endif

irqreturn_t hand_irq_event(int irq, void* dev)
{
	printk("hand irq");
	return IRQ_HANDLED;
}

static int __init pwm_trig_init(void)
{
	printk("__init\n");
	struct device *mydev;
	major = register_chrdev(0, DEV_NAME, &pwm_led_ops);
	cls = class_create(THIS_MODULE, DEV_NAME"s");
	mydev = device_create(cls, 0, MKDEV(major,0), NULL, "%s_%d", DEV_NAME, 1);

	sysfs_create_file(&(mydev->kobj), &dev_attr_pwm_ledxx.attr);

	irqno = gpio_to_irq(GPIO_NUM);
	printk("irq num:%d \n", irqno);

	request_irq(irqno, hand_irq_event, IRQF_TRIGGER_FALLING, "eint_keydown", NULL);


	printk("success exit \n");
	return 0;
	
err:
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

