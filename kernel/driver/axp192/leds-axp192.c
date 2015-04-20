#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/mfd/axp192.h>

struct pmic_led_data {
	struct pmic_led *led;
	struct pmic_light *light;
	struct led_classdev	cdev;
	struct work_struct	work;
	int (*set_brightness)(struct pmic_led_data *);
};

struct pmic_light_method {
	int (*set_brightness)(struct pmic_led_data *led_data);
};

struct pmic_light {
	struct device		*master;
	struct mutex		lock;
	int nleds;
	struct pmic_led_data led_data[0];
};

static int unknown_set_brightness(struct pmic_led_data *led_data)
{
	printk(KERN_INFO "unknown_set_brightness %d\n", led_data->led->brightness);
	return 0;
}

static int axp192_set_brightness(struct pmic_led_data *led_data)
{
	int ret = 0;
	uint8_t val, led_status;
	struct pmic_light *light = led_data->light;
	struct pmic_led *led = led_data->led;

	if (!mutex_trylock(&light->lock))
		return -1;

	switch (led->brightness) {
		case LED_FULL:
			led_status = AXP192_LED_ON;
			break;
		case LED_OFF:
			led_status = AXP192_LED_OFF;
			break;
		case 1:
			led_status = AXP192_LED_1HZ;
			break;
		case 4:
			led_status = AXP192_LED_4HZ;
			break;
		default:
			goto exit;
	}

	ret = pmic_read(light->master, POWER_OFF_SETTING, &val);
	if (ret)
		goto exit;

	val &= ~(0x03<<4);
	val |= led_status | (1 << 3);

	ret = pmic_write(light->master, POWER_OFF_SETTING, val);
	if (ret)
		goto exit;

	printk(KERN_INFO "%s %d reg[0x%02x]=0x%02x\n", __func__, led->brightness, POWER_OFF_SETTING, val);

exit:
	mutex_unlock(&light->lock);
	return ret;
}

static void led_func(struct work_struct *work)
{
	struct pmic_led_data *led_data =
		container_of(work, struct pmic_led_data, work);

	led_data->set_brightness(led_data);
}

static void pmic_set_led(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct pmic_led_data *led_data =
		container_of(led_cdev, struct pmic_led_data, cdev);
	struct pmic_led *led = led_data->led;

	led->brightness = value;
	schedule_work(&led_data->work);
}

static void setup_pmic_led(struct device *parent, struct pmic_led_data *led_data)
{
	struct led_classdev *cdev = &led_data->cdev;
	struct pmic_led *led = led_data->led;

	cdev->name = led->name;
	cdev->default_trigger = led->default_trigger;
	cdev->brightness = LED_OFF;
	cdev->brightness_set = pmic_set_led;

	led_classdev_register(parent, cdev);

	INIT_WORK(&led_data->work, led_func);
}

static struct pmic_light_method pmic_light_method[] = {
	{
		.set_brightness = axp192_set_brightness,
	},
	{
		.set_brightness = unknown_set_brightness,
	},
};

static int pmic_light_probe(struct platform_device *pdev)
{
	int type, i;
	struct pmic_light *light;
	struct pmic_light_platform_data *pdata = pdev->dev.platform_data;

	light = kzalloc(sizeof(struct pmic_light) +
			sizeof(struct pmic_led_data) * pdata->nleds, GFP_KERNEL);
	if (!light)
		return -ENOMEM;

	light->nleds = pdata->nleds;
	light->master = pdev->dev.parent;
	mutex_init(&light->lock);

	type = platform_get_device_id(pdev)->driver_data;

	if (pdata->init)
		pdata->init();

	printk(KERN_INFO "PMIC: adding leds.\n");
	for (i = 0; i < pdata->nleds; i++) {
		struct pmic_led_data *led_data = &light->led_data[i];
		struct pmic_led *led = &pdata->leds[i];

		led_data->led = led;
		led_data->set_brightness = pmic_light_method[type].set_brightness;

		led_data->light = light;

		setup_pmic_led(&pdev->dev, led_data);
		printk(KERN_INFO "[%d] --> %s\n", i, led->name);
	}

	platform_set_drvdata(pdev, light);

	return 0;
}

/* TODO: */
static int pmic_light_remove(struct platform_device *pdev)
{
	struct pmic_light *light = platform_get_drvdata(pdev);

	kfree(light);

	return 0;
}

static const struct platform_device_id pmic_light_id[] = {
	{"axp192-light", 0},
	{"unknown-light", 1},
	{},
};
MODULE_DEVICE_TABLE(platform, pmic_light_id);

static struct platform_driver pmic_light_driver = {
	.id_table	= pmic_light_id,
	.probe		= pmic_light_probe,
	.remove		= pmic_light_remove,
	.driver		= {
		.name		= "pmic-light",
		.owner		= THIS_MODULE,
	},
};

static int __init pmic_light_init(void)
{
	return platform_driver_register(&pmic_light_driver);
}

static void __exit pmic_light_exit(void)
{
 	platform_driver_unregister(&pmic_light_driver);
}

module_init(pmic_light_init);
module_exit(pmic_light_exit);

MODULE_DESCRIPTION("pmic light driver");
MODULE_LICENSE("GPL");
