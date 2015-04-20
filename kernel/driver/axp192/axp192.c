#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mfd/axp192.h>

//#define AXP192_I2C_TEST
/************************ axp192 *********************************/
static int axp192_write(struct i2c_client *client,
		uint8_t reg, uint8_t val)
{
#ifdef AXP192_I2C_TEST
	printk("axp192_write:\n");
	printk("REG[%02x]=%02x\n", reg, val);
	return 0;
#else
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writing 0x%02x to 0x%02x\n", val, reg);
		return ret;
	}
	return 0;
#endif
}

static int axp192_writes(struct i2c_client *client,
		uint8_t reg, uint8_t *val, int len)
{
#ifdef AXP192_I2C_TEST
	int i;
	printk("axp192_writes:\n");
	for (i = 0; i < len; i++)
		printk("REG[%02x]=%02x\n", reg + i, val[i]);
	return 0;
#else
	int ret, i;
	unsigned char *data;

	data = kzalloc(len * 2, GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "%s: failed to alloc mem\n",__func__);
		return -ENOMEM;
	}
	
	for (i = 0; i < len; i++) {
		data[2 * i] = reg + i;
		data[2 * i + 1] = val[i];
	}
	
	ret = i2c_master_send(client, data, len * 2);

	kfree(data);

	if (ret != (len * 2)) {
		dev_err(&client->dev, "failed while axp192_writes.\n");
		return -EIO;
	}

	return 0;
#endif
}

static int axp192_read(struct i2c_client *client,
		uint8_t reg,uint8_t *val)
{
#ifdef AXP192_I2C_TEST
	memset(val, 0, 1);
	printk("axp192_read:\n");
	printk("REG[%02x]=%02x\n", reg, *val);
	return 0;
#else
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading at 0x%02x\n", reg);
		return ret;
	}

	*val = (uint8_t)ret;
	return 0;
#endif
}

static int axp192_reads(struct i2c_client *client,
		uint8_t reg, uint8_t *val, int len)
{
#ifdef AXP192_I2C_TEST
	int i;
	memset(val, 0, len);
	printk("axp192_reads:\n");
	for (i = 0; i < len; i++)
		printk("REG[%02x]=%02x\n", reg + i, val[i]);
	return 0;
#else
	int ret;

	ret = i2c_smbus_read_i2c_block_data(client,reg,len,val);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading from 0x%02x\n", reg);
		return ret;
	}
	return 0;
#endif
}

static int axp192_setbits(struct i2c_client *client,
		uint8_t reg, uint8_t bmask)
{
	int ret = 0;
	uint8_t reg_val;
	struct pmic_chip *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	ret = axp192_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & bmask) != bmask) {
		reg_val |= bmask;
		ret = axp192_write(client, reg, reg_val);
	}
out:
	mutex_unlock(&chip->lock);
	return ret;
}

static int axp192_clrbits(struct i2c_client *client,
		uint8_t reg, uint8_t bmask)
{
	int32_t ret = 0;
	uint8_t reg_val;
	struct pmic_chip *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	ret = axp192_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if (reg_val & bmask) {
		reg_val &= ~bmask;
		ret = axp192_write(client, reg, reg_val);
	}
out:
	mutex_unlock(&chip->lock);
	return ret;
}

static int axp192_unmask_events(struct i2c_client *client,
		uint64_t events)
{
	int ret;
	uint8_t val[5];
	uint64_t enable;

	ret = axp192_reads(client, 0x40, val, 4);
	if (ret)
		goto out;
	ret = axp192_read(client, 0x4a, &val[4]);
	if (ret)
		goto out;

	enable = ((uint64_t)val[0]) | ((uint64_t)val[1]<<8) |
		((uint64_t)val[2]<<16) | ((uint64_t)val[3]<<24) |
		((uint64_t)val[4]<<32);
	events |= enable;

	val[0] = (events >> 0) & 0xff;
	val[1] = (events >> 8) & 0xff;
	val[2] = (events >> 16) & 0xff;
	val[3] = (events >> 24) & 0xff;
	val[4] = (events >> 32) & 0xff;

	ret = axp192_writes(client, 0x40, val, 4);
	if (ret)
		goto out;
	ret = axp192_write(client, 0x4a, val[4]);
out:
	return ret;
}

