#ifndef __PMIC_BASE__
#define __PMIC_BASE__

#include <linux/i2c.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/leds.h>
#include "axp192_regs.h"

enum {
	PMIC_ID_LDO0 = 0,
	PMIC_ID_LDO1,
	PMIC_ID_LDO2,
	PMIC_ID_LDO3,
	PMIC_ID_DCDC1,
	PMIC_ID_DCDC2,
	PMIC_ID_DCDC3,
	PMIC_ID_LIGHT,
	PMIC_ID_SYSFS,
	PMIC_ID_BATT,
	PMIC_ID_CHARGER,
	PMIC_ID_KEY,
};

struct pmic_cfg {
	uint8_t reg;
	uint8_t val;
};

struct pmic_charger_platform_data {
	void (*init)(void);
	int enable_ncfgs;
	int disable_ncfgs;
	struct pmic_cfg *enable_cfgs;
	struct pmic_cfg *disable_cfgs;
	void (*enable)(void);
	void (*disable)(void);
};

struct pmic_battery_ops {
	int (*batt_status)(void);
	int (*batt_online)(void);
	int (*batt_volt_max_design)(void);
	int (*batt_volt_min_design)(void);
	int (*batt_volt)(void);
	int (*batt_volt_now)(void);
	int (*batt_current)(void);
	int (*batt_current_now)(void);
	int (*batt_cap)(void);
	int (*batt_temp)(void);
	int (*batt_power_now)(void);
};

struct pmic_charger_ops {
	int (*usb_online)(void);
	int (*ac_online)(void);
};

struct pmic_led {
	const char *name;
	const char *default_trigger;
	enum led_brightness	brightness;
};

struct pmic_light_platform_data {
	int nleds;
	struct pmic_led *leds;
	void (*init)(void);
};

struct pmic_key {
	const char *name;
	int code;
	int type;
	unsigned int msec;
	uint64_t event;
};

struct pmic_key_platform_data {
	int nkeys;
	struct pmic_key *keys;
};

#define PMIC_SUBDEV(_name, _id, _pdata)		\
	[_id]	= {								\
		.id = _id,							\
		.name = _name,						\
		.platform_data = _pdata,			\
	}

struct pmic_subdev {
	int id;
	char *name;
	void *platform_data;
};

struct pmic_platform_data {
	int ncfgs;
	struct pmic_cfg *cfgs;
	int nsubdevs;
	struct pmic_subdev *subdevs;
	unsigned int irq_gpio;
	void (*init)(void);
};

struct pmic_io_ops {
	int (*read)(struct i2c_client *dev, uint8_t reg, uint8_t *val);
	int (*reads)(struct i2c_client *dev, uint8_t reg, uint8_t *val, int len);
	int (*write)(struct i2c_client *dev, uint8_t reg, uint8_t val);
	int (*writes)(struct i2c_client *dev, uint8_t reg, uint8_t *val, int len);
	int (*setbits)(struct i2c_client *dev, uint8_t reg, uint8_t bmask);
	int (*clrbits)(struct i2c_client *dev, uint8_t reg, uint8_t bmask);
	int (*unmask_events)(struct i2c_client *dev, uint64_t events);
	int (*mask_events)(struct i2c_client *dev, uint64_t events);
	int (*read_events)(struct i2c_client *dev, uint64_t *events);
};

struct pmic_chip {
	const char *name;
	struct i2c_client *client;
	struct mutex lock;
	struct wake_lock wakelock;
	struct device *dev;
	const struct pmic_io_ops *ops;
	struct work_struct irq_work;
	struct blocking_notifier_head notifier_list;
	struct pmic_platform_data *pdata;
};


static inline int pmic_read(struct device *dev, uint8_t reg, uint8_t *val)
{
 	int ret = -EINVAL;
 	struct i2c_client *client = to_i2c_client(dev);	
	struct pmic_chip * chip = i2c_get_clientdata(client);

 	if (chip && chip->ops && chip->ops->read)
		ret = chip->ops->read(client,reg,val);

	return ret;
}

static inline int pmic_reads(struct device *dev, uint8_t reg, uint8_t *val, int len)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->reads)
		ret = chip->ops->reads(client,reg,val,len);

	return ret;
}

static inline int pmic_write(struct device *dev, uint8_t reg, uint8_t val)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->write)
		ret = chip->ops->write(client,reg,val);
	
	return ret;
}

static inline int pmic_writes(struct device *dev, uint8_t reg, uint8_t *val, int len)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->writes)
		ret = chip->ops->writes(client,reg,val,len);
	
	return ret;
}

static inline int pmic_setbits(struct device *dev, uint8_t reg, uint8_t bmask)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->setbits)
		ret = chip->ops->setbits(client,reg,bmask);
	
	return ret;
}

static inline int pmic_clrbits(struct device *dev, uint8_t reg, uint8_t bmask)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->clrbits)
		ret = chip->ops->clrbits(client,reg,bmask);
	
	return ret;
}

static inline int pmic_read_events(struct device *dev, uint64_t *events)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->read_events)
		ret = chip->ops->read_events(client,events);
	
	return ret;
}

static inline int pmic_mask_events(struct device *dev, uint64_t events)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->mask_events)
		ret = chip->ops->mask_events(client,events);
	
	return ret;
}

static inline int pmic_unmask_events(struct device *dev, uint64_t events)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->unmask_events)
		ret = chip->ops->unmask_events(client,events);
	
	return ret;
}

static inline int pmic_request_irq(struct device *dev, struct notifier_block *nb, uint64_t events)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->unmask_events) {
		ret = chip->ops->unmask_events(client,events);
		if (ret < 0)
			return ret;
	}

	blocking_notifier_chain_register(&chip->notifier_list, nb);
	
	return ret;
}

static inline int pmic_free_irq(struct device *dev, struct notifier_block *nb, uint64_t events)
{
	int ret = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct pmic_chip * chip = i2c_get_clientdata(client);

	if (chip && chip->ops && chip->ops->mask_events) {
		ret = chip->ops->mask_events(client,events);
		if (ret < 0)
			return ret;
	}

	blocking_notifier_chain_unregister(&chip->notifier_list, nb);
	
	return ret;
}

#endif
