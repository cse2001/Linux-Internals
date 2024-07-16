/*
 * ioctl_process.c
 *
 * Implementation of ioctl device driver providing a call to modify the parent process of the current process.
 *
 * Author: Aratrik Chandra
 * License: GPL
 * Description: This module allows modification of the parent process of the current process. It provides an ioctl call
 *              that takes a pid as an argument and modifies the task structure of the current process to change its
 *              parent process to the process with the given pid. Specifically, the process with the given pid receives
 *              a SIGCHLD signal on exit of the current process (which makes the ioctl call).
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#include "ioctl_process.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_DESCRIPTION("ioctl device driver which provides a call with specifications: Takes a pid as an argument. Modifies the task structure of the current process to change its parent process to the process with the given pid. More specifically, the process with the given pid should receive a SIGCHLD signal on exit of the current process (which makes the ioctl call).");

static int major_number; ///< Major number assigned to the device driver

/**
 * @brief Open callback function for the ioctl device.
 *
 * @param inode Pointer to the inode structure.
 * @param file Pointer to the file structure.
 * @return Returns 0 on success.
 */
static int ioctl_open(struct inode *inode, struct file *file)
{
    pr_info("Device opened\n");
    return 0;
}

/**
 * @brief Release callback function for the ioctl device.
 *
 * @param inode Pointer to the inode structure.
 * @param file Pointer to the file structure.
 * @return Returns 0 on success.
 */
static int ioctl_release(struct inode *inode, struct file *file)
{
    pr_info("Device closed\n");
    return 0;
}

/**
 * @brief Modify the parent of the current process and add it as a child to the new parent.
 *
 * @param old_parent_task Pointer to the old parent task_struct.
 * @param new_parent_task Pointer to the new parent task_struct.
 */
static void modify_parent(struct task_struct *old_parent_task, struct task_struct *new_parent_task)
{
    struct task_struct *entry;
    struct list_head *node;
    static DEFINE_SPINLOCK(children_list_lock);

    // Iterate over the children of the old parent to find the current process
    list_for_each(node, &old_parent_task->children)
    {
        entry = list_entry(node, struct task_struct, sibling);
        if (entry->pid == current->pid)
        {
            // Remove the current process from the list of children of the old parent
            list_del(node);
            printk("task with pid: %u, Removed from its old parent:%u\n", entry->pid, old_parent_task->pid);
            break;
        }
    }

    spin_lock(&children_list_lock);
    // Inherit the current process by adding it to the list of children of the new parent
    list_add_tail(&current->sibling, &new_parent_task->children);

    // Update the parent pointers of the current process
    current->real_parent = new_parent_task;
    current->parent = new_parent_task;

    printk("task with pid: %u, Newly added parent:%u\n", current->pid, new_parent_task->pid);

    spin_unlock(&children_list_lock);
}

/**
 * @brief IOCTL function to modify the parent of the current process.
 *
 * @param file Pointer to the file structure.
 * @param cmd IOCTL command.
 * @param arg Argument for the IOCTL command.
 * @return Returns 0 on success, error code on failure.
 */
static long ioctl_process(struct file *file, unsigned int cmd, unsigned long arg)
{
    pid_t new_parent_pid;
    struct task_struct *new_parent_task, *old_parent_task;
    
    if (cmd != IOCTL_SET_PARENT_PID)
        return -ENOTTY;

    if (copy_from_user(&new_parent_pid, (pid_t *)arg, sizeof(pid_t)))
        return -EFAULT;

    rcu_read_lock();

    // Find the task_struct for the target process
    new_parent_task = pid_task(find_vpid(new_parent_pid), PIDTYPE_PID);
    old_parent_task = current->parent;

    if (!new_parent_task || !old_parent_task)
        return -ESRCH; // Process not found

    printk("newParent: %s and oldParent: %s\n", new_parent_task->comm, old_parent_task->comm);

    modify_parent(current->parent, new_parent_task); // Call the method to modify the parent of the current process

    rcu_read_unlock();
    return 0;
}

static struct file_operations my_fops = {  //Registers ioctl handlers
    .owner = THIS_MODULE,
    .open = ioctl_open,
    .release = ioctl_release,
    .unlocked_ioctl = ioctl_process};

static int __init init_ioctl_process(void)
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

static void __exit exit_ioctl_process(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("Character device unregistered\n");
}

module_init(init_ioctl_process);
module_exit(exit_ioctl_process);