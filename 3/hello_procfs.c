/**
 * @file hello_procfs.c
 * @brief A kernel module creating a custom entry in the /proc filesystem.
 *
 * This module creates a new entry in the /proc filesystem called "/proc/hello_procfs".
 * When users access this entry using the "cat" command, it displays the message "Hello World!".
 *
 * Author: Aratrik Chandra
 *
 * References:
 * - Linux procfs Documentation: https://www.kernel.org/doc/html/latest/filesystems/proc.html
 *                                  https://sysprog21.github.io/lkmpg/
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_DESCRIPTION("A /proc filesystem kernel module that creates a new entry in the /proc filesystem called \"/proc/hello_procfs\". This entry allows users to read and display \"Hello World!\" message when they use the \"cat\" command on it.");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_LICENSE("GPL");

char PROCFS_NAME[13] = "hello_procfs"; ///< Name of the custom procfs entry

static struct proc_dir_entry *our_proc_file; ///< Pointer to the custom procfs entry

/**
 * @brief Read callback function for the custom procfs entry.
 *
 * This function is called when users read from the custom procfs entry ("/proc/hello_procfs").
 * It copies the "Hello World!" message to the user space buffer.
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
    char custom_message[14] = "Hello World!\n"; // Message to be displayed
    int len = sizeof(custom_message);           // Length of the message
    ssize_t ret = len;

    if (*offset >= len || copy_to_user(buffer, custom_message, len))
    { // Check if offset exceeds message length or if copy to user fails
        ret = 0;
    }
    else
    {
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
        *offset += len;
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
 * It creates a new entry in the /proc filesystem ("/proc/hello_procfs")
 * and assigns the read callback function to it.
 *
 * @return Returns 0 on success, or an error code otherwise.
 */
static int __init init_hello_procfs(void)
{

    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops); // Create custom procfs entry

    if (NULL == our_proc_file)
    {
        proc_remove(our_proc_file); // Remove procfs entry if creation fails
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
static void __exit exit_hello_procfs(void)
{
    proc_remove(our_proc_file); // Remove custom procfs entry
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(init_hello_procfs); // Register module initialization function
module_exit(exit_hello_procfs); // Register module exit function
