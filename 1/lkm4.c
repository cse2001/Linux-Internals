/**
 * @file lkm4.c
 * @brief A Linux kernel module to determine the size of allocated virtual and physical address spaces for a given process (PID).
 *
 * This module takes a process ID (PID) as input and determines the size of the allocated virtual address space (sum of all VMAs)
 * and the mapped physical address space for the corresponding process. It iterates through the virtual memory areas (VMAs) of the
 * process's memory descriptor and calculates the size of the virtual address space. For each virtual page in the VMAs, it checks
 * if the page is mapped to physical memory and calculates the size of the physical address space accordingly.
 *
 * Author: Aratrik Chandra
 * License: GPL
 *
 * References:
 * - Linux Kernel Source Code: https://elixir.bootlin.com/linux/v6.1/source
 * - Linux Kernel Documentation: https://www.kernel.org/doc/html/latest/
 * - Memory mapping: https://static.lwn.net/images/pdf/LDD3/ch15.pdf
 * - PT structure:  https://stackoverflow.com/questions/67119348/siblings-of-struct-task-struct-current-always-include-a-process-with-pid-0/67128018#67128018
 * - Memory mapping: https://linux-kernel-labs.github.io/refs/heads/master/labs/memory_mapping.html
 *                  https://stackoverflow.com/questions/67224020/how-do-i-find-the-ram-usage-of-a-process-using-its-id
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
#include <linux/mm_types.h>
#include <linux/rwsem.h>
#include <linux/proc_fs.h>

MODULE_DESCRIPTION("This module, takes a PID, determines the size of the allocated virtual address space (sum of all vmas) and the mapped physical address space for the correspomding process(PID).");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_LICENSE("GPL");

static int target_pid = -1; // Default value, should be set by insmod

module_param(target_pid, int, S_IRUGO); // Module parameter for target PID

/**
 * @brief Checks if the given virtual address is currently mapped to physical RAM memory.
 *
 * This method checks if the given virtual address is mapped to physical RAM memory by traversing
 * the page table hierarchy and checking if the corresponding page table entry is present.
 *
 * @param vma Pointer to the VM area structure.
 * @param addr Virtual address to check.
 * @return Returns 1 if the page is mapped to physical memory, 0 otherwise.
 */
static int is_page_mapped(struct vm_area_struct *vma, unsigned long addr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    pgd = pgd_offset(vma->vm_mm, addr); // Get the page global directory entry corresponding to the virtual address

    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
        return 0;

    p4d = p4d_offset(pgd, addr); // Get the page upper directory(using p4d)

    if (p4d_none(*p4d) || p4d_bad(*p4d))
    {
        return 0;
    }
    pud = pud_offset(p4d, addr); // Get the page upper directory entry corresponding to the virtual address

    if (pud_none(*pud) || unlikely(pud_bad(*pud)))
        return 0;

    pmd = pmd_offset(pud, addr); // Get the page middle directory entry corresponding to the virtual address

    if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
        return 0;

    pte = pte_offset_map(pmd, addr); // Get the page table entry corresponding to the virtual address and map it

    if (!pte || pte_none(*pte) || !pte_present(*pte))
        return 0; // Page is not mapped to physical memory

    pte_unmap(pte);
    return 1; // Page is mapped to physical memory
}

/**
 * @brief Calculates the size of allocated virtual and physical address spaces.
 *
 * This function calculates the size of the allocated virtual address space (sum of all VMAs)
 * and the mapped physical address space for the corresponding process. It iterates through
 * the VMAs of the process's memory descriptor and calculates the size of the virtual address space.
 * For each virtual page in the VMAs, it checks if the page is mapped to physical memory and calculates
 * the size of the physical address space accordingly.
 *
 * @param mm Pointer to the memory descriptor structure of the process.
 * @param virt_size Pointer to store the size of the virtual address space.
 * @param phys_size Pointer to store the size of the physical address space.
 */
void calculate_memory_sizes(struct mm_struct *mm, unsigned long *virt_size, unsigned long *phys_size)
{
    struct vm_area_struct *vma;
    unsigned long addr;

    VMA_ITERATOR(iter, mm, 0);

    for_each_vma(iter, vma)
    {
        *virt_size += vma->vm_end - vma->vm_start;

        for (addr = vma->vm_start; addr < vma->vm_end; addr += PAGE_SIZE)
        {
            if (is_page_mapped(vma, addr))
            {
                // Page is mapped to physical memory, add to physical address space
                *phys_size += PAGE_SIZE;
            }
            // else: Page is not mapped to physical memory
        }
    }
}


