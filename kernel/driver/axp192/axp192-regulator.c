#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/axp192.h>

struct pmic_regulator_info {
	int min_uV;
	int max_uV;
	int step_uV;
	int volt_reg;
	int volt_shift;
	int volt_bits;
	int enable_reg;
	int enable_shift;
	int enable_bits;
	int enable_val;
	int disable_val;
};

struct pmic_regulator {
	int suspend;
	struct mutex lock;
	struct regulator_desc desc;
	struct device *dev;
	struct pmic_regulator_info info;
};

static int __pmic_regulator_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV)
{
	int ret, volt_uV, step;
	uint8_t val, mask;
	uint8_t selector;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	struct pmic_regulator_info *info = &regulator->info;

	if ((max_uV < info->min_uV) || (min_uV > info->max_uV))
		return -EINVAL;

	if (min_uV < info->min_uV)
		volt_uV = info->min_uV;
	else
		volt_uV = min_uV;

	ret = pmic_read(regulator->dev, info->volt_reg, &val);
	if (ret < 0)
		return ret;

	step = info->step_uV ? info->step_uV : 1;

	selector = ((volt_uV - info->min_uV + step - 1) / step)
			<< info->volt_shift;

	mask = (((1 << info->volt_bits) - 1) << info->volt_shift);

	if ((val & mask) == selector)
		return 0;

	val &= ~mask;
	val |= selector;

	ret = pmic_write(regulator->dev, info->volt_reg, val);
//	printk("%s %d reg[0x%02x]=0x%02x\n", __func__, volt_uV, info->volt_reg, val);

	return ret;
}

static int __pmic_regulator_get_voltage(struct regulator_dev *rdev)
{
	int ret;
	uint8_t val, mask;
	uint8_t selector;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	struct pmic_regulator_info *info = &regulator->info;

	ret = pmic_read(regulator->dev, info->volt_reg, &val);
	if (ret < 0)
		return ret;

	mask = ((1 << info->volt_bits) - 1);
	selector = (val >> info->volt_shift) & mask;

	ret = info->min_uV + selector * info->step_uV;
//	printk("%s %d \n", __func__, ret);

	return ret;
}

static int __pmic_regulator_enable(struct regulator_dev *rdev)
{
	int ret;
	uint8_t val, mask;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	struct pmic_regulator_info *info = &regulator->info;
	
	ret = pmic_read(regulator->dev, info->enable_reg, &val);
	if (ret < 0)
		return 0;

	mask = ((1 << info->enable_bits) - 1) << info->enable_shift;
	val &= ~mask;
	val |= info->enable_val << info->enable_shift;

	ret = pmic_write(regulator->dev, info->enable_reg, val);
//	printk("%s reg[0x%02x]=0x%02x\n", __func__, info->enable_reg, val);

	return ret;
}

static int __pmic_regulator_disable(struct regulator_dev *rdev)
{
	int ret;
	uint8_t val, mask;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	struct pmic_regulator_info *info = &regulator->info;
	
	ret = pmic_read(regulator->dev, info->enable_reg, &val);
	if (ret < 0)
		return 0;

	mask = ((1 << info->enable_bits) - 1) << info->enable_shift;
	val &= ~mask;
	val |= info->disable_val << info->enable_shift;

	ret = pmic_write(regulator->dev, info->enable_reg, val);
//	printk("%s reg[0x%02x]=0x%02x\n", __func__, info->enable_reg, val);

	return ret;
}

static int __pmic_regulator_is_enabled(struct regulator_dev *rdev)
{
	int ret;
	uint8_t val, mask;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	struct pmic_regulator_info *info = &regulator->info;

	ret = pmic_read(regulator->dev, info->enable_reg, &val);
	if (ret < 0)
		return ret;

	mask = ((1 << info->enable_bits) - 1) << info->enable_shift;
	ret = ((val & mask) == (info->enable_val << info->enable_shift));

	return ret;
}

