/**
 * @file lkm3.c
 * @brief A Linux kernel module to translate virtual address to physical address.
 *
 * This module takes a process ID (PID) and a virtual address as input parameters. It
 * determines if the virtual address is mapped to a physical address and, if so, calculates
 * the corresponding physical address. It then prints the PID, virtual address, and physical
 * address to the kernel log.
 *
 * Author: Aratrik Chandra
 *
 * References:
 * - Linux Kernel Source Code: https://elixir.bootlin.com/linux/v6.1/source/arch/x86
 * - Linux Kernel Documentation: https://www.kernel.org/doc/html/latest/
 *                               https://www.kernel.org/doc/html/next/core-api/printk-basics.html
 * - Mapple tree document: https://lwn.net/Articles/845507/
 * - Memory mapping: https://static.lwn.net/images/pdf/LDD3/ch15.pdf
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("This module takes PID and a virtual address as its input, determines if the virtual address is mapped to physical address, and if so, determines its physical address (pseudo physical address if running in a VM) and finally prints the pid, virtual address, and corresponding physical address.");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_LICENSE("GPL");

static int target_pid = -1;
static unsigned long virt_address;

module_param(target_pid, int, S_IRUGO);     // Module parameter for target PID
module_param(virt_address, ulong, S_IRUGO); // Module parameter for virtual address

/**
 * @brief Translates virtual address to physical address.
 *
 * This function takes a pointer to the task_struct of a process and a virtual address.
 * It traverses the page table hierarchy to find the corresponding physical address.
 *
 * @param task Pointer to the task_struct representing the process.
 * @param virt_address Virtual address to translate.
 * @return Returns the physical address if translation is successful, or an error code otherwise.
 */
int virt_to_phys_addr_translator(struct task_struct *task, unsigned long virt_address, unsigned long *phys_aadr)
{

    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    // Get the page global directory
    pgd = pgd_offset(task->mm, virt_address);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
    {
        printk(KERN_ERR "Page Global Directory not present.\n");
        rcu_read_unlock();
        return -EFAULT;
    }

    // Get the page upper directory (changed pud_offset to p4d_offset)
    p4d = p4d_offset(pgd, virt_address);
    if (p4d_none(*p4d) || p4d_bad(*p4d))
    {
        printk(KERN_ERR "Page Upper Directory not present.\n");
        rcu_read_unlock();
        return -EFAULT;
    }

    // Get the page upper directory
    pud = pud_offset(p4d, virt_address);
    if (pud_none(*pud) || pud_bad(*pud))
    {
        printk(KERN_ERR "Page Upper Directory not present.\n");
        rcu_read_unlock();
        return -EFAULT;
    }

    // Get the page middle directory
    pmd = pmd_offset(pud, virt_address);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
    {
        printk(KERN_ERR "Page Middle Directory not present.\n");
        rcu_read_unlock();
        return -EFAULT;
    }

    // Get the page table entry
    pte = pte_offset_kernel(pmd, virt_address);
    if (!pte || !pte_present(*pte))
    {
        printk(KERN_ERR "Page Table Entry not present.\n");
        rcu_read_unlock();
        return -EFAULT;
    }

    // Calculate the physical address
    *phys_aadr = page_to_pfn(pte_page(*pte)) << PAGE_SHIFT | (virt_address & ~PAGE_MASK);
    return 0;
}

/**
 * @brief Translates virtual address to physical address.
 *
 * This function takes a pointer to the task_struct of a process and a virtual address.
 * It traverses the page table hierarchy to find the corresponding physical address.
 *
 * @param task Pointer to the task_struct representing the process.
 * @param virt_address Virtual address to translate.
 * @param phys_aadr Pointer to store the resulting physical address.
 * @return Returns 0 if translation is successful, or an error code otherwise.
 */
static int __init init_lkm3(void)
{

    struct task_struct *task;
    unsigned long phys_address = 0;

    printk(KERN_INFO "\n-----------------------------------------------------\n"); // Print separator line
    printk(KERN_INFO "Module loaded.\n");

    if (target_pid < 0 || virt_address == 0)
    {
        printk(KERN_ERR "Invalid input parameters.\n");
        return -EINVAL;
    }

    rcu_read_lock();

    task = pid_task(find_get_pid(target_pid), PIDTYPE_PID); // Find the process with the given PID
    if (!task)
    {
        printk(KERN_ERR "Process with PID %d not found.\n", target_pid);
        rcu_read_unlock();
        return -ESRCH;
    }

    if (virt_to_phys_addr_translator(task, virt_address, &phys_address) != 0) // Translate virtual address to physical address
        return -1;                                                            // Return error code if translation fails

    rcu_read_unlock();

    // Print the results
    printk(KERN_INFO "PID: %d, Virtual Address: 0x%lx, Physical Address: 0x%lx\n", target_pid, virt_address, phys_address);

    printk(KERN_INFO "\n-----------------------------------------------------\n"); // Print separator line

    return 0;
}

/**
 * @brief Module exit function.
 *
 * This function is called when the module is unloaded from the kernel.
 * It prints a message indicating that the module is being unloaded.
 */
static void __exit exit_lkm3(void)
{
    printk(KERN_INFO "Module unloaded.\n");
}

module_init(init_lkm3); // Register module initialization function
module_exit(exit_lkm3); // Register module exit function