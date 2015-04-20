#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/wakelock.h>
#include <linux/notifier.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/mfd/axp192.h>

struct pmic_key_data {
	struct pmic_key *key;
	struct input_dev *input;
	struct delayed_work work;
	struct notifier_block irq_nb;
};

struct pmic_kpad {
	struct device *master;
	struct input_dev *input;
	struct wake_lock wake_lock;
	int nkeys;
	struct pmic_key_data kdata[0];
};

static void pmic_delayed_func(struct work_struct *work)
{
	struct pmic_key_data *kdata =
		container_of(to_delayed_work(work), struct pmic_key_data, work);
	struct pmic_kpad *kpad = input_get_drvdata(kdata->input);
	struct pmic_key *key = kdata->key;

	input_report_key(kdata->input, key->code, 0);
	input_sync(kdata->input);
	wake_unlock(&kpad->wake_lock);
	printk(KERN_INFO "%s over\n", key->name);
}

static int pmic_kpad_event(struct notifier_block *nb,
		unsigned long event, void *data)
{
	int ret = NOTIFY_DONE;
	struct pmic_key_data *kdata = container_of(nb, struct pmic_key_data, irq_nb);
	struct pmic_kpad *kpad = input_get_drvdata(kdata->input);
	struct pmic_key *key = kdata->key;

	if (event & key->event) {
		wake_lock(&kpad->wake_lock);
		input_report_key(kdata->input, key->code, 1);
		schedule_delayed_work(&kdata->work, msecs_to_jiffies(key->msec));
		ret = NOTIFY_OK;
		printk(KERN_INFO "%s start\n", key->name);
	}
	return ret;
}

static int __setup_pmic_kpad(struct pmic_kpad *kpad, struct pmic_key_data *kdata)
{
	struct pmic_key *key;
	struct input_dev *input;

	key = kdata->key;
	input = kdata->input;

	INIT_DELAYED_WORK(&kdata->work, pmic_delayed_func);

	kdata->irq_nb.notifier_call = pmic_kpad_event;
	pmic_request_irq(kpad->master, &kdata->irq_nb, key->event);

	input_set_capability(input, key->type, key->code);

	return 0;
}

static int pmic_kpad_probe(struct platform_device *pdev)
{
	int ret = 0, i;
	struct pmic_kpad *kpad;
	struct input_dev *input;
	struct pmic_key_platform_data *pdata = pdev->dev.platform_data;

	kpad = kzalloc(sizeof(struct pmic_kpad) +
			sizeof(struct pmic_key_data) * pdata->nkeys, GFP_KERNEL);
	input = input_allocate_device();
	if (!kpad || !input)
		return -ENOMEM;

	input->name = pdev->name;
	input->phys = "pmic-keys/input0";
	input->dev.parent = &pdev->dev;
	input->id.bustype = BUS_I2C;
	input->id.vendor = 0x0002;
	input->id.product = 0x0002;
	input->id.version = 0x0001;
	input_set_drvdata(input, kpad);

	kpad->input = input;
	kpad->master = pdev->dev.parent;
	kpad->nkeys = pdata->nkeys;

	printk("PMIC: adding keys.\n");
	for (i = 0; i < kpad->nkeys; i++) {
		struct pmic_key_data *kdata = &kpad->kdata[i];
		struct pmic_key *key = &pdata->keys[i];

		kdata->key = key;
		kdata->input = input;

		__setup_pmic_kpad(kpad, kdata);

		printk(KERN_INFO "\t[%d] --> %s\n", i, key->name);
	}

	wake_lock_init(&kpad->wake_lock, WAKE_LOCK_SUSPEND, pdev->name);

	ret = input_register_device(input);
	if (ret)
		goto err;

	device_init_wakeup(&pdev->dev, 1);

	platform_set_drvdata(pdev, kpad);

	return 0;

err:
	if (input)
		kfree(input);

	if (kpad)
		kfree(kpad);

	return ret;
}

/* TODO: */
static int pmic_kpad_remove(struct platform_device *pdev)
{
	struct pmic_kpad *kpad = platform_get_drvdata(pdev);

	device_init_wakeup(&pdev->dev, 0);
	input_unregister_device(kpad->input);
	wake_lock_destroy(&kpad->wake_lock);
	kfree(kpad);
	return 0;
}

static const struct platform_device_id pmic_kpad_id_table[] = {
	{"axp192-powerkey",	0},
	{},
};
MODULE_DEVICE_TABLE(platform, pmic_kpad_id_table);

static struct platform_driver pmic_kpad_driver = {
	.driver		= {
		.name	= "pmic-kpad",
		.owner	= THIS_MODULE,
	},
	.id_table	= pmic_kpad_id_table, 
	.probe		= pmic_kpad_probe,
	.remove		= pmic_kpad_remove,
};

static int __init pmic_kpad_init(void)
{
	return platform_driver_register(&pmic_kpad_driver);
}
static void __exit pmic_kpad_exit(void)
{
	platform_driver_unregister(&pmic_kpad_driver);
}

module_init(pmic_kpad_init);
module_exit(pmic_kpad_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pmic kpad driver");