static int axp192_mask_events(struct i2c_client *client,
		uint64_t events)
{
	int ret;
	uint8_t val[5];
	uint64_t enable;

	ret = axp192_reads(client, 0x40, val, 4);
	if (ret)
		goto out;
	ret = axp192_read(client, 0x4a, &val[4]);
	if (ret)
		goto out;

	enable = ((uint64_t)val[0]) | ((uint64_t)val[1]<<8) |
		((uint64_t)val[2]<<16) | ((uint64_t)val[3]<<24) |
		((uint64_t)val[4]<<32);
	events &= ~enable;

	val[0] = (events >> 0) & 0xff;
	val[1] = (events >> 8) & 0xff;
	val[2] = (events >> 16) & 0xff;
	val[3] = (events >> 24) & 0xff;
	val[4] = (events >> 32) & 0xff;

	ret = axp192_writes(client, 0x40, val, 4);
	if (ret)
		goto out;
	ret = axp192_write(client, 0x4a, val[4]);
out:
	return ret;
}

static int axp192_read_events(struct i2c_client *client,
		uint64_t *events)
{
	int ret;
	uint8_t val[5];

	ret = axp192_reads(client, 0x44, val, 4);
	if (ret)
		return ret;
	ret = axp192_read(client, 0x4d, &val[4]);
	if (ret)
		return ret;

	ret = axp192_writes(client, 0x44, val, 4);
	if (ret)
		return ret;
	ret = axp192_write(client, 0x4d, val[4]);
	if (ret)
		return ret;

	*events = ((uint64_t)val[0]) | ((uint64_t)val[1]<<8) |
		((uint64_t)val[2]<<16) | ((uint64_t)val[3]<<24) |
		((uint64_t)val[4]<<32);

	return 0;
}
/************************ axp192 *********************************/

static const struct pmic_io_ops pmic_io_ops[] = {
	[0]	= {
		.read			= axp192_read,
		.reads			= axp192_reads,
		.write			= axp192_write,
		.writes			= axp192_writes,
		.setbits		= axp192_setbits,
		.clrbits		= axp192_clrbits,
		.read_events	= axp192_read_events,
		.mask_events	= axp192_mask_events,
		.unmask_events	= axp192_unmask_events,
	},
};

static void pmic_irq_work(struct work_struct *work)
{
	struct pmic_chip *chip =
		container_of(work, struct pmic_chip, irq_work);
	uint64_t events = 0;

	if (pmic_read_events(chip->dev, &events))
		goto irq_work_out;

	printk("PMIC IRQ events: 0x%016llx\n", events);
	blocking_notifier_call_chain(
			&chip->notifier_list,
			(unsigned long)events,
			((void *)((unsigned long)(events>>32))));

irq_work_out:
	enable_irq(chip->client->irq);
	wake_unlock(&chip->wakelock);
}

static irqreturn_t pmic_irq_handler(int irq, void *data)
{
	struct pmic_chip *chip = data;

	wake_lock(&chip->wakelock);
	disable_irq_nosync(irq);
	schedule_work(&chip->irq_work);

	return IRQ_HANDLED;
}

static int __remove_subdev(struct device *dev, void *unused)
{
	platform_device_unregister(to_platform_device(dev));
	return 0;
}

static void pmic_del_subdevs(struct pmic_chip *chip)
{
	device_for_each_child(chip->dev, NULL, __remove_subdev);
}

static int pmic_add_subdevs(struct pmic_chip *chip,
			struct pmic_platform_data *pdata)
{
	int ret, i;
	struct pmic_subdev *subdev;
	struct platform_device *pdev;

