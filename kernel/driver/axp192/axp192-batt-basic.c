#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/mfd/axp192.h>

#include "axp192-common.c"

struct pmic_batt {
	const char *name;
	int state;
	uint64_t event;
	struct device *master;
	struct mutex mutex;
	struct wake_lock wakelock;
	struct notifier_block nb;
	struct pmic_battery_ops *ops;
};

extern void pmic_sysfs_register_battery(struct pmic_battery_ops *ops);
extern void pmic_sysfs_unregister_battery(struct pmic_battery_ops *ops);
extern void pmic_sysfs_changed(void);

static struct pmic_batt pmic_batt;

static int pmic_batt_status(void)
{
	struct pmic_batt *batt = &pmic_batt;

	if (!__battery_online(batt->master))
		return POWER_SUPPLY_STATUS_UNKNOWN;

	/* ac <--> usb */
	if (__usb_online(batt->master))
		if (__is_batt_charged(batt->master))
			return POWER_SUPPLY_STATUS_CHARGING;
		else
			return POWER_SUPPLY_STATUS_NOT_CHARGING;
	else
		return POWER_SUPPLY_STATUS_DISCHARGING;
}

static int pmic_batt_online(void)
{
	struct pmic_batt *batt = &pmic_batt;

	return __battery_online(batt->master);
}

static int pmic_batt_volt_max_design(void)
{
	return 4200;
}

static int pmic_batt_volt_min_design(void)
{
	return 3300;
}

static int pmic_batt_volt_now(void)
{
	struct pmic_batt *batt = &pmic_batt;

	return __battery_voltage(batt->master);
}

static int pmic_batt_current_now(void)
{
	struct pmic_batt *batt = &pmic_batt;

	return __battery_current(batt->master);
}

static int pmic_batt_cap(void)
{
	struct pmic_batt *batt = &pmic_batt;
	int volt, max, min;

	volt = __battery_voltage(batt->master);
	max = pmic_batt_volt_max_design();
	min = pmic_batt_volt_min_design();

	if (volt > max)
		volt = max;
	if (volt < min)
		volt = min;
	return (volt - min) * 100 / (max - min);
}

static int pmic_batt_temp(void)
{
	return 25 * 10;
}

static int pmic_batt_power_now(void)
{
	struct pmic_batt *batt = &pmic_batt;

	return __battery_power(batt->master);
}

static struct pmic_battery_ops axp192_battery_ops = {
	.batt_status			= pmic_batt_status,
	.batt_online			= pmic_batt_online,
	.batt_volt_max_design	= pmic_batt_volt_max_design,
	.batt_volt_min_design	= pmic_batt_volt_min_design,
	.batt_volt_now			= pmic_batt_volt_now,
	.batt_current_now		= pmic_batt_current_now,
	.batt_cap				= pmic_batt_cap,
	.batt_temp				= pmic_batt_temp,
	.batt_power_now			= pmic_batt_power_now,
};

static int pmic_batt_event(struct notifier_block *n,
		unsigned long data, void *extra)
{
	struct pmic_batt *batt =
		container_of(n, struct pmic_batt, nb);
	uint64_t event = ((uint64_t)data) |
		(((uint64_t)((unsigned long)extra)) << 32);

	if (!(event & batt->event))
		return NOTIFY_DONE;

	wake_lock_timeout(&batt->wakelock, 3 * HZ);

	pmic_sysfs_changed();

	return NOTIFY_OK;
}

static int pmic_batt_probe(struct platform_device *pdev)
{
	struct pmic_batt *batt;

	batt = &pmic_batt;

	batt->name = pdev->name;
	batt->master = pdev->dev.parent;
	batt->ops = &axp192_battery_ops;

	wake_lock_init(&batt->wakelock, WAKE_LOCK_SUSPEND, pdev->name);
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

	return 0;
}

static int pmic_batt_remove(struct platform_device *pdev)
{
	struct pmic_batt *batt = platform_get_drvdata(pdev);

	pmic_free_irq(batt->master, &batt->nb, batt->event);
	pmic_sysfs_unregister_battery(batt->ops);

	return 0;
}

static struct platform_driver pmic_batt_driver = {
	.probe		= pmic_batt_probe,
	.remove		= pmic_batt_remove,
	.driver		= {
		.name	= "axp192-batt-base",
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
