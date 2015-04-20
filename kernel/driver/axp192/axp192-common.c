/*
 * pmic axp192 common api
 */

static inline void __axp192_poweroff(struct device *dev)
{
	printk(KERN_EMERG "%s\n", __func__);

	/*
	 * set led control by charger 
	 */
	pmic_clrbits(dev, POWER_OFF_SETTING, (1<<3));
	/*
	 * poweroff
	 */
	pmic_setbits(dev, POWER_OFF_SETTING, (1<<7));

	while(1);

	printk(KERN_EMERG "%s : should not reach here!\n", __func__);
}

static inline int __gpio1_voltage(struct device *dev)
{
	int ret;
	uint8_t high_bit, low_bit, range;
	int volt;

	ret = pmic_read(dev, GPIO1_VOLTAGE_HIGH_8, &high_bit);
	if (ret)
		goto err;

	ret = pmic_read(dev, GPIO1_VOLTAGE_LOW_4, &low_bit);
	if (ret)
		goto err;

	volt = (high_bit << 4) | (low_bit & 0xf);
	volt=volt / 2;

	ret = pmic_read(dev, ADC_INPUT_RANGE, &range);
	if (ret)
		goto err;

	range = !!(range & 1<<1);
	volt += 700 * range;

	return volt;
err:
	printk(KERN_ERR "Failed to read gpio1 voltage\n");
	return ret;
}

static inline int __is_batt_charged(struct device *dev)
{
	int ret;
	uint8_t val;

	ret = pmic_read(dev, AXP192_POWER_MODE, &val);
	if (ret < 0)
		goto err;

	return (val >> 6) & 0x01;
err:
	return ret;
}

static inline int __battery_online(struct device *dev)
{
	int ret;
	uint8_t val;

	ret = pmic_read(dev, AXP192_POWER_MODE, &val);
	if (ret < 0)
		goto err;

	return (val >> 5) & 0x01;
err:
	printk(KERN_ERR "Failed to read battery status\n");
	return ret;
}

static inline int __battery_voltage(struct device *dev)
{
	int ret;
	uint8_t high_bit, low_bit;
	int volt;

	ret = pmic_read(dev, BATT_VOLTAGE_HIGH_8, &high_bit);
	if (ret)
		goto err;

	ret = pmic_read(dev, BATT_VOLTAGE_LOW_4, &low_bit);
	if (ret)
		goto err;

	volt = (high_bit << 4) | (low_bit & 0xf);
	volt=volt * 11;
	volt=volt / 10;

	return volt;
err:
	printk(KERN_ERR "Failed to read battery voltage\n");
	return ret;
}

static inline int __battery_current(struct device *dev)
{
	int ret, charge;
	uint8_t high_bit, low_bit, reg_high, reg_low, power_state;
	uint32_t curr;

	ret = pmic_read(dev, AXP192_POWER_STATE, &power_state);
	if (ret)
		goto err;

	charge = (power_state >> 2) & 0x01;

	if (charge) {
		reg_high = BATT_CHARGE_CURRENT_HIGH_8;
		reg_low = BATT_CHARGE_CURRENT_LOW_5;
		/*if charging return negtive ,input*/
		charge = -1;
	} else {
		reg_high = BATT_DISCHARGE_CURRENT_HIGH_8;
		reg_low = BATT_DISCHARGE_CURRENT_LOW_5;
		charge = 1;
	}
	
	ret = pmic_read(dev, reg_high, &high_bit);
	if (ret)
		goto err;

	ret = pmic_read(dev, reg_low, &low_bit);
	if (ret)
		goto err;

	curr = (high_bit << 5) | (low_bit & 0x1f);
	curr = curr / 2;

	return curr * charge;

err:
	printk(KERN_ERR "Failed to read battery current\n");
	return ret;
}

static inline int __battery_power(struct device *dev)
{
	uint8_t high_bit, mid_bit, low_bit;
	int batt_power;

	pmic_read(dev, BATT_INSTANT_POWER_HIGH_8, &high_bit);
	pmic_read(dev, BATT_INSTANT_POWER_MID_8, &mid_bit);
	pmic_read(dev, BATT_INSTANT_POWER_LOW_8, &low_bit);

	batt_power = (high_bit << 16) | (mid_bit << 8) | (low_bit);
	batt_power *= 11;
	batt_power /= 10;
	batt_power >>= 1;

	return batt_power;
}

