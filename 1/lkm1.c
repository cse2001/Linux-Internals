/**
 * @file lkm1.c
 * @brief A Linux kernel module to list all processes in a running or runnable state.
 *
 * This module lists all processes in a running or runnable state in the Linux kernel.
 * It utilizes the Linux kernel's task management APIs to iterate through all processes
 * and print information about processes that are in the TASK_RUNNING state.
 *
 * This code is designed to be loaded as a kernel module and provides functionality to
 * print the PID and state of each running process.
 *
 * Author: Aratrik Chandra
 *
 *
 * References:
 * - Linux Kernel Source Code: https://elixir.bootlin.com/linux/v6.1/source/arch/x86
 * - Linux Kernel Documentation: https://www.kernel.org/doc/html/latest/
 *                               https://www.kernel.org/doc/html/next/core-api/printk-basics.html
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("list all processes in a running or runnable state");
MODULE_AUTHOR("Aratrik Chandra");
MODULE_LICENSE("GPL"); // License information for the module

/**
 * @brief Prints information about processes in the TASK_RUNNING state.
 *
 * This function iterates through all processes in the Linux kernel and prints
 * information about processes that are in the TASK_RUNNING state.
 *
 * @return void
 */
void print_running_processes(void)
{

    struct task_struct *task;

    for_each_process(task)
    {
        if (task->__state == TASK_RUNNING)
        {
            printk(KERN_INFO "PID: %d STATE: %u\n", task->pid, task->__state);
        }
    }
}

/**
 * @brief Module initialization function.
 *
 * This function is called when the module is loaded into the kernel.
 * It prints a message indicating that the module is being initialized
 * and then calls the print_running_processes function to list all running processes.
 *
 * @return Returns 0 on success.
 */
static int __init init_lkm1(void)
{
    printk(KERN_INFO "\n-----------------------------------------------------\n");
    printk(KERN_INFO "Module loaded.\n");
    printk(KERN_INFO "List of processes in RUNNING or RUNNABLE state:-\n");

    print_running_processes();

    printk(KERN_INFO "\n-----------------------------------------------------\n");
    return 0;
}

/**
 * @brief Module exit function.
 *
 * This function is called when the module is unloaded from the kernel.
 * It prints a message indicating that the module is being unloaded.
 *
 * @return void
 */
static void __exit exit_lkm1(void)
{
    printk(KERN_INFO "Module unloaded.\n");
}

module_init(init_lkm1); // Register module initialization function
module_exit(exit_lkm1); // Register module exit function