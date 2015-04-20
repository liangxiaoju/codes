#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/mfd/axp192.h>

struct pmic_charger {
	int online;
	int state;
	int type;
	uint64_t event;
	struct device *master;
	struct led_trigger *led;
	struct wake_lock wakelock;
	struct delayed_work work;
	struct notifier_block nb;
	struct pmic_charger_ops *ops;
	struct pmic_charger_platform_data *pdata;
};

extern void pmic_sysfs_register_charger(struct pmic_charger_ops *ops);
extern void pmic_sysfs_unregister_charger(struct pmic_charger_ops *ops);
extern void pmic_sysfs_changed(void);

static struct pmic_charger pmic_charger;

static int pmic_enable_charger(struct pmic_charger *charger)
{
	int i;
	struct pmic_cfg *cfg;
	struct pmic_charger_platform_data *pdata = charger->pdata;

	wake_lock(&charger->wakelock);

	if (pdata->enable)
		pdata->enable();

	for (i = 0; i < pdata->enable_ncfgs; i++) {
		cfg = &pdata->enable_cfgs[i];
		pmic_write(charger->master, cfg->reg, cfg->val);
	}

	led_trigger_event(charger->led, LED_FULL);

	pmic_sysfs_changed();

	return 0;
}

static int pmic_disable_charger(struct pmic_charger *charger)
{
	int i;
	struct pmic_cfg *cfg;
	struct pmic_charger_platform_data *pdata = charger->pdata;

	if (pdata->disable)
		pdata->disable();

	for (i = 0; i < pdata->disable_ncfgs; i++) {
		cfg = &pdata->disable_cfgs[i];
		pmic_write(charger->master, cfg->reg, cfg->val);
	}

	led_trigger_event(charger->led, LED_OFF);

	pmic_sysfs_changed();

	wake_unlock(&charger->wakelock);

	return 0;
}

static int pmic_charger_event(struct notifier_block *n,
		unsigned long data, void *extra)
{
	struct pmic_charger *charger =
		container_of(n, struct pmic_charger, nb);
	uint64_t event = ((uint64_t)data) |
		(((uint64_t)((unsigned long)extra)) << 32);

	if (!(event & charger->event))
		return NOTIFY_DONE;

	if (event & (EVENT_VBUS_PLUGIN | EVENT_BATT_PLUGIN |
				EVENT_BATT_CHARGING))
		pmic_enable_charger(charger);

	else if (event & (EVENT_VBUS_PLUGOUT | EVENT_BATT_PLUGOUT |
				EVENT_BATT_HOT | EVENT_BATT_COLD | EVENT_CHIP_HOT))
		pmic_disable_charger(charger);

	if (event & (EVENT_ACIN_OVERVOLTAGE | EVENT_VBUS_OVERVOLTAGE |
				EVENT_CHIP_HOT))
		led_trigger_event(charger->led, HZ_4);

	return NOTIFY_OK;
}

static int axp192_usb_online(void)
{
	int ret;
	uint8_t val;
	struct pmic_charger *charger = &pmic_charger;

	ret = pmic_read(charger->master, AXP192_POWER_STATE, &val);
	if (ret < 0)
		return ret;

	return (val >> 5) & 0x01;
}

static int axp192_ac_online(void)
{
	int ret;
	uint8_t val;
	struct pmic_charger *charger = &pmic_charger;

	ret = pmic_read(charger->master, AXP192_POWER_STATE, &val);
	if (ret < 0)
		return ret;

	return (val >> 7) & 0x01;
}

static struct pmic_charger_ops axp192_charger_ops = {
	/* we exchange usb and ac for our board */
	.usb_online	= axp192_ac_online,
	.ac_online	= axp192_usb_online,
};

static void pmic_init_charger(struct pmic_charger *charger)
{
	struct pmic_charger_ops *ops = charger->ops;

	if (!ops)
		return;

	if (ops->usb_online() || ops->ac_online)
		pmic_enable_charger(charger);
	else
		pmic_disable_charger(charger);
}

static int pmic_charger_probe(struct platform_device *pdev)
{
	struct pmic_charger *charger;
	struct pmic_charger_platform_data *pdata = pdev->dev.platform_data;

	charger = &pmic_charger;

	charger->ops = &axp192_charger_ops;
	charger->master = pdev->dev.parent;
	charger->type	= platform_get_device_id(pdev)->driver_data;
	charger->pdata = pdata;

	wake_lock_init(&charger->wakelock, WAKE_LOCK_SUSPEND, pdev->name);

	led_trigger_register_simple(pdev->name, &charger->led);

	pmic_sysfs_register_charger(charger->ops);

	if (pdata->init)
		pdata->init();

	pmic_init_charger(charger);

	charger->event =
		EVENT_ACIN_PLUGIN		|
		EVENT_ACIN_PLUGOUT		|
		EVENT_VBUS_PLUGIN		|
		EVENT_VBUS_PLUGOUT		|
		EVENT_ACIN_OVERVOLTAGE	|
		EVENT_VBUS_OVERVOLTAGE	|
		EVENT_CHIP_HOT			|
		EVENT_BATT_HOT			|
		EVENT_BATT_COLD			|
		EVENT_BATT_PLUGIN		|
		EVENT_BATT_CHARGING		|
		0;
	charger->nb.notifier_call = pmic_charger_event;
	pmic_request_irq(charger->master, &charger->nb, charger->event);

	platform_set_drvdata(pdev, charger);

	return 0;
}

static int pmic_charger_remove(struct platform_device *pdev)
{
	struct pmic_charger *charger = platform_get_drvdata(pdev);

	pmic_free_irq(charger->master, &charger->nb, charger->event);
	led_trigger_unregister_simple(charger->led);
	pmic_sysfs_unregister_charger(charger->ops);

	return 0;
}

static struct platform_device_id pmic_charger_id[] = {
	{ "axp192-charger", 0 },
	{},
};

static struct platform_driver pmic_charger_driver = {
	.id_table	= pmic_charger_id,
	.probe		= pmic_charger_probe,
	.remove		= pmic_charger_remove,
	.driver		= {
		.name	= "pmic-charger",
		.owner	= THIS_MODULE,
	},
};

static int __init pmic_charger_init(void)
{
	return platform_driver_register(&pmic_charger_driver);
}
module_init(pmic_charger_init);

static void __exit pmic_charger_exit(void)
{
	platform_driver_unregister(&pmic_charger_driver);
}
module_exit(pmic_charger_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pmic charger driver");
