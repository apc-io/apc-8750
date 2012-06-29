/*
 *  linux/drivers/mmc/core/sdio_bus.c
 *
 *  Copyright 2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * SDIO function driver model
 */

#include <linux/device.h>
#include <linux/err.h>

#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>

#include "sdio_cis.h"
#include "sdio_bus.h"

#ifdef CONFIG_MMC_EMBEDDED_SDIO
#include <linux/mmc/host.h>
#endif

#if 0
#define DBG(x...)	printk(KERN_ALERT x)
#else
#define DBG(x...)	do { } while (0)
#endif
/* show configuration fields */
#define sdio_config_attr(field, format_string)				\
static ssize_t								\
field##_show(struct device *dev, struct device_attribute *attr, char *buf)				\
{									\
	struct sdio_func *func;						\
									\
	func = dev_to_sdio_func (dev);					\
	return sprintf (buf, format_string, func->field);		\
}

sdio_config_attr(class, "0x%02x\n");
sdio_config_attr(vendor, "0x%04x\n");
sdio_config_attr(device, "0x%04x\n");

static ssize_t modalias_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sdio_func *func = dev_to_sdio_func (dev);
	DBG("[%s] \n",__func__);
	return sprintf(buf, "sdio:c%02Xv%04Xd%04X\n",
			func->class, func->vendor, func->device);
}

static struct device_attribute sdio_dev_attrs[] = {
	__ATTR_RO(class),
	__ATTR_RO(vendor),
	__ATTR_RO(device),
	__ATTR_RO(modalias),
	__ATTR_NULL,
};

static const struct sdio_device_id *sdio_match_one(struct sdio_func *func,
	const struct sdio_device_id *id)
{
	DBG("[%s] s\n",__func__);
	if (id->class != (__u8)SDIO_ANY_ID && id->class != func->class) {
		DBG("[%s] e1\n",__func__);
		return NULL;
	}
	if (id->vendor != (__u16)SDIO_ANY_ID && id->vendor != func->vendor) {
		DBG("[%s] e2\n",__func__);
		return NULL;
	}
	if (id->device != (__u16)SDIO_ANY_ID && id->device != func->device) {
		DBG("[%s] e3\n",__func__);
		return NULL;
	}
	DBG("[%s] e4\n",__func__);
	return id;
}

static const struct sdio_device_id *sdio_match_device(struct sdio_func *func,
	struct sdio_driver *sdrv)
{
	const struct sdio_device_id *ids;
	DBG("[%s] s\n",__func__);
	
	ids = sdrv->id_table;

	if (ids) {
		while (ids->class || ids->vendor || ids->device) {
			if (sdio_match_one(func, ids)) {
				DBG("[%s] e1\n",__func__);
				return ids;
			}
			ids++;
		}
	}

	DBG("[%s] e2\n",__func__);
	return NULL;
}

static int sdio_bus_match(struct device *dev, struct device_driver *drv)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	struct sdio_driver *sdrv = to_sdio_driver(drv);

	if (sdio_match_device(func, sdrv)) {
		DBG("[%s] e1\n",__func__);
		return 1;
	}

	DBG("[%s] e2\n",__func__);
	return 0;
}

static int
sdio_bus_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	DBG("[%s] s\n",__func__);

	if (add_uevent_var(env,
			"SDIO_CLASS=%02X", func->class)) {
		DBG("[%s] e1\n",__func__);
		return -ENOMEM;
	}

	if (add_uevent_var(env, 
			"SDIO_ID=%04X:%04X", func->vendor, func->device)) {
		DBG("[%s] e2\n",__func__);
		return -ENOMEM;
	}

	if (add_uevent_var(env,
			"MODALIAS=sdio:c%02Xv%04Xd%04X",
			func->class, func->vendor, func->device)) {
		DBG("[%s] e3\n",__func__);
		return -ENOMEM;
	}

	DBG("[%s] e4\n",__func__);
	return 0;
}

static int sdio_bus_probe(struct device *dev)
{
	struct sdio_driver *drv = to_sdio_driver(dev->driver);
	struct sdio_func *func = dev_to_sdio_func(dev);
	const struct sdio_device_id *id;
	int ret;

	DBG("[%s] s\n",__func__);
	id = sdio_match_device(func, drv);
	if (!id) {
		DBG("[%s] e1\n",__func__);
		return -ENODEV;
	}

	/* Set the default block size so the driver is sure it's something
	 * sensible. */
	sdio_claim_host(func);
	ret = sdio_set_block_size(func, 0);
	sdio_release_host(func);
	if (ret) {
		DBG("[%s] e2\n",__func__);
		return ret;
	}
	DBG("[%s] e3\n",__func__);
	return drv->probe(func, id);
}