static int pmic_regulator_list_voltage(struct regulator_dev *rdev, unsigned selector)
{
	int ret;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	struct pmic_regulator_info *info = &regulator->info;
	
	ret = info->min_uV + info->step_uV * selector;

	if (ret > info->max_uV)
		return -EINVAL;

	return ret;
}

static int pmic_regulator_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV)
{
	int ret;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);

	mutex_lock(&regulator->lock);
	if (regulator->suspend) {
		ret = -EBUSY;
		goto out;
	}
	ret = __pmic_regulator_set_voltage(rdev, min_uV, max_uV);
out:
	mutex_unlock(&regulator->lock);
	return ret;
}

static int pmic_regulator_get_voltage(struct regulator_dev *rdev)
{
	int ret;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);

	mutex_lock(&regulator->lock);
	ret = __pmic_regulator_get_voltage(rdev);
	mutex_unlock(&regulator->lock);

	return ret;
}

static int pmic_regulator_enable(struct regulator_dev *rdev)
{
	int ret;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	
	mutex_lock(&regulator->lock);
	ret = __pmic_regulator_enable(rdev);
	mutex_unlock(&regulator->lock);

	return ret;
}

static int pmic_regulator_disable(struct regulator_dev *rdev)
{
	int ret;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	
	mutex_lock(&regulator->lock);
	ret = __pmic_regulator_disable(rdev);
	mutex_unlock(&regulator->lock);

	return ret;
}

static int pmic_regulator_is_enabled(struct regulator_dev *rdev)
{
	int ret;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);
	
	mutex_lock(&regulator->lock);
	ret = __pmic_regulator_is_enabled(rdev);
	mutex_unlock(&regulator->lock);

	return ret;
}

static int pmic_regulator_set_suspend_voltage(struct regulator_dev *rdev, int uV)
{
	int ret;
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);

	mutex_lock(&regulator->lock);
	ret = __pmic_regulator_set_voltage(rdev, uV, uV);
	regulator->suspend = (ret < 0) ? regulator->suspend : 1;
	mutex_unlock(&regulator->lock);

	return ret;
}

static int pmic_regulator_enable_time(struct regulator_dev *rdev)
{
	return 10;
}

struct regulator_ops pmic_regulator_ops = {
	.list_voltage			= pmic_regulator_list_voltage,
	.set_voltage			= pmic_regulator_set_voltage,
	.get_voltage			= pmic_regulator_get_voltage,
	.enable					= pmic_regulator_enable,
	.disable				= pmic_regulator_disable,
	.is_enabled				= pmic_regulator_is_enabled,
	.set_suspend_enable		= pmic_regulator_enable,
	.set_suspend_disable	= pmic_regulator_disable,
	.set_suspend_voltage	= pmic_regulator_set_suspend_voltage,
	.enable_time			= pmic_regulator_enable_time,
};

#define PMIC_REGULATOR(									\
		_name, _id,										\
		_ops,											\
		min, max, step,									\
		vreg, vshift, vbits,							\
		ereg, eshift, ebits,							\
		eval, dval)										\
														\
	[_id]	= {											\
		.desc	= {										\
			.name		= #_name,						\
			.id		= _id,								\
			.n_voltages	= (step)?((max-min)/step+1):1,	\
			.ops		= _ops,							\
			.irq		= 0,							\
			.type		= REGULATOR_VOLTAGE,			\
			.owner		= THIS_MODULE,					\
		},												\
		.info	= {										\
			.min_uV		= min,							\
			.max_uV		= max,							\
			.step_uV	= step,							\
			.volt_reg	= vreg,							\
			.volt_shift	= vshift,						\
			.volt_bits	= vbits,						\
			.enable_reg	= ereg,							\
			.enable_shift	= eshift,					\
			.enable_bits	= ebits,					\
			.enable_val		= eval,						\
			.disable_val	= dval,						\
		},												\
	}

