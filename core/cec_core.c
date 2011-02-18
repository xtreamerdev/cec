#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include "cec.h"
#include "cec_dev.h"

/*------------------------------------------------------------------
 * Func : cec_bus_match
 *
 * Desc : cec dev exit function
 *
 * Parm : dev    : device
 *        driver : driver
 *         
 * Retn : 1 : device & driver matched, 0 : device & driver not matched
 *------------------------------------------------------------------*/
int cec_bus_match(
    struct device*          dev, 
    struct device_driver*   drv
    )
{        
    // cec bus have no idea to match device & driver, return 1 to pass all     
    return 1;
}


/*------------------------------------------------------------------
 * Func : cec_bus_suspend
 *
 * Desc : suspend cec dev
 *
 * Parm : dev    : device to be suspend 
 *        state  : event : power state
 *               PM_EVENT_ON      0
 *               PM_EVENT_FREEZE  1
 *               PM_EVENT_SUSPEND 2
 *         
 * Retn : 0
 *------------------------------------------------------------------*/
int cec_bus_suspend(struct device * dev, pm_message_t state)
{    
    cec_device* p_dev = to_cec_device(dev);
    cec_driver* p_drv = to_cec_driver(dev->driver);    
    return (p_drv->suspend) ? p_drv->suspend(p_dev) : 0;
}


/*------------------------------------------------------------------
 * Func : cec_bus_resume
 *
 * Desc : resume cec bus
 *
 * Parm : dev    : device to be resumed 
 *         
 * Retn : 0
 *------------------------------------------------------------------*/
int cec_bus_resume(struct device * dev)
{
    cec_device* p_dev = to_cec_device(dev);
    cec_driver* p_drv = to_cec_driver(dev->driver);    
    return (p_drv->resume) ? p_drv->resume(p_dev) : 0;
}



struct bus_type cec_bus_type = {
    .name    = "cec",
    .match   = cec_bus_match,    
    .suspend = cec_bus_suspend,    
    .resume  = cec_bus_resume,    
};



/*------------------------------------------------------------------
 * Func : register_cec_device
 *
 * Desc : register cec device
 *
 * Parm : device    : cec device to be registered
 *         
 * Retn : N/A
 *------------------------------------------------------------------*/
