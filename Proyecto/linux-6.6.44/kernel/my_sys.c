
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/uaccess.h>
#include <linux/vmstat.h>
#include <linux/sched/signal.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/fs.h>        // Para manejar archivos
#include <linux/slab.h>      // Para manejo de memoria dinámica

#include <linux/sched.h>
#include <linux/ktime.h>




struct proc_mem_info {
    pid_t pid;
    unsigned long rss; // Resident Set Size
    unsigned long cpu_time; // Tiempo total de CPU (user + system)
    char comm[TASK_COMM_LEN]; // Nombre del proceso
    unsigned long cpu_percentage; // Porcentaje de CPU usado
};


SYSCALL_DEFINE1(get_ram_stats, char __user *, user_buffer) {
    struct sysinfo inf;
    char buffer[256];
    unsigned long cached_mem, used_mem;

    // Obtener información de memoria
    si_meminfo(&inf);

    // Calcular memoria caché correctamente
    cached_mem = global_node_page_state(NR_FILE_PAGES) - inf.bufferram;
    cached_mem *= PAGE_SIZE;

    // Calcular memoria usada: total - (free + buffers + cached)
    used_mem = (inf.totalram - inf.freeram - inf.bufferram - cached_mem / inf.mem_unit) * inf.mem_unit;

    // Escribir la información en el buffer
    snprintf(buffer, sizeof(buffer),
             "\"ram_stats\": {\n"
             "  \"total\": %lu,\n"
             "  \"used\": %lu,\n"
             "  \"free\": %lu,\n"
             "  \"cache\": %lu,\n"
             "  \"buffers\": %lu,\n"
             "  \"percentage_used\": %lu\n"
             "}\n",
             inf.totalram * inf.mem_unit,
             used_mem,
             inf.freeram * inf.mem_unit,
             cached_mem,
             inf.bufferram * inf.mem_unit,
             used_mem * 100 / (inf.totalram * inf.mem_unit));

    // Copiar la información al espacio de usuario
    if (copy_to_user(user_buffer, buffer, sizeof(buffer))) {
        return -EFAULT;
    }

    printk(KERN_INFO "RAM stats: %s\n", buffer);
    return 0;
}

SYSCALL_DEFINE1(get_swap_info, char __user *, user_buffer) {
    struct sysinfo sys_info;      // Estructura para almacenar información del sistema
    unsigned long swap_used;      // Variable para almacenar la cantidad de swap usado
    char buffer[128];             // Buffer para construir la respuesta a enviar al espacio de usuario

    // Obtiene la información de swap y la almacena en la estructura sys_info
    si_swapinfo(&sys_info);

    // Calcula el swap usado restando el swap libre del total
    swap_used = sys_info.totalswap - sys_info.freeswap;

    // Construye el mensaje en formato JSON con las estadísticas de swap
    snprintf(buffer, sizeof(buffer),
             "\"swap_info\": {\n"
             "  \"total\": %lu,\n"
             "  \"used\": %lu,\n"
             "  \"free\": %lu\n"
             "}\n",
             sys_info.totalswap << (PAGE_SHIFT - 10),  // Total de swap en kilobytes
             swap_used << (PAGE_SHIFT - 10),          // Swap usado en kilobytes
             sys_info.freeswap << (PAGE_SHIFT - 10)); // Swap libre en kilobytes

    // Copia el contenido del buffer al espacio de usuario
    if (copy_to_user(user_buffer, buffer, sizeof(buffer))) {
        return -EFAULT;  // Retorna un error si falla la copia
    }

    // Imprime las estadísticas de swap en el registro del kernel
    printk(KERN_INFO "Swap stats: %s\n", buffer);
    return 0;  // Retorna 0 en caso de éxito
}




// Define la llamada al sistema para obtener estadísticas de fallos de página
SYSCALL_DEFINE1(get_page_faults_info, char __user *, user_buffer) {
    unsigned long minor_page_faults, major_page_faults; // Variables para contar fallos de página
    char buffer[128];   // Buffer para construir la respuesta

    // Calcula los fallos de página menores y mayores
    minor_page_faults = global_node_page_state(NR_FILE_PAGES) - global_node_page_state(NR_FREE_PAGES);
    major_page_faults = global_node_page_state(NR_FILE_MAPPED);

    // Construye el mensaje en formato JSON con las estadísticas de fallos de página
    snprintf(buffer, sizeof(buffer),
             "\"page_faults_info\": {\n"
             "  \"minor_faults\": %lu,\n"
             "  \"major_faults\": %lu\n"
             "}\n",
             minor_page_faults, major_page_faults);

    // Copia el contenido del buffer al espacio de usuario
    if (copy_to_user(user_buffer, buffer, sizeof(buffer))) {
        return -EFAULT;  // Retorna un error si falla la copia
    }

    // Imprime las estadísticas de fallos de página en el registro del kernel
    printk(KERN_INFO "Page faults: %s\n", buffer);
    return 0;  // Retorna 0 en caso de éxito
}