static struct pmic_regulator pmic_regulator[][7] = {
	/* [0] --> axp192 */
	{
		PMIC_REGULATOR(
				LDO0, PMIC_ID_LDO0,
				&pmic_regulator_ops,
				1800000, 3300000, 100000,
				0x91, 4, 4,
				0x90, 0, 3,
				2, 7),
		PMIC_REGULATOR(
				LDO1, PMIC_ID_LDO1,
				&pmic_regulator_ops,
				3300000, 3300000, 0,
				0x06, 0, 0,
				0x06, 0, 1,
				1, 0),
		PMIC_REGULATOR(
				LDO2, PMIC_ID_LDO2,
				&pmic_regulator_ops,
				1800000, 3300000, 100000,
				0x28, 4, 4,
				0x12, 2, 1,
				1, 0),
		PMIC_REGULATOR(
				LDO3, PMIC_ID_LDO3,
				&pmic_regulator_ops,
				1800000, 3300000, 100000,
				0x28, 0, 4,
				0x12, 3, 1,
				1, 0),
		PMIC_REGULATOR(
				DCDC1, PMIC_ID_DCDC1,
				&pmic_regulator_ops,
				700000, 3500000, 25000,
				0x26, 0, 7,
				0x12, 0, 1,
				1, 0),
		PMIC_REGULATOR(
				DCDC2, PMIC_ID_DCDC2,
				&pmic_regulator_ops,
				700000, 2275000, 25000,
				0x23, 0, 6,
				0x10, 0, 1,
				1, 0),
		PMIC_REGULATOR(
				DCDC3, PMIC_ID_DCDC3,
				&pmic_regulator_ops,
				700000, 3500000, 25000,
				0x27, 0, 7,
				0x12, 1, 1,
				1, 0),
	},
	/* [1] --> other */
	{},
};

static struct pmic_regulator *find_pmic_regulator(int type, int id)
{
	int i;
	struct pmic_regulator *regulator;

	for (i = 0; i < ARRAY_SIZE(pmic_regulator[type]); i++) {
		regulator = &pmic_regulator[type][id];
		if (id == regulator->desc.id)
			return regulator;
	}

	return NULL;
}

static int pmic_regulator_probe(struct platform_device *pdev)
{
	struct pmic_regulator *regulator;
	struct regulator_init_data *init_data = pdev->dev.platform_data;
	struct regulator_dev *rdev;
	int type;

	type = platform_get_device_id(pdev)->driver_data;

	regulator = find_pmic_regulator(type, pdev->id);
	if (!regulator) {
		dev_err(&pdev->dev, "Failed to find regulator %d\n", pdev->id);
		return -EINVAL;
	}

	regulator->dev = pdev->dev.parent;
	mutex_init(&regulator->lock);

	rdev = regulator_register(&regulator->desc, &pdev->dev, init_data, regulator);
	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "Failed to register regulator %s\n", regulator->desc.name);
		return PTR_ERR(rdev);
	}

	platform_set_drvdata(pdev, rdev);

	return 0;
}

static int pmic_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);

	regulator_unregister(rdev);

	return 0;
}

static int pmic_regulator_resume(struct device *dev)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct pmic_regulator *regulator = rdev_get_drvdata(rdev);

	regulator->suspend = 0;

	return 0;
}

static const struct dev_pm_ops pmic_regulator_pm = {
	.resume_noirq	= pmic_regulator_resume,
};

static struct platform_device_id pmic_regulator_id[] = {
	{"axp192-regulator", 0},
	{"unknown-regulator", 1},
	{}
};
MODULE_DEVICE_TABLE(platform, pmic_regulator_id);

static struct platform_driver pmic_regulator_driver = {
	.probe		= pmic_regulator_probe,
	.remove		= pmic_regulator_remove,
	.id_table	= pmic_regulator_id,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "pmic-regulator",
		.pm	= &pmic_regulator_pm,
	},
};

static __init int pmic_regulator_init(void)
{
	return platform_driver_register(&pmic_regulator_driver);
}

static __exit void pmic_regulator_exit(void)
{
	platform_driver_unregister(&pmic_regulator_driver);
}

subsys_initcall(pmic_regulator_init);
module_exit(pmic_regulator_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pmic regulator driver");