void cec_device_release(struct device* dev)
{    
    cec_device* p_dev = to_cec_device(dev);
    
    printk("cec dev %s released\n", p_dev->name);    
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            cec_device                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

static int cec_device_count = 0;

/*------------------------------------------------------------------
 * Func : register_cec_device
 *
 * Desc : register cec device
 *
 * Parm : device    : cec device to be registered
 *         
 * Retn : 0
 *------------------------------------------------------------------*/
int register_cec_device(cec_device* device)
{
    struct device* dev = &device->dev;
    
    printk("register cec device '%s' (%p) to cec%d\n", device->name, dev, cec_device_count);
    
    sprintf(dev->bus_id, "cec%d", cec_device_count++);       
    
    dev->bus = &cec_bus_type;
    
    dev->release  = cec_device_release;                    
    
    return device_register(dev);
}




/*------------------------------------------------------------------
 * Func : unregister_cec_device
 *
 * Desc : unregister cec device
 *
 * Parm : device : cec device to be unregistered
 *         
 * Retn : N/A
 *------------------------------------------------------------------*/
void unregister_cec_device(cec_device* device)
{    
    device_unregister(&device->dev);
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            cec_driver                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


/*------------------------------------------------------------------
 * Func : cec_drv_probe
 *
 * Desc : probe cec device
 *
 * Parm : dev : cec device to be probed
 *         
 * Retn : 0 : if supportted, others : failed
 *------------------------------------------------------------------*/
int cec_drv_probe(struct device * dev)
{
    cec_device* p_dev = to_cec_device(dev);    
    cec_driver* p_drv = to_cec_driver(dev->driver);    
    printk("probe : cec_dev '%s' (%p), cec_drv '%s' (%p)\n", p_dev->name, dev,p_drv->name, dev->driver);    
    
    if (!p_drv->probe)
        return -ENODEV;
        
    if (p_drv->probe(p_dev)==0)        
    {
        create_cec_dev_node(p_dev);
        return 0;    
    }

    return -ENODEV;
}



/*------------------------------------------------------------------
 * Func : cec_drv_remove
 *
 * Desc : this function was used to inform that a cec device has been 
 *        removed
 *
 * Parm : dev : cec device to be removed
 *         
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
int cec_drv_remove(struct device * dev)
{
    cec_device* p_dev = to_cec_device(dev);    
    cec_driver* p_drv = to_cec_driver(dev->driver);
    
    printk("remove cec_dev '%s'\n", p_dev->name);
    
    if (p_drv->remove) 
        p_drv->remove(p_dev);
        
    remove_cec_dev_node(p_dev);
        
    return 0;
}



/*------------------------------------------------------------------
 * Func : cec_drv_shutdown
 *
 * Desc : this function was used to short down a cec device 
 *
 * Parm : dev : cec device to be shut down
 *         
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
void cec_drv_shutdown(struct device * dev)
{
    cec_device* p_dev = to_cec_device(dev);    
    cec_driver* p_drv = to_cec_driver(dev->driver);    
    
    printk("shutdown cec_dev '%s'\n", p_dev->name);            
    
    if (p_drv->enable)
        p_drv->enable(p_dev, 0);    
}



/*------------------------------------------------------------------
 * Func : cec_drv_suspend
 *
 * Desc : this function was used to suspend a cec device 
 *
 * Parm : dev : cec device to be suspend
 *         
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
int cec_drv_suspend(struct device * dev, pm_message_t state, u32 level)
{
    cec_device* p_dev = to_cec_device(dev);    
    cec_driver* p_drv = to_cec_driver(dev->driver);    
    
    printk("suspend cec_dev '%s'\n", p_dev->name);
    return (p_drv->suspend) ? p_drv->suspend(p_dev) : 0;
}


/*------------------------------------------------------------------
 * Func : cec_drv_resume
 *
 * Desc : this function was used to resume a cec device 
 *
 * Parm : dev : cec device to be suspend
 *         
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
int cec_drv_resume(struct device * dev, u32 level)
{
    cec_device* p_dev = to_cec_device(dev);    
    cec_driver* p_drv = to_cec_driver(dev->driver);    
    
    printk("resume cec_dev '%s'\n", p_dev->name);
        
    return (p_drv->resume) ? p_drv->resume(p_dev) : 0;    
}


/*------------------------------------------------------------------
 * Func : register_cec_driver
 *
 * Desc : register cec device driver
 *
 * Parm : driver    : cec device driver to be registered
 *         
 * Retn : 0
 *------------------------------------------------------------------*/
int register_cec_driver(cec_driver* driver)
{
    struct device_driver* drv = &driver->drv;
    
    drv->name     = driver->name;
    drv->bus      = &cec_bus_type;
    drv->probe    = cec_drv_probe;
    drv->remove   = cec_drv_remove;
    drv->shutdown = cec_drv_shutdown;
    drv->suspend  = cec_drv_suspend;
    drv->resume   = cec_drv_resume;
    
    printk("register cec driver '%s' (%p)\n", drv->name, drv);
        
    return driver_register(drv);
}



/*------------------------------------------------------------------
 * Func : unregister_cec_driver
 *
 * Desc : unregister cec driver
 *
 * Parm : driver : cec driver to be unregistered
 *         
 * Retn : N/A
 *------------------------------------------------------------------*/
void unregister_cec_driver(cec_driver* driver)
{   
    struct device_driver* drv = &driver->drv;    
    printk("unregister cec driver '%s' (%p)\n", drv->name, &driver->drv); 
    driver_unregister(&driver->drv);
}

#ifdef CONFIG_CEC
#error WTFFF
#endif
#if (defined(CONFIG_CEC_CHARDEV) || defined(WITH_CEC_CHARDEV))
extern int cec_dev_module_init(void);
extern void cec_dev_module_exit(void);
#endif
#if (defined(CONFIG_MARS_CEC) || defined(WITH_MARS_CEC))
extern int mars_cec_module_init(void);
extern void mars_cec_module_exit(void);
#endif 

/*------------------------------------------------------------------
 * Func : cec_dev_module_init
 *
 * Desc : cec dev init function
 *
 * Parm : N/A
 *         
 * Retn : 0 : success, others fail  
 *------------------------------------------------------------------*/
static int __init cec_core_init(void)
{                 
    int ret;
    printk("%s, register cec_bus %p\n",__FUNCTION__, &cec_bus_type);
	    
    ret = bus_register(&cec_bus_type);     // register cec bus type	        
    if (ret) return ret;
#if (defined(CONFIG_CEC_CHARDEV) || defined(WITH_CEC_CHARDEV))
    ret = cec_dev_module_init();
    if (ret) {
	bus_unregister(&cec_bus_type);
	return ret;
    }
#if (defined(CONFIG_MARS_CEC) || defined(WITH_MARS_CEC))
    ret = mars_cec_module_init();
    if (ret) {
	cec_dev_module_exit();
	bus_unregister(&cec_bus_type);
	return ret;
    }
#endif
#endif
   return 0;	// success
}



/*------------------------------------------------------------------
 * Func : cec_dev_module_exit
 *
 * Desc : cec dev module exit function
 *
 * Parm : N/A
 *         
 * Retn : 0 : success, others fail  
 *------------------------------------------------------------------*/
static void __exit cec_core_exit(void)
{

#if (defined(CONFIG_CEC_CHARDEV) || defined(WITH_CEC_CHARDEV))
#if (defined(CONFIG_MARS_CEC) || defined(WITH_MARS_CEC))
    mars_cec_module_exit();
#endif
    cec_dev_module_exit();
#endif
    bus_unregister(&cec_bus_type);        // unregister cec bus
}

module_init(cec_core_init);
module_exit(cec_core_exit); 
/* required to avoid "Unknown symbol" errors on insmod */
MODULE_LICENSE("GPL");

/* Required by the cec_mars module if multiple modules are used for CEC */
EXPORT_SYMBOL_GPL(register_cec_driver);
EXPORT_SYMBOL_GPL(unregister_cec_driver);
EXPORT_SYMBOL_GPL(register_cec_device);
EXPORT_SYMBOL_GPL(unregister_cec_device);

