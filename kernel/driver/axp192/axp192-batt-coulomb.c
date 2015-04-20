#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/mfd/axp192.h>

#include "axp192-common.c"

extern void pmic_sysfs_register_battery(struct pmic_battery_ops *ops);
extern void pmic_sysfs_unregister_battery(struct pmic_battery_ops *ops);
extern void pmic_sysfs_changed(void);
extern int s3c_adc_get_adc_data(int channel);

#if 0
#define pmic_batt_debug(fmt, arg...)	\
	printk(KERN_INFO fmt, ## arg)
#else
#define pmic_batt_debug(fmt, arg...)
#endif

struct pmic_batt_info {
	/* unit: C/1000 */
	int total_coulomb;
	int curr_coulomb;
	int min_coulomb;

	int mAH;

	/* unit: ohm/1000 */
	int resistance;

	/* unit: ms */
	int interval;

	/* unit: mA */
	int suspend_current;

	int curr;
	int volt_a;
	int volt_b;
};

struct pmic_batt_coulomb {
	const char *name;
	int state;
	uint64_t event;
	/* unit: s */
	int report_interval;
	int sleep_time;
	struct device *master;
	struct mutex mutex;
	struct wake_lock wakelock;
	struct notifier_block nb;
	struct delayed_work adc_work;
	struct delayed_work poll_work;
	struct pmic_batt_info info;
	struct pmic_battery_ops *ops;
};

static struct pmic_batt_coulomb pmic_batt;

static int pmic_batt_event(struct notifier_block *n,
		unsigned long data, void *extra)
{
	struct pmic_batt_coulomb *batt =
		container_of(n, struct pmic_batt_coulomb, nb);
	uint64_t event = ((uint64_t)data) |
		(((uint64_t)((unsigned long)extra)) << 32);

	if (!(event & batt->event))
		return NOTIFY_DONE;

	wake_lock_timeout(&batt->wakelock, 3 * HZ);

	pmic_sysfs_changed();

	return NOTIFY_OK;
}

static int pmic_adc_get_volt_a(void)
{
	int adc, volt;

	adc = s3c_adc_get_adc_data(3);
	volt = ((3300*adc)/0xfff)*1000*2;

	pmic_batt_debug("A: adc=0x%03x volt=%duV\n", adc, volt);

	return volt;
}

static int pmic_adc_get_volt_b(void)
{
	int adc, volt;

	adc = s3c_adc_get_adc_data(2);
	volt = ((3300*adc)/0xfff)*1000*2;

	pmic_batt_debug("B: adc=0x%03x volt=%duV\n", adc, volt);

	return volt;
}

static void pmic_batt_adc_init(void)
{
	/* open adc */
}

static int pmic_batt_get_total_coulomb(void)
{
	/* unit: mAS (1mAh = 3.6C) */
	return ((6000 * 36) / 10) * 1000;
}

static int pmic_batt_get_last_coulomb(void)
{
	return 0;
}

static void pmic_batt_save_coulomb(void)
{
	/* save total_coulomb, curr_coulomb, rtc_time */
}

static int pmic_batt_get_init_coulomb(void)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	struct pmic_batt_info *info = &batt->info;
	int coulomb, total_coulomb, last_coulomb = 0;
	int capacity;

	capacity = __battery_capacity(batt->master);
	total_coulomb = info->total_coulomb;
	last_coulomb = pmic_batt_get_last_coulomb();
	coulomb = capacity * (total_coulomb / 100);

	if (last_coulomb > 0)
		coulomb = (last_coulomb / 100) * 70 + (coulomb / 100) * (100 - 70);

	return coulomb;
}

/*
 * return: <0 if discharge, >0 if charge
 */
static int pmic_get_curr(void)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	struct pmic_batt_info *info = &batt->info;
	/*
	 * a > b if discharge
	 * a < b if charge
	 * unit: uV
	 */
	int volt_a, volt_b;
	/* unit: mA */
	int curr;

	volt_a = pmic_adc_get_volt_a();
	volt_b = pmic_adc_get_volt_b();

	info->volt_a = volt_a;
	info->volt_b = volt_b;

	curr = (volt_b - volt_a) / info->resistance;

	return curr;
}

/* unit: mAS */
static int pmic_get_delta_coulomb(void)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	struct pmic_batt_info *info = &batt->info;
	int curr;

	curr = pmic_get_curr();

	info->curr = curr;

	return curr * info->interval / 1000;
}

static void pmic_batt_adjust_coulomb(int delta)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	struct pmic_batt_info *info = &batt->info;

	if (info->curr_coulomb > info->total_coulomb) {
		/* ? */
		info->curr_coulomb = info->total_coulomb;

	} else if (info->curr_coulomb < info->min_coulomb) {

		info->curr_coulomb = info->min_coulomb;
	}
}

