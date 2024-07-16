/**
 * @file lkm2.c
 * @brief A Linux kernel module to list child processes of a given parent process.
 *
 * This module takes a process ID (PID) as input and, for each of its child processes,
 * prints their PID and process state. It utilizes Linux kernel APIs to find the parent
 * process and iterate through its child processes.
 *
 * Author: Aratrik Chandra
 *
 * References:
 * - Linux Kernel Source Code: https://elixir.bootlin.com/linux/v6.1/source/arch/x86
 * - Linux Kernel Documentation: https://www.kernel.org/doc/html/latest/
 *                               https://www.kernel.org/doc/html/next/core-api/printk-basics.html
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>

MODULE_DESCRIPTION("This module takes process ID as input, and for each of its child processes, print their pid and process state.");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_LICENSE("GPL");

static int target_pid = -1;             // Parameter to receive target PID
module_param(target_pid, int, S_IRUGO); // // Module parameter to receive target PID

/**
 * @brief Prints information about child processes of a given parent process.
 *
 * This function takes a pointer to the parent task_struct and iterates through
 * its child processes. For each child process found, it prints the PID and process state.
 *
 * @param parent_task Pointer to the parent task_struct.
 * @return void
 */
void list_child_processes(struct task_struct *parent_task)
{
    struct task_struct *child_task;

    list_for_each_entry_rcu(child_task, &parent_task->children, sibling)
    {
        printk(KERN_INFO "Child PID: %d, State: %u\n", child_task->pid, child_task->__state);
    };
}

/**
 * @brief Module initialization function.
 *
 * This function is called when the module is loaded into the kernel. It first checks
 * if a target PID is provided as a module parameter. Then it finds the parent process
 * with the given PID and calls list_child_processes to print information about its child processes.
 *
 * @return Returns 0 on success, or an error code otherwise.
 */
static int init_lkm2(void)
{
    struct task_struct *parent_task;
    printk(KERN_INFO "\n-----------------------------------------------------\n"); // Print separator line
    printk(KERN_INFO "Module loaded.\n");

    if (target_pid < 0)
    {
        printk(KERN_ERR "Target PID not provided.\n");
        return -EINVAL;
    }

    rcu_read_lock(); // Acquire RCU read lock to safely access task structures

    parent_task = pid_task(find_get_pid(target_pid), PIDTYPE_PID); // Find the parent process with the given PID

    if (!parent_task)
    {
        // If parent process not found, print error message and release RCU lock
        printk(KERN_ERR "Process with PID %d not found.\n", target_pid);
        rcu_read_unlock();
        return -ESRCH;
    }

    // Iterate through the child processes
    list_child_processes(parent_task);

    rcu_read_unlock(); // Release RCU lock after accessing task structures

    printk(KERN_INFO "\n-----------------------------------------------------\n"); // Print separator line

    return 0;
}

/**
 * @brief Module exit function.
 *
 * This function is called when the module is unloaded from the kernel.
 * It prints a message indicating that the module is being unloaded.
 * @return void
 */
static void exit_lkm2(void)
{
    printk(KERN_INFO "Module unloaded.\n");
}

module_init(init_lkm2);
module_exit(exit_lkm2);