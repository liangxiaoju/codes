#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/io.h>
#include <linux/switch.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>

#define VBUS_NDET	S5PV210_GPH3(6)
#define HOST_NDET	S5PV210_GPJ3(7)
#define USB_SEL		S5PV210_GPJ4(1)

#ifdef DEBUG
#define usb_power_dbg(fmt, arg...)	\
	printk(KERN_INFO fmt, ## arg)
#else
#define usb_power_dbg(fmt, arg...)
#endif

struct usb_power {
	struct device *dev;
	struct regulator *vusb_d;
	struct regulator *vusb_a;
	struct regulator *v5v_pwren;
	struct delayed_work usb_detect;
	int usb_host_status;
	int usb_device_status;
	struct platform_device *hci_pdev[2];
	struct platform_device *pdev[2];
	struct wake_lock wakelock;
	struct task_struct *thread;
	int event;
	wait_queue_head_t wait;
	struct switch_dev sdev_device;
	struct switch_dev sdev_host;
};

static struct usb_power usb_power_ctrl;

static void usb_power_detect_init(void)
{
	s3c_gpio_setpull(VBUS_NDET, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(HOST_NDET, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(USB_SEL, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(VBUS_NDET, S3C_GPIO_SFN(0xf));
	s3c_gpio_cfgpin(HOST_NDET, S3C_GPIO_SFN(0xf));
	s3c_gpio_cfgpin(USB_SEL, S3C_GPIO_OUTPUT);

//	__raw_writel(__raw_readl(S5PV210_GPJ3_INTFLTCON1) | (0xf << 24), S5PV210_GPJ3_INTFLTCON1);
}

static void enable_usb_device(void)
{
	struct usb_power *ctrl = &usb_power_ctrl;

	s3c_gpio_setpin(USB_SEL, 0);

#if 0
	regulator_enable(ctrl->vusb_d);
	regulator_enable(ctrl->vusb_a);
#endif

	usb_power_dbg("%s\n", __func__);

	switch_set_state(&ctrl->sdev_device, 1);
	wake_lock(&ctrl->wakelock);
}

static void disable_usb_device(void)
{
	struct usb_power *ctrl = &usb_power_ctrl;

#if 0
	regulator_force_disable(ctrl->vusb_d);
	regulator_force_disable(ctrl->vusb_a);
#endif

	usb_power_dbg("%s\n", __func__);
	switch_set_state(&ctrl->sdev_device, 0);
	wake_unlock(&ctrl->wakelock);
}

static void add_usb_hci_pdev(void)
{
	int ret, i;
	struct platform_device *pdev;
	struct usb_power *ctrl = &usb_power_ctrl;

	for (i = 0; i < 2; i++) {

		if (ctrl->pdev[i])
			return ;

		pdev = ctrl->hci_pdev[i];

		ctrl->pdev[i] = platform_device_alloc(pdev->name, -1);
		ctrl->pdev[i]->dev.dma_mask = pdev->dev.dma_mask;
		ctrl->pdev[i]->dev.coherent_dma_mask = pdev->dev.coherent_dma_mask;

		platform_device_add_resources(ctrl->pdev[i],
			pdev->resource, pdev->num_resources);

		ret = platform_device_add(ctrl->pdev[i]);
		if (ret < 0) {
			printk(KERN_ERR "Failed to add platform_device: %s\n",
					pdev->name);
			break;
		}
	}

	wake_lock(&ctrl->wakelock);
}

static void del_usb_hci_pdev(void)
{
	int i;
	struct usb_power *ctrl = &usb_power_ctrl;

	for (i = 1; i >= 0; i--) {

		if (ctrl->pdev[i] == NULL)
			return ;

		platform_device_unregister(ctrl->pdev[i]);
		ctrl->pdev[i] = NULL;
	}

	wake_unlock(&ctrl->wakelock);
}

static void enable_usb_host(void)
{
	struct usb_power *ctrl = &usb_power_ctrl;

	s3c_gpio_setpin(USB_SEL, 1);

#if 0
	regulator_enable(ctrl->vusb_d);
	regulator_enable(ctrl->vusb_a);
#endif

	add_usb_hci_pdev();

	switch_set_state(&ctrl->sdev_host, 1);

	usb_power_dbg("%s\n", __func__);
}

static void disable_usb_host(void)
{
	struct usb_power *ctrl = &usb_power_ctrl;

	del_usb_hci_pdev();

	usb_power_dbg("%s\n", __func__);

#if 0
	regulator_force_disable(ctrl->vusb_d);
	regulator_force_disable(ctrl->vusb_a);
#endif

	s3c_gpio_setpin(USB_SEL, 0);

	switch_set_state(&ctrl->sdev_host, 0);
}

static int check_usb_host_status(void)
{
	int status;
	struct usb_power *ctrl = &usb_power_ctrl;

	usb_power_dbg("%s\n", __func__);

	s3c_gpio_cfgpin(HOST_NDET, S3C_GPIO_INPUT);
	ndelay(100);
	status = gpio_get_value(HOST_NDET);

	s3c_gpio_cfgpin(HOST_NDET, S3C_GPIO_SFN(0xf));

	ctrl->usb_host_status = !status;
	ctrl->usb_device_status = 0;

	return !status;
}

static int check_usb_device_status(void)
{
	int status;
	struct usb_power *ctrl = &usb_power_ctrl;

	usb_power_dbg("%s\n", __func__);

	if (check_usb_host_status())
		return 0;

	s3c_gpio_cfgpin(VBUS_NDET, S3C_GPIO_INPUT);
	ndelay(100);
	status = gpio_get_value(VBUS_NDET);

	s3c_gpio_cfgpin(VBUS_NDET, S3C_GPIO_SFN(0xf));

	ctrl->usb_host_status = 0;
	ctrl->usb_device_status = !status;

	return !status;
}

static int usb_detect_kthread(void *__unused)
{
	struct usb_power *ctrl = &usb_power_ctrl;

	do {
		if (check_usb_host_status())
			enable_usb_host();
		else {
			disable_usb_host();

			if (check_usb_device_status())
				enable_usb_device();
			else
				disable_usb_device();
		}

		ctrl->event = 0;

		wait_event_interruptible(ctrl->wait, ctrl->event);

	} while (!kthread_should_stop());

	usb_power_dbg("kthread [kusbpd] exit.\n");

	return 0;
}

static void usb_work_handle(struct work_struct *work)
{
	struct usb_power *ctrl =
		container_of(to_delayed_work(work),
			struct usb_power, usb_detect);

	ctrl->event = 1;
	wake_up_interruptible(&ctrl->wait);
}

static irqreturn_t usb_detect_handle(int irq, void *dev_id)
{
	struct usb_power *ctrl = &usb_power_ctrl;

	schedule_delayed_work(&ctrl->usb_detect, 0);

	if (irq == IRQ_EINT_GROUP(21, 7)) {
		usb_power_dbg("IRQ: HOST_NDET\n");
	} else if (irq == gpio_to_irq(VBUS_NDET)) {
		usb_power_dbg("IRQ: VBUS_NDET\n");
	}

	return IRQ_HANDLED;
}

static int usb_power_probe(struct platform_device *pdev)
{
	int ret;
	struct usb_power *ctrl;

	printk(KERN_INFO "%s\n", __func__);

	ctrl = &usb_power_ctrl;
	ctrl->dev = &pdev->dev;
	ctrl->hci_pdev[0] = (struct platform_device *)pdev->resource->start;
	ctrl->hci_pdev[1] = (struct platform_device *)pdev->resource->end;

	INIT_DELAYED_WORK(&ctrl->usb_detect, usb_work_handle);
	init_waitqueue_head(&ctrl->wait);
	wake_lock_init(&ctrl->wakelock, WAKE_LOCK_SUSPEND, "USB-POWER");

	platform_set_drvdata(pdev, ctrl);

	ctrl->sdev_device.name = "usb_connected";
	ret = switch_dev_register(&ctrl->sdev_device);
	if (ret < 0)
		goto err_usb_connected;

	ctrl->sdev_host.name = "usb_host_connected";
	ret = switch_dev_register(&ctrl->sdev_host);
	if (ret < 0)
		goto err_usb_host_connected;

	ret = gpio_request(HOST_NDET, "HOST_nDET");
	if (ret < 0)
		printk("Failed to request HOST_nDET.\n");

	ret = gpio_request(VBUS_NDET, "VBUS_nDET");
	if (ret < 0)
		printk("Failed to request VBUS_nDET.\n");

	ctrl->vusb_d = regulator_get(NULL, "VUSB_D_PWREN");
	if (IS_ERR(ctrl->vusb_d)) {
		printk(KERN_ERR "Failed to get VUSB_D_PWREN.\n");
		ret = PTR_ERR(ctrl->vusb_d);
		goto err_vusb_d;
	}

	ctrl->vusb_a = regulator_get(NULL, "VUSB_A_PWREN");
	if (IS_ERR(ctrl->vusb_a)) {
		printk(KERN_ERR "Failed to get VUSB_A_PWREN.\n");
		ret = PTR_ERR(ctrl->vusb_a);
		goto err_vusb_a;
	}

	ctrl->v5v_pwren = regulator_get(NULL, "V5V_PWREN");
	if (IS_ERR(ctrl->v5v_pwren)) {
		printk(KERN_ERR "Failed to get V5V_PWREN.\n");
		ret = PTR_ERR(ctrl->v5v_pwren);
		goto err_v5v_pwren;
	}

	/* v5v always on */
	regulator_enable(ctrl->v5v_pwren);

	usb_power_detect_init();

	ctrl->thread = kthread_run(usb_detect_kthread, NULL, "kusbpd");
	if (IS_ERR(ctrl->thread))
		goto err_kthread;

	ret = request_irq(IRQ_EINT_GROUP(21, 7),
			usb_detect_handle,
			IRQF_DISABLED |
			IRQF_TRIGGER_RISING |
			IRQF_TRIGGER_FALLING,
			"HOST_NDET",
			ctrl);
	if (ret) {
		printk(KERN_ERR "Failed to request HOST_NDET irq.%d\n",ret);
		goto err_host_ndet;
	}

	ret = request_irq(gpio_to_irq(VBUS_NDET),
			usb_detect_handle,
			IRQF_DISABLED |
			IRQF_TRIGGER_RISING |
			IRQF_TRIGGER_FALLING,
			"VBUS_NDET",
			ctrl);
	if (ret) {
		printk(KERN_ERR "Failed to request VBUS_NDET irq.\n");
		goto err_vbus_ndet;
	}

	return 0;

err_vbus_ndet:
	free_irq(IRQ_EINT_GROUP(21, 7), ctrl);
err_host_ndet:
	kthread_stop(ctrl->thread);
err_kthread:
	regulator_put(ctrl->v5v_pwren);
err_v5v_pwren:
	regulator_put(ctrl->vusb_a);
err_vusb_a:
	regulator_put(ctrl->vusb_d);
err_vusb_d:
	switch_dev_unregister(&ctrl->sdev_host);
err_usb_host_connected:
	switch_dev_unregister(&ctrl->sdev_device);
err_usb_connected:
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int usb_power_remove(struct platform_device *pdev)
{
	struct usb_power *ctrl = platform_get_drvdata(pdev);

	free_irq(gpio_to_irq(VBUS_NDET), ctrl);
	free_irq(IRQ_EINT_GROUP(21, 7), ctrl);
	kthread_stop(ctrl->thread);
	regulator_put(ctrl->v5v_pwren);
	regulator_put(ctrl->vusb_a);
	regulator_put(ctrl->vusb_d);
	switch_dev_unregister(&ctrl->sdev_host);
	switch_dev_unregister(&ctrl->sdev_device);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static int usb_power_prepare(struct device *dev)
{
	del_usb_hci_pdev();
	disable_irq(IRQ_EINT_GROUP(21, 7));

	return 0;
}

static void usb_power_complete(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_power *ctrl = platform_get_drvdata(pdev);

	set_irq_type(IRQ_EINT_GROUP(21, 7), IRQ_TYPE_EDGE_BOTH);
	enable_irq(IRQ_EINT_GROUP(21, 7));

	schedule_delayed_work(&ctrl->usb_detect, HZ);
}

static int usb_power_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_power *ctrl = platform_get_drvdata(pdev);

	regulator_force_disable(ctrl->vusb_a);
	regulator_force_disable(ctrl->vusb_d);

	return 0;
}

static int usb_power_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_power *ctrl = platform_get_drvdata(pdev);

	regulator_enable(ctrl->vusb_d);
	regulator_enable(ctrl->vusb_a);

	return 0;
}

static int usb_power_runtime_suspend(struct device *dev)
{
	usb_power_dbg("%s\n", __func__);

	return 0;
}

static int usb_power_runtime_resume(struct device *dev)
{
	usb_power_dbg("%s\n", __func__);

	return 0;
}

static struct dev_pm_ops usb_power_pm = {
	.prepare	= usb_power_prepare,
	.complete	= usb_power_complete,
	.suspend	= usb_power_suspend,
	.resume		= usb_power_resume,
	.runtime_suspend= usb_power_runtime_suspend,
	.runtime_resume	= usb_power_runtime_resume,
};

static struct platform_driver usb_power_driver = {
	.probe		= usb_power_probe,
	.remove		= usb_power_remove,
	.driver		= {
		.name	= "usb-power",
		.owner	= THIS_MODULE,
		.pm	= &usb_power_pm,
	},
};

static int __init usb_power_init(void)
{
	int ret;

	printk("USB Power Switch Driver.\n");
	ret = platform_driver_register(&usb_power_driver);
	if (ret < 0) {
		printk("Failed to register USB Power Switch.\n");
		return ret;
	}

	return 0;
}

static void __exit usb_power_exit(void)
{
	platform_driver_unregister(&usb_power_driver);
}

late_initcall(usb_power_init);
module_exit(usb_power_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("liangxiaoju <liangxiaoju@szboeye.com>");