	printk(KERN_INFO "\e[32mPMIC: adding subdevs\e[0m\n");
	for (i = 0; i < pdata->nsubdevs; i++) {
		subdev = &(pdata->subdevs[i]);

		pdev = platform_device_alloc(subdev->name, subdev->id);

		pdev->dev.parent = chip->dev;
		pdev->dev.platform_data = subdev->platform_data;

		ret = platform_device_add(pdev);
		if (ret)
			goto err;
		printk(KERN_INFO " [%d] --> %s\n", subdev->id, subdev->name);
	}
	return 0;

err:
	pmic_del_subdevs(chip);
	return ret;
}

static void pmic_init_chip(struct pmic_chip *chip)
{
	int i;
	struct pmic_platform_data *pdata = chip->pdata;
	int num = pdata->ncfgs;
	struct pmic_cfg *cfgs = pdata->cfgs;

	for (i = 0; i < num; i++)
		pmic_write(chip->dev, cfgs[i].reg, cfgs[i].val);
}

static const struct i2c_device_id pmic_id_table[] = {
	{"axp192",	0},
	{},
};
MODULE_DEVICE_TABLE(i2c, pmic_id_table);

static int pmic_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret = 0;
	struct pmic_platform_data *pdata = client->dev.platform_data;
	struct pmic_chip *chip;

	printk(KERN_INFO "## PMIC Probe.");
	chip = kzalloc(sizeof(struct pmic_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->name = pmic_id_table[id->driver_data].name;
	chip->client = client;
	chip->pdata = pdata;
	chip->dev = &client->dev;
	chip->ops = &pmic_io_ops[id->driver_data];
	mutex_init(&chip->lock);
	INIT_WORK(&chip->irq_work, pmic_irq_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&chip->notifier_list);
	wake_lock_init(&chip->wakelock, WAKE_LOCK_SUSPEND, chip->name);

	i2c_set_clientdata(client, chip);

	/* init reg */
	pmic_init_chip(chip);

	/* init gpio */
	if (pdata->init)
		pdata->init();

	ret = gpio_request(pdata->irq_gpio, chip->name);
	if (ret)
		goto err_request_gpio;

	ret = request_irq(client->irq, pmic_irq_handler,
			IRQF_DISABLED | IRQF_TRIGGER_LOW,
			chip->name, chip);
	if (ret)
		goto err_request_irq;

	ret = pmic_add_subdevs(chip, pdata);
	if (ret) {
		goto err_add_subdevs;
	}

	enable_irq_wake(client->irq);
	device_init_wakeup(&client->dev, 1);

	return 0;

err_add_subdevs:
	free_irq(client->irq, chip);
err_request_irq:
	gpio_free(pdata->irq_gpio);
err_request_gpio:
	kfree(chip);
	return ret;
}

static int pmic_remove(struct i2c_client *client)
{
	struct pmic_chip *chip = i2c_get_clientdata(client);
	struct pmic_platform_data *pdata = client->dev.platform_data;

	device_init_wakeup(&client->dev, 0);
	pmic_del_subdevs(chip);
	free_irq(client->irq, chip);
	gpio_free(pdata->irq_gpio);
	kfree(chip);

	return 0;
}

static int pmic_suspend(struct i2c_client *client, pm_message_t state)
{
	if (device_may_wakeup(&client->dev))
		enable_irq_wake(client->irq);

	disable_irq(client->irq);
	return 0;
}

static int pmic_resume(struct i2c_client *client)
{
	if (device_may_wakeup(&client->dev))
		disable_irq_wake(client->irq);

	enable_irq(client->irq);
	return 0;
}

static struct i2c_driver pmic_driver = {
	.driver = {
		.name   = "pmic",
		.owner  = THIS_MODULE,
	},
	.id_table	= pmic_id_table, 
	.probe		= pmic_probe,
	.remove		= __devexit_p(pmic_remove),
#ifdef CONFIG_PM
	.suspend	= pmic_suspend,
	.resume		= pmic_resume,
#endif
};

static int __init pmic_init(void)
{
	return i2c_add_driver(&pmic_driver);
}
subsys_initcall(pmic_init);

static void __exit pmic_exit(void)
{
	i2c_del_driver(&pmic_driver);
}
module_exit(pmic_exit);

MODULE_DESCRIPTION("PMIC driver");
MODULE_LICENSE("GPL");