// Define la llamada al sistema para obtener estadísticas de páginas de memoria
SYSCALL_DEFINE1(get_memory_pages_info, char __user *, user_buffer) {
    unsigned long active_memory_pages, inactive_memory_pages; // Variables para contar páginas activas e inactivas
    char buffer[128];   // Buffer para construir la respuesta

    // Calcula las páginas activas e inactivas
    active_memory_pages = global_node_page_state(NR_ACTIVE_ANON) + global_node_page_state(NR_ACTIVE_FILE);
    inactive_memory_pages = global_node_page_state(NR_INACTIVE_ANON) + global_node_page_state(NR_INACTIVE_FILE);

    // Construye el mensaje en formato JSON con las estadísticas de páginas de memoria
    snprintf(buffer, sizeof(buffer),
             "\"memory_pages_info\": {\n"
             "  \"active\": %lu,\n"
             "  \"inactive\": %lu\n"
             "}\n",
             active_memory_pages, inactive_memory_pages);

    // Copia el contenido del buffer al espacio de usuario
    if (copy_to_user(user_buffer, buffer, sizeof(buffer))) {
        return -EFAULT;  // Retorna un error si falla la copia
    }

    // Imprime las estadísticas de páginas de memoria en el registro del kernel
    printk(KERN_INFO "Memory pages: %s\n", buffer);
    return 0;  // Retorna 0 en caso de éxito
}
// syscall para obtener los 5 procesos que más memoria usan





// Función para obtener la información de los procesos
static int obtener_info_procesos(struct proc_mem_info **procs, int *count) {
    struct task_struct *task;
    int proc_count = 0, max_procs = 1024;
    unsigned long total_time_ns = ktime_get_ns(); // Tiempo total en nanosegundos desde el inicio del sistema

    // Reservar memoria para almacenar los procesos
    *procs = kmalloc_array(max_procs, sizeof(struct proc_mem_info), GFP_KERNEL);
    if (!*procs)
        return -ENOMEM;

    // Recorre todos los procesos y guarda su RSS y PID
    for_each_process(task) {
        if (task->mm) {
            if (proc_count < max_procs) {
                unsigned long task_total_time = task->utime + task->stime; // Tiempo total de CPU usado por el proceso
                (*procs)[proc_count].pid = task->pid;
                (*procs)[proc_count].rss = get_mm_rss(task->mm) << PAGE_SHIFT;
                (*procs)[proc_count].cpu_time = task_total_time;
                strncpy((*procs)[proc_count].comm, task->comm, TASK_COMM_LEN);

                // Cálculo del porcentaje de CPU: tiempo de CPU del proceso dividido por el tiempo total del sistema
                (*procs)[proc_count].cpu_percentage = (task_total_time * 100) / (total_time_ns / 1000000000); // Convertir de nanosegundos a segundos
                proc_count++;
            }
        }
    }

    *count = proc_count;
    return 0;
}

SYSCALL_DEFINE2(procesos_info, char __user *, buffer, size_t, size) {
    struct proc_mem_info *procs;
    int proc_count, err;
    char *output;
    int output_size = size;

    // Obtener información de los procesos
    err = obtener_info_procesos(&procs, &proc_count);
    if (err) {
        return err; // Error al obtener procesos
    }

    // Crear un buffer para la salida
    output = kmalloc(output_size, GFP_KERNEL);
    if (!output) {
        kfree(procs);
        return -ENOMEM;
    }

    // Agregar información de los procesos en formato JSON
    snprintf(output, output_size, "{\n  \"Procesos_que_mas_memoria_usan\": [\n");

    // Escribir los primeros 5 procesos que más memoria usan
    for (int i = 0; i < 5 && i < proc_count; i++) {
        snprintf(output + strlen(output), output_size - strlen(output), 
            "    { \"pid\": %d, \"name\": \"%s\", \"rss\": %lu, \"cpu_time\": %lu, \"cpu_percentage\": %lu }%s\n",
            procs[i].pid, procs[i].comm, procs[i].rss, procs[i].cpu_time, procs[i].cpu_percentage,
            (i == 4 || i == proc_count - 1) ? "" : ",");
    }

    snprintf(output + strlen(output), output_size - strlen(output), "  ]\n}\n");

    // Copiar datos al espacio del usuario
    if (copy_to_user(buffer, output, strlen(output))) {
        kfree(procs);
        kfree(output);
        return -EFAULT;
    }

    // Liberar la memoria utilizada
    kfree(procs);
    kfree(output);

    return 0;
}