static void pmic_adc_work_func(struct work_struct *data)
{
	struct pmic_batt_coulomb *batt =
		container_of(to_delayed_work(data), struct pmic_batt_coulomb, adc_work);
	struct pmic_batt_info *info = &batt->info;
	int delta_coulomb;

	mutex_lock(&batt->mutex);

	delta_coulomb = pmic_get_delta_coulomb();

	info->curr_coulomb += delta_coulomb;

	pmic_batt_adjust_coulomb(delta_coulomb);

	mutex_unlock(&batt->mutex);

	pmic_batt_debug("delta=%dmAS,curr=%umAS,min=%umAS,total=%umAS\n", delta_coulomb,  info->curr_coulomb, info->min_coulomb, info->total_coulomb);

	schedule_delayed_work(&batt->adc_work, msecs_to_jiffies(info->interval));
}

static void pmic_poll_work_func(struct work_struct *data)
{
	struct pmic_batt_coulomb *batt =
		container_of(to_delayed_work(data), struct pmic_batt_coulomb, poll_work);

	pmic_sysfs_changed();

	schedule_delayed_work(&batt->poll_work, batt->report_interval * HZ);
	pmic_batt_debug("poll_work\n");
}

static void pmic_init_batt(struct pmic_batt_coulomb *batt)
{
	struct pmic_batt_info *info = &batt->info;

	/* init adc */
	pmic_batt_adc_init();
	/* get total coulomb first */
	info->total_coulomb = pmic_batt_get_total_coulomb();
	/* then current coulomb */
	info->curr_coulomb = pmic_batt_get_init_coulomb();
	info->min_coulomb = 0;

	/* 50 mill ohm */
	info->resistance = 50;
	/* adc poll time 1000ms */
	info->interval = 1000;
	/* suspend current 10mA */
	info->suspend_current = 10;

	/* uevent report interval */
	batt->report_interval = 5*60;

	pmic_batt_debug("init: curr=%u, min=%u, total=%u\n", info->curr_coulomb, info->min_coulomb, info->total_coulomb);
}

static int pmic_batt_online(void)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;

	return __battery_online(batt->master);
}

static int pmic_batt_volt_now(void)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;

	return __battery_voltage(batt->master);
}

static int pmic_batt_current_now(void)
{
	return pmic_get_curr();
}

static int pmic_batt_cap(void)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	struct pmic_batt_info *info = &batt->info;
	int coulomb, cap;

	mutex_lock(&batt->mutex);

	coulomb = clamp(info->curr_coulomb, info->min_coulomb, info->total_coulomb);

	cap = ((uint32_t)(coulomb - info->min_coulomb)) * 100 /
		(info->total_coulomb - info->min_coulomb);

	/* if 4 < capacity < 10 and volt < 3400mV, we believe the cap is about 4% */
	if (cap < 10 && cap > 4)
		if (__battery_voltage(batt->master) < 3400)
			info->curr_coulomb =
				((info->total_coulomb - info->min_coulomb) / 100) * 4 + info->min_coulomb;

	mutex_unlock(&batt->mutex);

	return cap;
}

static int pmic_batt_status(void)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	int cap;

	if (!__battery_online(batt->master))
		return POWER_SUPPLY_STATUS_UNKNOWN;

	/* ac <--> usb */
	if (__usb_online(batt->master)) {
		cap = pmic_batt_cap();
		if (cap >= 96)
			return POWER_SUPPLY_STATUS_FULL;
		else
			if (__is_batt_charged(batt->master))
				return POWER_SUPPLY_STATUS_CHARGING;
			else
				if (cap > 90)
					return POWER_SUPPLY_STATUS_FULL;
				else
					return POWER_SUPPLY_STATUS_NOT_CHARGING;
	} else
		return POWER_SUPPLY_STATUS_DISCHARGING;
}

static struct pmic_battery_ops axp192_battery_ops = {
	.batt_status			= pmic_batt_status,
	.batt_online			= pmic_batt_online,
	.batt_volt_now			= pmic_batt_volt_now,
	.batt_current_now		= pmic_batt_current_now,
	.batt_cap				= pmic_batt_cap,
};

static ssize_t adc_interval_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	struct pmic_batt_info *info = &batt->info;

	return sprintf(buf, "%d\n", info->interval);
}

static ssize_t adc_interval_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct pmic_batt_coulomb *batt = &pmic_batt;
	struct pmic_batt_info *info = &batt->info;
	long interval;
	int ret;

	mutex_lock(&batt->mutex);

	ret = strict_strtol(buf, 10, &interval);
	if (ret < 0 || interval <= 0) {
		mutex_unlock(&batt->mutex);
		return ret;
	}

	info->interval = interval;

	mutex_unlock(&batt->mutex);

	return n;
}

static DEVICE_ATTR(adc_interval, 0644, adc_interval_show, adc_interval_store);

static ssize_t curr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;
	struct pmic_batt_coulomb *batt = &pmic_batt;

	mutex_lock(&batt->mutex);
	ret = sprintf(buf, "%d\n", batt->info.curr);
	mutex_unlock(&batt->mutex);

	return ret;
}

