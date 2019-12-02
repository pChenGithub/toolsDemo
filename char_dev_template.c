#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/leds.h>

#include <linux/fs.h>
#include <linux/pwm.h>

#if 1
static int major;
static struct class *cls;
struct pwm_device *pwm=0;

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
static int __init pwm_trig_init(void)
{
	printk("__init\n");
	struct device *mydev;
	major = register_chrdev(0, "pwm_led", &pwm_led_ops);
	cls = class_create(THIS_MODULE, "pwm_leds");
	mydev = device_create(cls, 0, MKDEV(major,0), NULL, "pwm_led%d", 1);

	sysfs_create_file(&(mydev->kobj), &dev_attr_pwm_ledxx.attr);

	printk("request pwm \n");
	//pwm = pwm_request(2, "pwm2");
	pwm = pwm_get(mydev, "pwm_led");
	if (!pwm) {
		printk("get pwm error \n");
		goto err;
	}
	printk("request pwm done, pwm: 0x%x \n", pwm);

	pwm_config(pwm, 2000000, 5000000); // 0.5ms
	printk("config pwm done \n");
	pwm_set_polarity(pwm, PWM_POLARITY_NORMAL);
	printk("set polarity pwm done \n");

	printk("success exit \n");
	return 0;
	
err:
	printk("err exit \n");
	device_destroy(cls, MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major, "pwm_led");
	return 0;
}

static void __exit pwm_trig_exit(void)
{
	printk("__exit\n");
	device_destroy(cls, MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major, "pwm_led");
}

module_init(pwm_trig_init);
module_exit(pwm_trig_exit);

MODULE_AUTHOR("p.Chen <18768590968@163.com>");
MODULE_DESCRIPTION("pwm trigger");
MODULE_LICENSE("GPL");

