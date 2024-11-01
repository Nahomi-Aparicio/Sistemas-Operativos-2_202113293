// include/linux/syscalls_usac.h
#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/types.h>

asmlinkage long sys_my_encrypt(const char __user *input_path, const char __user *output_path, const char __user *key_path, int num_threads);

asmlinkage long sys_my_decrypt(const char __user *input_path, const char __user *output_path, const char __user *key_path, int num_threads);
#endif // _SYSCALLS_USAC_H