static DEVICE_ATTR(curr, 0644, curr_show, NULL);

static ssize_t volt_a_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;
	struct pmic_batt_coulomb *batt = &pmic_batt;

	mutex_lock(&batt->mutex);
	ret = sprintf(buf, "%d\n", batt->info.volt_a);
	mutex_unlock(&batt->mutex);

	return ret;
}

static DEVICE_ATTR(volt_a, 0644, volt_a_show, NULL);

static ssize_t volt_b_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;
	struct pmic_batt_coulomb *batt = &pmic_batt;

	mutex_lock(&batt->mutex);
	ret = sprintf(buf, "%d\n", batt->info.volt_b);
	mutex_unlock(&batt->mutex);

	return ret;
}

static DEVICE_ATTR(volt_b, 0644, volt_b_show, NULL);

static ssize_t cap_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", pmic_batt_cap());
}

static DEVICE_ATTR(cap, 0644, cap_show, NULL);

static struct attribute *pmic_batt_attrs[] = {
	&dev_attr_adc_interval.attr,
	&dev_attr_curr.attr,
	&dev_attr_volt_a.attr,
	&dev_attr_volt_b.attr,
	&dev_attr_cap.attr,
	NULL,
};

static struct attribute_group pmic_batt_attr_group = {
	.name	= "pmic-batt",
	.attrs	= pmic_batt_attrs,
};

static int pmic_batt_probe(struct platform_device *pdev)
{
	struct pmic_batt_coulomb *batt;

	batt = &pmic_batt;

	batt->name = pdev->name;
	batt->master = pdev->dev.parent;
	batt->ops = &axp192_battery_ops;

	wake_lock_init(&batt->wakelock, WAKE_LOCK_SUSPEND, pdev->name);
	mutex_init(&batt->mutex);
	INIT_DELAYED_WORK(&batt->adc_work, pmic_adc_work_func);
	INIT_DELAYED_WORK(&batt->poll_work, pmic_poll_work_func);
	pmic_sysfs_register_battery(batt->ops);

	batt->event =
		EVENT_BATT_PLUGIN		|
		EVENT_BATT_PLUGOUT		|
		EVENT_BATT_ACTIVTE		|
		EVENT_BATT_DISACTIVE	|
		EVENT_BATT_CHARGING		|
		EVENT_BATT_CHARGE_END	|
		EVENT_BATT_HOT			|
		EVENT_BATT_COLD			|
		EVENT_LOW_POWER_WARNING	|
		0;
	batt->nb.notifier_call = pmic_batt_event;
	pmic_request_irq(batt->master, &batt->nb, batt->event);

	platform_set_drvdata(pdev, batt);

	pmic_init_batt(batt);

	if (batt->info.interval > 0)
		schedule_delayed_work(&batt->adc_work, msecs_to_jiffies(batt->info.interval));

	if (batt->report_interval > 0)
		schedule_delayed_work(&batt->poll_work, batt->report_interval * HZ);

	return sysfs_create_group(&pdev->dev.kobj, &pmic_batt_attr_group);
}

static int pmic_batt_remove(struct platform_device *pdev)
{
	struct pmic_batt_coulomb *batt = platform_get_drvdata(pdev);

	sysfs_remove_group(&pdev->dev.kobj, &pmic_batt_attr_group);
	pmic_free_irq(batt->master, &batt->nb, batt->event);
	pmic_sysfs_unregister_battery(batt->ops);
	cancel_delayed_work_sync(&batt->poll_work);
	cancel_delayed_work_sync(&batt->adc_work);

	return 0;
}

static int pmic_batt_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct pmic_batt_coulomb *batt = platform_get_drvdata(pdev);

	batt->sleep_time = get_seconds();

	return 0;
}

static int pmic_batt_resume(struct platform_device *pdev)
{
	struct pmic_batt_coulomb *batt = platform_get_drvdata(pdev);

	batt->sleep_time = get_seconds() - batt->sleep_time;
	batt->info.curr_coulomb += batt->sleep_time * batt->info.suspend_current;

	return 0;
}

static void pmic_batt_shutdown(struct platform_device *pdev)
{
	pmic_batt_save_coulomb();
}

static struct platform_driver pmic_batt_driver = {
	.probe		= pmic_batt_probe,
	.remove		= pmic_batt_remove,
	.suspend	= pmic_batt_suspend,
	.resume		= pmic_batt_resume,
	.shutdown	= pmic_batt_shutdown,
	.driver		= {
		.name	= "axp192-batt-coulomb",
		.owner	= THIS_MODULE,
	},
};

static int __init pmic_batt_init(void)
{
	return platform_driver_register(&pmic_batt_driver);
}
module_init(pmic_batt_init);

static void __exit pmic_batt_exit(void)
{
	platform_driver_unregister(&pmic_batt_driver);
}
module_exit(pmic_batt_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pmic battery driver");
