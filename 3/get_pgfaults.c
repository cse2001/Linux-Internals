/**
 * @file get_pgfaults.c
 * @brief A kernel module creating a custom entry in the /proc filesystem to retrieve and display page fault statistics.
 *
 * This module creates a new entry in the /proc filesystem named "/proc/get_pgfaults".
 * Users can access this entry to retrieve and display information about the total count of page faults
 * encountered by the operating system since it booted.
 *
 * Author: Aratrik Chandra
 *
 * References:
 * - Linux procfs Documentation: https://www.kernel.org/doc/html/latest/filesystems/proc.html
 *                               https://sysprog21.github.io/lkmpg/
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/vmstat.h>

MODULE_DESCRIPTION("A /proc filesystem kernel module named get_pgfaults.c with the objective of introducing a fresh entry into the /proc filesystem, specifically labeled \"/proc/get_pgfaults\". Users should be able to use commands like \"cat\" to retrieve and display information regarding the total count of page faults the operating system has encountered since it booted.");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_LICENSE("GPL");

char PROCFS_NAME[13] = "get_pgfaults"; ///< Name of the custom procfs entry

static struct proc_dir_entry *our_proc_file; ///< Pointer to the custom procfs entry

/**
 * @brief Retrieve the total count of page faults encountered by the operating system since booting.
 *
 * This function retrieves the total count of page faults encountered by the operating system since booting.
 *
 * @param void
 * @return Total count of page faults.
 */
unsigned long get_total_pagefaults(void)
{

    unsigned long events[NR_VM_EVENT_ITEMS]; // Array to store VM events
    all_vm_events(events);                   // Get all VM events
    return events[PGFAULT];                  // Get page fault count
}

/**
 * @brief Read callback function for the custom procfs entry.
 *
 * This function is called when users read from the custom procfs entry ("/proc/get_pgfaults").
 * It retrieves the total count of page faults encountered by the operating system since booting
 * and formats the information as a string to be displayed to the user.
 *
 * @param file_pointer Pointer to the file structure representing the procfs entry.
 * @param buffer User space buffer to copy the data to.
 * @param buffer_length Length of the user space buffer.
 * @param offset Pointer to the file offset.
 * @return Number of bytes read on success, 0 on failure.
 */
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer,
                             size_t buffer_length, loff_t *offset)
{
    unsigned long page_faults; // variable to store page fault counts
    ssize_t len, ret;
    char formatedStr[50];                 // Buffer to store formatted string
    page_faults = get_total_pagefaults(); // Get page fault count

    printk(KERN_INFO "Page Faults since boot: %lu\n", page_faults);

    snprintf(formatedStr, sizeof(formatedStr), "Page Faults - %lu\n", page_faults);

    len = sizeof(formatedStr); // Calculate length of the formatted string
    ret = len;                 // Set return value to length

    if (*offset >= len || copy_to_user(buffer, formatedStr, len))
    { // Check if offset exceeds length or if copy to user fails
        ret = 0;
    }
    else
    {
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
        *offset += len; // Update file offset
    }

    return ret; // Return number of bytes read or 0 on failure
}

static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read // Assign read callback function
};

/**
 * @brief Module initialization function.
 *
 * This function is called when the module is loaded into the kernel.
 * It creates a new entry in the /proc filesystem ("/proc/get_pgfaults")
 * and assigns the read callback function to it.
 *
 * @return Returns 0 on success, or an error code otherwise.
 */
static int __init init_get_pgfaults(void)
{

    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops); // Create custom procfs entry

    if (NULL == our_proc_file)
    {
        proc_remove(our_proc_file);
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

/**
 * @brief Module exit function.
 *
 * This function is called when the module is unloaded from the kernel.
 * It removes the custom entry from the /proc filesystem.
 */
static void __exit exit_get_pgfaults(void)
{
    proc_remove(our_proc_file); // Remove custom procfs entry
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(init_get_pgfaults); // Register module initialization function
module_exit(exit_get_pgfaults); // Register module exit function