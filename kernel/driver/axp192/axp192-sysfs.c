#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/mfd/axp192.h>

struct pmic_sysfs {
	const char *name;
	struct power_supply batt;
	struct power_supply usb;
	struct power_supply ac;
	struct pmic_battery_ops *b_ops;
	struct pmic_charger_ops *c_ops;
};

static struct pmic_sysfs pmic_sysfs;

void pmic_sysfs_register_charger(struct pmic_charger_ops *ops)
{
	pmic_sysfs.c_ops = ops;
}
EXPORT_SYMBOL_GPL(pmic_sysfs_register_charger);

void pmic_sysfs_unregister_charger(struct pmic_charger_ops *ops)
{
	if (pmic_sysfs.c_ops == ops)
		pmic_sysfs.c_ops = NULL;
}
EXPORT_SYMBOL_GPL(pmic_sysfs_unregister_charger);

void pmic_sysfs_register_battery(struct pmic_battery_ops *ops)
{
	pmic_sysfs.b_ops = ops;
}
EXPORT_SYMBOL_GPL(pmic_sysfs_register_battery);

void pmic_sysfs_unregister_battery(struct pmic_battery_ops *ops)
{
	if (pmic_sysfs.b_ops == ops)
		pmic_sysfs.b_ops = NULL;
}
EXPORT_SYMBOL_GPL(pmic_sysfs_unregister_battery);

void pmic_sysfs_changed(void)
{
//	power_supply_changed(&pmic_sysfs.usb);
	power_supply_changed(&pmic_sysfs.ac);
	power_supply_changed(&pmic_sysfs.batt);
}
EXPORT_SYMBOL_GPL(pmic_sysfs_changed);

static enum power_supply_property pmic_sysfs_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int pmic_sysfs_get_usb_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct pmic_sysfs *sysfs =
		container_of(psy, struct pmic_sysfs, usb);
	struct pmic_charger_ops *ops = sysfs->c_ops;

	val->intval = 0;
	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			if (ops && ops->usb_online)
				val->intval = ops->usb_online();
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static enum power_supply_property pmic_sysfs_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int pmic_sysfs_get_ac_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct pmic_sysfs *sysfs =
		container_of(psy, struct pmic_sysfs, ac);
	struct pmic_charger_ops *ops = sysfs->c_ops;

	val->intval = 0;
	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			if (ops && ops->ac_online)
				val->intval = ops->ac_online();
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static enum power_supply_property pmic_sysfs_batt_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
};

static int pmic_sysfs_get_batt_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct pmic_sysfs *sysfs =
		container_of(psy, struct pmic_sysfs, batt);
	struct pmic_battery_ops *ops = sysfs->b_ops;

	val->intval = 0;
	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			if (ops && ops->batt_status)
				val->intval = ops->batt_status();
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			if (ops && ops->batt_online)
				val->intval = ops->batt_online();
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
			if (ops && ops->batt_volt_max_design)
				val->intval = ops->batt_volt_max_design();
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
			if (ops && ops->batt_volt_min_design)
				val->intval = ops->batt_volt_min_design();
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_AVG:
			/* android use microvolts instead of millivolts */
			if (ops && ops->batt_volt)
				val->intval = ops->batt_volt() * 1000;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			if (ops && ops->batt_volt_now)
				val->intval = ops->batt_volt_now() * 1000;
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			if (ops && ops->batt_current_now)
				val->intval = ops->batt_current_now();
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			if (ops && ops->batt_cap)
				val->intval = ops->batt_cap();
			break;
		case POWER_SUPPLY_PROP_TEMP:
			if (ops && ops->batt_temp)
				val->intval = ops->batt_temp();
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int pmic_sysfs_setup_psy(struct device *parent, struct pmic_sysfs *sysfs)
{
	int ret = 0;
	struct power_supply *psy;

	psy = &sysfs->usb;
	psy->name = "usb";
	psy->type = POWER_SUPPLY_TYPE_USB;
	psy->properties = pmic_sysfs_usb_props;
	psy->num_properties = ARRAY_SIZE(pmic_sysfs_usb_props);
	psy->get_property = pmic_sysfs_get_usb_property;

	psy = &sysfs->ac;
	psy->name = "ac";
	psy->type = POWER_SUPPLY_TYPE_MAINS;
	psy->properties = pmic_sysfs_ac_props;
	psy->num_properties = ARRAY_SIZE(pmic_sysfs_ac_props);
	psy->get_property = pmic_sysfs_get_ac_property;
//	psy->supplied_to = supply_list;
//	psy->num_supplicants = ARRAY_SIZE(supply_list);

	psy = &sysfs->batt;
	psy->name = "battery";
	psy->type = POWER_SUPPLY_TYPE_BATTERY;
	psy->properties = pmic_sysfs_batt_props;
	psy->num_properties = ARRAY_SIZE(pmic_sysfs_batt_props);
	psy->get_property = pmic_sysfs_get_batt_property;

//	ret = power_supply_register(parent, &sysfs->usb);
	if (ret)
		return ret;
	ret = power_supply_register(parent, &sysfs->ac);
	if (ret)
		goto err_psy_ac;
	ret = power_supply_register(parent, &sysfs->batt);
	if (ret)
		goto err_psy_batt;

	return 0;

err_psy_batt:
	power_supply_unregister(&sysfs->ac);
err_psy_ac:
	power_supply_unregister(&sysfs->usb);
	return ret;
}

static int pmic_sysfs_probe(struct platform_device *pdev)
{
	struct pmic_sysfs *sysfs = &pmic_sysfs;

	sysfs->name = pdev->name;

	pmic_sysfs_setup_psy(&pdev->dev, sysfs);

	platform_set_drvdata(pdev, sysfs);

	return 0;
}

static int pmic_sysfs_remove(struct platform_device *pdev)
{
	struct pmic_sysfs *sysfs = platform_get_drvdata(pdev);

	power_supply_unregister(&sysfs->batt);
	power_supply_unregister(&sysfs->ac);
	power_supply_unregister(&sysfs->usb);

	return 0;
}

static struct platform_driver pmic_sysfs_driver = {
	.probe		= pmic_sysfs_probe,
	.remove		= pmic_sysfs_remove,
	.driver		= {
		.name	= "pmic-sysfs",
		.owner	= THIS_MODULE,
	},
};

static int __init pmic_sysfs_init(void)
{
	return platform_driver_register(&pmic_sysfs_driver);
}
module_init(pmic_sysfs_init);

static void __exit pmic_sysfs_exit(void)
{
	platform_driver_unregister(&pmic_sysfs_driver);
}
module_exit(pmic_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pmic sysfs driver");
