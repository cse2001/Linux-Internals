/**
 * @file ioctl_mem.c
 * @brief Kernel module implementing an ioctl device driver.
 *
 * This module provides functionality for two ioctl operations:
 * 1. It provides the physical address for a given virtual address. The physical-to-virtual
 *    address translation is performed for the current running process.
 * 2. It writes a byte value to the memory address corresponding to a given physical address.
 *
 * Author: Aratrik Chandra
 *
 * References:
 * - Linux Kernel Source Code: https://elixir.bootlin.com/linux/v6.1/source/
 * - Linux Kernel Documentation: https://www.kernel.org/doc/html/latest/
 * - Memory Management in Linux Kernel: https://www.kernel.org/doc/html/latest/admin-guide/mm/index.html
 * - ioctl related: https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html
 *                  https://github.com/pokitoz/ioctl_driver
 *                  https://www.opensourceforu.com/2011/08/io-control-in-linux/
 *                  https://tldp.org/LDP/lkmpg/2.4/html/x856.html
 *                  https://lwn.net/Articles/119652/
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <asm/io.h>
#include "ioctl_mem.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_DESCRIPTION("ioctl device driver which 1. provides the physical address for a given virtual address. (The physical to virtual-address translation should be done for the current running process.) 2. Given a physical address and a byte value, it writes the value at the memory address.");


static int major_number; ///< Major number assigned to the device driver

/**
 * @brief Retrieves the physical address corresponding to a given virtual address.
 *
 * This function performs the physical-to-virtual address translation for the current running process.
 * 
 * @param mm Pointer to the mm_struct representing the memory descriptor of the current process.
 * @param virt_addr Virtual address for which the physical address is requested.
 * @param phys_addr Pointer to store the physical address.
 * @return Returns 0 on success, or an error code otherwise.
 */
static int get_phys_addr(struct mm_struct *mm, unsigned long virt_addr, unsigned long *phys_addr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    pgd = pgd_offset(mm, virt_addr);
    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
    {
        return -EFAULT;
    }

    p4d = p4d_offset(pgd, virt_addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d))
    {
        return -EFAULT;
    }
    pud = pud_offset(p4d, virt_addr);
    if (pud_none(*pud) || unlikely(pud_bad(*pud)))
    {
        return -EFAULT;
    }

    pmd = pmd_offset(pud, virt_addr);
    if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
    {
        return -EFAULT;
    }

    pte = pte_offset_kernel(pmd, virt_addr);
    if (!pte || pte_none(*pte))
    {
        return -EFAULT;
    }

    *phys_addr  = page_to_pfn(pte_page(*pte)) << PAGE_SHIFT | offset_in_page(virt_addr);

    printk(KERN_INFO "Physical address : %lx\n", *phys_addr);
    return 0;
}

/**
 * @brief Writes a byte value to the memory address corresponding to a given physical address.
 *
 * This function writes a byte value to the memory address corresponding to the provided physical address.
 * 
 * @param phys_addr Physical address where the value needs to be written.
 * @param value Pointer to the byte value to be written.
 * @return Returns 0 on success, or an error code otherwise.
 */static int write_to_physical_addr(unsigned long phys_addr, unsigned char *value)
{
    void __iomem *mem = ioremap(phys_addr, sizeof(unsigned char));
    if (!mem)
        return -EFAULT;

    // Write to the physical address
    writeb(*value, mem);

    iounmap(mem);

    return 0;
}


/**
 * @brief Handles the IOCTL_GET_PHYSICAL_ADDR command.
 *
 * This function retrieves the physical address corresponding to a given virtual address.
 * The physical-to-virtual address translation is performed for the current running process.
 * 
 * @param arg Argument associated with the IOCTL_GET_PHYSICAL_ADDR command.
 * @return Returns 0 on success, or an error code otherwise.
 */
static int ioctl_get_physical_addr(unsigned long arg)
{
    struct ioctl_data data;
    struct task_struct *task;
    struct mm_struct *mm;

    if (copy_from_user(&data, (struct ioctl_data __user *)arg, sizeof(struct ioctl_data)))
        return -EFAULT;

    task = current;
    mm = get_task_mm(task);
    if (!mm)
    {
        return -EFAULT;
    }

    if (get_phys_addr(mm, data.virtual_addr, &data.physical_addr) == 0)
    {
        if (copy_to_user((struct ioctl_data __user *)arg, &data, sizeof(struct ioctl_data)))
        {
            return -EFAULT;
        }
    }
    else
    {
        return -EFAULT;
    }

    mmput(mm);
    return 0;
}

/**
 * @brief Handles the IOCTL_WRITE_MEM command.
 *
 * This function writes a byte value to the memory address corresponding to a given physical address.
 * 
 * @param arg Argument associated with the IOCTL_WRITE_MEM command.
 * @return Returns 0 on success, or an error code otherwise.
 */
static int ioctl_write_mem(unsigned long arg)
{
    struct ioctl_data data;

    if (copy_from_user(&data, (struct ioctl_data *)arg, sizeof(struct ioctl_data)))
        return -EFAULT;

    if (write_to_physical_addr(data.physical_addr, &data.value))         // Perform the write operation to the physical address
        return -EFAULT;

    return 0;
}

/**
 * @brief Ioctl handler function for the device driver.
 *
 * This function handles ioctl commands received by the device driver.
 * It supports two ioctl operations: IOCTL_GET_PHYSICAL_ADDR and IOCTL_WRITE_MEM.
 * 
 * @param file Pointer to the file structure representing the device file.
 * @param cmd Ioctl command.
 * @param arg Argument associated with the ioctl command.
 * @return Returns 0 on success, or an error code otherwise.
 */
static long ioctl_memory_control_switch(struct file *file, unsigned int cmd, unsigned long arg)
{

    switch (cmd)
    {
    case IOCTL_GET_PHYSICAL_ADDR:
        return ioctl_get_physical_addr(arg);

    case IOCTL_WRITE_MEM:
        return ioctl_write_mem(arg);

    default:
        return ENOTTY; // Not a valid ioctl command
    }

    return 0;
    
}

static struct file_operations my_fops = {
    .unlocked_ioctl = ioctl_memory_control_switch}; //Registers ioctl handlers


/**
 * @brief Module initialization function.
 *
 * This function is called when the module is loaded into the kernel.
 * It registers the character device and sets up the file operations.
 *
 * @return Returns 0 on success, or an error code otherwise.
 */
static int __init init_ioctl_mem(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &my_fops);

    if (major_number < 0)
    {
        printk(KERN_ALERT "Failed to register a major number\n");
        return major_number;
    }

    pr_info("Character device registered, major number: %d\n", major_number);
    return 0;
}


/**
 * @brief Module exit function.
 *
 * This function is called when the module is unloaded from the kernel.
 * It unregisters the character device.
 */
static void __exit exit_ioctl_mem(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("Character device unregistered\n");
}

module_init(init_ioctl_mem);
module_exit(exit_ioctl_mem);