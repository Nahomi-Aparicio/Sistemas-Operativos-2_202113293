#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/sysinfo.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/vmstat.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("User");
MODULE_DESCRIPTION("RAM Stats Kernel Module");

static int __init ram_stats_init(void) {
    struct sysinfo inf;
    unsigned long total_mem, free_mem, used_mem, cache_mem, buffers_mem;
    char buffer[256];

    // Obtener la información de la memoria
    si_meminfo(&inf);

    total_mem = inf.totalram * inf.mem_unit;        // Memoria total
    free_mem = inf.freeram * inf.mem_unit;          // Memoria libre
    buffers_mem = inf.bufferram * inf.mem_unit;     // Buffers de memoria

    // Obtener la memoria en caché (similar a lo que muestra /proc/meminfo)
    cache_mem = global_node_page_state(NR_FILE_PAGES) * PAGE_SIZE;

    // Calcular la memoria usada similar al comando 'free -h'
    used_mem = total_mem - free_mem - cache_mem - buffers_mem;

    // Crear la salida con los datos
    snprintf(buffer, sizeof(buffer),
             "RAM stats:\n"
             "  Total: %lu KB\n"
             "  Used: %lu KB\n"
             "  Free: %lu KB\n"
             "  Cache: %lu KB\n"
             "  Buffers: %lu KB\n"
             "  Percentage Used: %lu%%\n",
             total_mem / 1024,               // Convertir de Bytes a KB
             used_mem / 1024,                // Convertir de Bytes a KB
             free_mem / 1024,                // Convertir de Bytes a KB
             cache_mem / 1024,               // Convertir de Bytes a KB
             buffers_mem / 1024,             // Convertir de Bytes a KB
             (used_mem * 100) / total_mem);  // Porcentaje de uso de memoria

    // Imprimir la información en el log del kernel
    printk(KERN_INFO "%s\n", buffer);
    return 0;
}

static void __exit ram_stats_exit(void) {
    printk(KERN_INFO "RAM stats module unloaded.\n");
}

module_init(ram_stats_init);
module_exit(ram_stats_exit);