static int sdio_bus_remove(struct device *dev)
{
	struct sdio_driver *drv = to_sdio_driver(dev->driver);
	struct sdio_func *func = dev_to_sdio_func(dev);
	
	DBG("[%s] s\n",__func__);
	drv->remove(func);

	if (func->irq_handler) {
		printk(KERN_WARNING "WARNING: driver %s did not remove "
			"its interrupt handler!\n", drv->name);
		sdio_claim_host(func);
		sdio_release_irq(func);
		sdio_release_host(func);
	}
	DBG("[%s] e1\n",__func__);
	return 0;
}

static struct bus_type sdio_bus_type = {
	.name		= "sdio",
	.dev_attrs	= sdio_dev_attrs,
	.match		= sdio_bus_match,
	.uevent		= sdio_bus_uevent,
	.probe		= sdio_bus_probe,
	.remove		= sdio_bus_remove,
};

int sdio_register_bus(void)
{
	int ret;
	DBG("[%s] s\n",__func__);
	
	ret = bus_register(&sdio_bus_type);
	
	DBG("[%s] e\n",__func__);
	return ret;
}

void sdio_unregister_bus(void)
{
	DBG("[%s] s\n",__func__);
	bus_unregister(&sdio_bus_type);
	DBG("[%s] e\n",__func__);
}

/**
 *	sdio_register_driver - register a function driver
 *	@drv: SDIO function driver
 */
int sdio_register_driver(struct sdio_driver *drv)
{
	int ret;
	DBG("[%s] s\n",__func__);
	drv->drv.name = drv->name;
	drv->drv.bus = &sdio_bus_type;
	ret = driver_register(&drv->drv);
	
	DBG("[%s] e\n",__func__);
	return ret;
}
EXPORT_SYMBOL_GPL(sdio_register_driver);

/**
 *	sdio_unregister_driver - unregister a function driver
 *	@drv: SDIO function driver
 */
void sdio_unregister_driver(struct sdio_driver *drv)
{
	DBG("[%s] s\n",__func__);
	drv->drv.bus = &sdio_bus_type;
	driver_unregister(&drv->drv);
	DBG("[%s] e\n",__func__);
}
EXPORT_SYMBOL_GPL(sdio_unregister_driver);

static void sdio_release_func(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	DBG("[%s] s\n",__func__);
	
#ifdef CONFIG_MMC_EMBEDDED_SDIO
	/*
	 * If this device is embedded then we never allocated
	 * cis tables for this func
	 */
	if (!func->card->host->embedded_sdio_data.funcs)
#endif
		sdio_free_func_cis(func);

	if (func->info)
		kfree(func->info);

	kfree(func);
	
	DBG("[%s] e\n",__func__);
}

/*
 * Allocate and initialise a new SDIO function structure.
 */
struct sdio_func *sdio_alloc_func(struct mmc_card *card)
{
	struct sdio_func *func;
	DBG("[%s] s\n",__func__);
	
	func = kzalloc(sizeof(struct sdio_func), GFP_KERNEL);
	if (!func) {
		DBG("[%s] e1\n",__func__);
		return ERR_PTR(-ENOMEM);
	}

	func->card = card;

	device_initialize(&func->dev);

	func->dev.parent = &card->dev;
	func->dev.bus = &sdio_bus_type;
	func->dev.release = sdio_release_func;

	DBG("[%s] e2\n",__func__);
	return func;
}

/*
 * Register a new SDIO function with the driver model.
 */
int sdio_add_func(struct sdio_func *func)
{
	int ret;
	DBG("[%s] s\n",__func__);
	
	dev_set_name(&func->dev, "%s:%d", mmc_card_id(func->card), func->num);

	ret = device_add(&func->dev);
	if (ret == 0)
		sdio_func_set_present(func);

	DBG("[%s] e\n",__func__);
	return ret;
}

/*
 * Unregister a SDIO function with the driver model, and
 * (eventually) free it.
 */
void sdio_remove_func(struct sdio_func *func)
{
	DBG("[%s] s\n",__func__);
	if (sdio_func_present(func))
		device_del(&func->dev);

	put_device(&func->dev);
	DBG("[%s] e\n",__func__);
}