/**
 * @deprecated
 * @brief Calculate the total Resident Set Size (RSS) of a process.
 *
 * This function calculates the total Resident Set Size (RSS) of a process by summing up the counts of different memory types
 * and multiplying by the page size.
 *
 * @param mm Pointer to the mm_struct representing the memory descriptor of the process.
 * @return Total RSS of the process.
 */
long calculate_total_rss(struct mm_struct *mm) {
    long rss = (atomic_long_read(&mm->rss_stat.count[MM_FILEPAGES]) +
                atomic_long_read(&mm->rss_stat.count[MM_ANONPAGES]) +
                atomic_long_read(&mm->rss_stat.count[MM_SWAPENTS]) +
                atomic_long_read(&mm->rss_stat.count[MM_SHMEMPAGES])) * PAGE_SIZE;
    return rss;
}

/**
 * @brief Module initialization function.
 *
 * This function is called when the module is loaded into the kernel. It first checks if a valid
 * target PID is provided as a module parameter. Then it retrieves the task_struct of the process
 * corresponding to the provided PID. If the process is not found, it prints an error message and
 * returns an error code. If the process is found, it retrieves the memory descriptor (mm_struct)
 * for the process and locks the memory map for reading to prevent changes. After that, it calls the
 * `calculate_memory_sizes` function to calculate the size of the allocated virtual and physical
 * address spaces for the process. Once the calculation is complete, it releases the memory map lock,
 * prints the sizes of the virtual and physical address spaces in kilobytes (KB), and then unloads
 * the module.
 *
 * @return Returns 0 if the module initialization succeeds, or an error code if it fails.
 */
static int __init init_lkm4(void)
{
    struct task_struct *task; // Pointer to the task_struct representing the process
    struct mm_struct *mm;     // Pointer to the memory descriptor (mm_struct) of the process
    unsigned long virt_size;  // Size of the allocated virtual address space
    unsigned long phys_size;  // Size of the mapped physical address space

    printk(KERN_INFO "\n-----------------------------------------------------\n"); // Print separator line
    printk(KERN_INFO "Module loaded.\n");

    if (target_pid < 0) // Check if a valid target PID is provided
    {
        printk(KERN_ERR "Invalid input parameters.\n");
        return -EINVAL;
    }

    rcu_read_lock(); // Acquire RCU read lock to safely access task structures

    task = pid_task(find_get_pid(target_pid), PIDTYPE_PID); // Find the process with the given PID

    if (!task)
    {
        // Print error message if process not found
        printk(KERN_ERR "Process with PID %d not found.\n", target_pid);
        rcu_read_unlock();
        return -ESRCH;
    }

    mm = get_task_mm(task); // Get the memory descriptor (mm_struct) of the process

    if (!mm)
    {
        // Print error message if memory descriptor retrieval fails
        printk(KERN_ERR "Failed to get memory descriptor for PID %d.\n", target_pid);
        rcu_read_unlock();
        return -EFAULT;
    }
  
    down_read(&mm->mmap_lock); // Lock the memory map for reading to prevent changes

    calculate_memory_sizes(mm, &virt_size, &phys_size); // Call the function to calculate memory sizes

    up_read(&mm->mmap_lock); // Release the memory map lock

    rcu_read_unlock(); // Release RCU read lock

    printk(KERN_INFO "PID: %d, Virtual Address Space (VAS): %lu KB\n", target_pid, virt_size >> 10);
    printk(KERN_INFO "PID: %d, Physical Address Space (PAS): %lu KB (accurate)\n", target_pid, phys_size >> 10);

    //should not use this approach as it gives obsolete snapshots, so not accurate
    printk(KERN_INFO "PID: %d, Physical Address Space (RSS): %ld KB (@deprecated as not accurate)\n", target_pid, calculate_total_rss(mm)>>10);  

    printk(KERN_INFO "\n-----------------------------------------------------\n"); // Print separator line

    return 0;
}

/**
 * @brief Module exit function.
 *
 * This function is called when the module is unloaded from the kernel.
 * It prints a message indicating that the module is being unloaded.
 */
static void __exit exit_lkm4(void)
{
    printk(KERN_INFO "Module unloaded.\n");
}

module_init(init_lkm4); // Register module initialization function
module_exit(exit_lkm4); // Register module exit function