#include <linux/mfd/axp192_data.h>
static inline int __battery_capacity(struct device *dev)
{
	search_table_t *table;
	unsigned int upper, lower;
	int volt, charged;
	int size;

	volt = __battery_voltage(dev);
	charged = __is_batt_charged(dev);

	if (charged) {
		table = CV_charge_table_s;
		size = ARRAY_SIZE(CV_charge_table_s);
	} else {
		table = CV_discharge_table_s;
		size = ARRAY_SIZE(CV_discharge_table_s);
	}

	upper = 0;
	lower = size - 1;

	return search_table_return(table, volt, &upper, &lower);
}

static inline int __usb_online(struct device *dev)
{
	int ret;
	uint8_t val;

	ret = pmic_read(dev, AXP192_POWER_STATE, &val);
	if (ret < 0)
		goto err;

	return (val >> 5) & 0x01;
err:
	printk(KERN_ERR "Failed to read usb status\n");
	return ret;
}

static inline int __usb_voltage(struct device *dev)
{
	int ret;
	uint8_t high_bit, low_bit;
	int volt;
	
	ret = pmic_read(dev, VBUS_VOLTAGE_HIGH_8, &high_bit);
	if (ret)
		goto err;
	
	ret = pmic_read(dev, VBUS_VOLTAGE_LOW_4, &low_bit);
	if (ret)
		goto err;

	volt = (high_bit << 4) | (low_bit & 0x0f);
	volt = volt * 17;
	volt = volt / 10;

	return volt;
err:
	printk(KERN_ERR "Failed to read usb voltage\n");
	return ret;
}

static inline int __usb_current(struct device *dev)
{
	int ret;
	uint8_t high_bit, low_bit;
	int curr;
	
	ret = pmic_read(dev, VBUS_CURRENT_HIGH_8, &high_bit);
	if (ret)
		goto err;
	
	ret = pmic_read(dev, VBUS_CURRENT_LOW_4, &low_bit);
	if (ret)
		goto err;

	curr = (high_bit << 4) | (low_bit & 0x0f);
	curr = curr * 375;
	curr = curr / 1000;

	return curr;
err:
	printk(KERN_ERR "Failed to read usb voltage\n");
	return ret;
}

static inline int __ac_online(struct device *dev)
{
	int ret;
	uint8_t val;
	ret = pmic_read(dev, AXP192_POWER_STATE, &val);
	if (ret < 0)
		goto err;

	return (val >> 7) & 0x01;
err:
	printk(KERN_ERR "Failed to read ac status\n");
	return ret;
}

static inline int __ac_voltage(struct device *dev)
{
	int ret;
	uint8_t high_bit, low_bit;
	int volt;

	ret = pmic_read(dev, ACIN_VOLTAGE_HIGH_8, &high_bit);
	if (ret)
		goto err;
	
	ret = pmic_read(dev, ACIN_VOLTAGE_LOW_4, &low_bit);
	if (ret)
		goto err;

	volt = (high_bit << 4) | (low_bit & 0x0f);
	volt = volt * 17;
	volt = volt / 10;

	return volt;
err:
	printk(KERN_INFO "Failed to read ac voltage\n");
	return ret;
}

static inline int __ac_current(struct device *dev)
{
	int ret;
	uint8_t high_bit, low_bit;
	int curr;
	
	ret = pmic_read(dev, ACIN_CURRENT_HIGH_8, &high_bit);
	if (ret)
		goto err;
	
	ret = pmic_read(dev, ACIN_CURRENT_LOW_4, &low_bit);
	if (ret)
		goto err;

	curr = (high_bit << 4) | (low_bit & 0x0f);
	curr = curr * 625;
	curr = curr / 1000;

	return curr;
err:
	printk(KERN_ERR "Failed to read ac current\n");
	return ret;
}

static inline int __chip_temperature(struct device *dev)
{
	int ret;
	uint8_t high_bit;
	uint8_t low_bit;
	unsigned int temp;

	ret = pmic_read(dev, INNER_TEMPERATURE_HIGH_8, &high_bit);
	if (ret)
		goto err;

	ret = pmic_read(dev, INNER_TEMPERATURE_LOW_4, &low_bit);
	if (ret)
		goto err;

	temp = (high_bit << 4) | (low_bit & 0x0f);
	temp = temp - 1447;

	return temp;
err:
	printk(KERN_ERR "Failed to read chip temperature\n");
	return ret;
}

/* timeout unit: minute */
static inline int __timer_ctrl(struct device *dev, int timeout)
{
	int ret;

	ret = pmic_write(dev, ADC_TIMER_CONTROL, 1 << 7);
	if (ret)
		goto err;

	ret = pmic_write(dev, ADC_TIMER_CONTROL, timeout && 0xff);

err:
	printk(KERN_ERR "Failed to control timer.\n");
	return ret;
}
