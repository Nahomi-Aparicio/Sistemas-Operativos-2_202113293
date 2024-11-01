## UNIVERSIDAD SAN CARLOS DE GUATEMALA 
## FACULTAD DE INGENIERIA 
## ESCUELA DE CIENCIAS Y SISTEMAS 
## Sistemas Operativos 2 
## SECCIÓN A 

### PRACTICA 2 

### **Nombre:** Genesis Nahomi Aparicio Acan   
### **Carne:** 202113293  


### Indice
- [Desarrollo de llamadas al sistema](#pruebas)
- [Modificar el Archivo de Encabezado ](#Encabezado)
- [Actualizar syscall_64.tbl ](#Actualizar)
- [Actualizar Makefile](#Makefile)
- [Actualizar syscall.h ](#syscall)
- [Instalar el kernel](#Instalar)
- [Errores](#Errores)

---


## Desarrollo de llamadas al sistema  <div id="Desarrollo"></div>

Primero debes de dirigirte a la siguiente carpeta `linux-6.6.44/kernel`

### Llamada my_sys.c

En la carpeta anterior crearas un archivo llamado `my_sys.c`, este archivo se desglosa de la siguiente manera 


1.  Inclusión de Cabeceras

Estas cabeceras incluyen las definiciones necesarias para trabajar con llamadas al sistema (syscalls)

```bash
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


```
2.  get_ram_stats 

Esta syscall recopila estadísticas sobre el uso de la RAM del sistema, como la memoria total, usada, libre, en caché y buffers. Utiliza la función si_meminfo para obtener estos datos. Luego, calcula la memoria usada restando la memoria libre, buffers y caché de la memoria total. La información se formatea en un buffer en formato JSON y se copia al espacio de usuario mediante copy_to_user. Si la copia falla, retorna un error -EFAULT; de lo contrario, imprime la información en el log del kernel y devuelve 0 para indicar éxito.

```bash
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
```

3.  get_swap_info

Esta syscall proporciona estadísticas de la memoria swap del sistema, como la cantidad total, usada y libre. Primero, obtiene esta información utilizando la función `si_swapinfo` para llenar una estructura `sysinfo`, luego calcula el swap usado restando el swap libre del total. Los resultados se organizan en formato JSON y se envían al espacio de usuario con copy_to_user. Retorna 0 si tiene éxito o -EFAULT en caso de error.`

```bash
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
```

4.  get_page_faults_info

Esta función se encarga de reportar estadísticas sobre los fallos de página (tanto menores como mayores). Calcula el número de fallos menores restando las páginas libres de las totales de archivos, y obtiene los fallos mayores a partir de las páginas mapeadas. La información se formatea como `JSON` y se envía al espacio de usuario. Al igual que las otras funciones, devuelve 0 si tiene éxito o -EFAULT si falla al copiar.

```bash
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
...
```

5. get_memory_pages_info

Esta syscall ofrece información sobre las páginas de memoria activas e inactivas del sistema. Calcula el número de páginas activas (tanto anónimas como de archivos) e inactivas, utilizando la función `global_node_page_state`. Los datos se organizan en formato JSON y se copian al espacio de usuario usando `copy_to_use`r, retornando 0 si tiene éxito o -EFAULT si hay un error.

```bash
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

```



6. obtener_info_procesos

Esta función recopila información sobre los procesos activos en el sistema. Inicializa una estructura para almacenar los datos y reserva memoria para hasta 1024 procesos. Luego, itera a través de todos los procesos, obteniendo el PID, el RSS (memoria residente), el tiempo total de CPU y el nombre del proceso. También calcula el porcentaje de uso de CPU en relación con el tiempo total del sistema. Al final, actualiza el conteo de procesos y devuelve 0 si todo fue exitoso.

 ```bash
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
...

```
7. SYSCALL_DEFINE2

Esta función define una syscall para que los usuarios puedan acceder a la información de los procesos. Llama a obtener_info_procesos para llenar una estructura con los datos recopilados. Reserva memoria para un buffer de salida en formato JSON y, si tiene éxito, construye la salida incluyendo los cinco procesos que más memoria utilizan. Luego, copia los datos al espacio de usuario y libera la memoria utilizada, finalizando con un retorno de 0 para indicar éxito.

```bash    
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

...
 ```


## Crear el Archivo de Encabezado <div id="Encabezado"></div>

dirigete al archivo  `linux-6.6.44/include/linux` y modifica  el archivo `syscalls_usac.h` el cual tendra el siguiente codigo

``` bash
// include/linux/syscalls_usac.h

#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/types.h>

asmlinkage long sys_my_encrypt(const char __user *input_path, const char __user *output_path, const char __user *key_path, int num_threads);

asmlinkage long sys_my_dencrypt(const char __user *input_path, const char __user *output_path, const char __user *key_path, int num_threads);



asmlinkage long get_ram_stats(char __user *buffer, size_t size);
asmlinkage long get_swap_info(char __user *buffer, size_t size);
asmlinkage long get_page_faults_info(char __user *buffer, size_t size);
asmlinkage long get_memory_pages_info(char __user *buffer, size_t size);
asmlinkage long procesos_info(char __user *buffer, size_t size);

#endif // _SYSCALLS_USAC_H
```


## Actualizar syscall_64.tbl <div id="Actualizar"></div>

dirigete al archivo  `arch/x86/entry/syscalls/syscall_64.tbl` y agrega al  archivo  lo siguiente

``` bash
469 64 get_ram_stats sys_get_ram_stats
470 64 get_swap_info sys_get_swap_info
471 64 get_page_faults_info sys_get_page_faults_info
472 64 get_memory_pages_info sys_get_memory_pages_info
473 64 procesos_info sys_procesos_info

```

## Actualizar Makefile <div id="Makefile"></div>

dirigete al archivo  `kernel/Makefile` y agrega al archivo lo siguiente para que funcionen tus dos llamadas al sistema 

``` bash
obj-y += my_encrypt.o
obj-y += my_dencrypt.o
obj-y += my_sys.o
```

## Actualizar syscall.h <div id="syscall"></div>

dirigete al archivo  `include/linux/syscall.h` y agrega al archivo lo siguiente para que funcionen tus dos llamadas al sistema 

``` bash

asmlinkage long get_ram_stats(char __user *buffer, size_t size);
asmlinkage long get_swap_info(char __user *buffer, size_t size);
asmlinkage long get_page_faults_info(char __user *buffer, size_t size);
asmlinkage long get_memory_pages_info(char __user *buffer, size_t size);
asmlinkage long procesos_info(char __user *buffer, size_t size);

```

## Compilar el kernel <div id="Compilar"></div>

necesitamos un archivo de configuración ya que no lo tenemos debemos de copiar el archivo que ya tenemos en sistema y copiarlo con el siguiente comando  

``` bash
cp -v /boot/config-"$(uname -r)" .config
```

ya que hay drivers que no necesitamos ya que están compilados en nuestro sistema colocamos el siguiente comando que omitirá todos estos archivos

``` bash
$ make localmodconfig
```

- al ejecutar el comando anterior, se le solicita algún input, simplemente presionar Enter cada vez (sin escribir una respuesta)
- Luego tenemos que modificar el `.config`, ya que al copiar nuestro `.config` se incluyeron nuestras llaves privadas, por lo que tendremos que eliminarlas del `.config`. **Este paso es esencial**

```bash
$ scripts/config --disable SYSTEM_TRUSTED_KEYS
$ scripts/config --disable SYSTEM_REVOCATION_KEYS
$ scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
$ scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
```


Ahora es el momento de compilar el kernel. Para esto simplemente ejecute el comando:

``` bash
fakeroot make -j$(nproc)
```

Para comprobar el estado del proceso al finalizar la compilación, usar el siguiente comando:

```bash
$ echo $?
```

si su salida es 0 significa que no hubo ningun error en la compilacion encambio si su salida es algun numero significa que ocurrio algun error y debe volver a compilar el kernel  




## Instalar el kernel <div id="Instalar"></div>

Primero se instalan los módulos del kernel ejecutando:

```bash
$ sudo make modules_install
```


Luego instalamos el kernel:

```bash
$ sudo make install
```

Después de eso, reiniciamos la computadora para que se complete la instalación.

```bash
$ sudo reboot
```


## Errores en la practica  <div id="errores"></div>

### Error de Enlace (Linker Error)

Referencia no definida a la función __x64_get_memory_pages_info durante la fase de enlace del proceso de compilación.


##### Solucion

Esto se soluciono cambiando el nombre de las funcion a
 ``` bash 
 472 64 get_memory_pages_info sys_get_memory_pages_info
 ```



### Error 2: miembro no existente

al usarse a versión del kernel que estás utilizando (6.6.44), la estructura  sysinfo no incluye un campo llamado cached y se intento usar `inf.cached`. 


##### Solucion
n lugar de utilizar inf.cached, puedes calcular la memoria caché utilizando otras funciones o campos disponibles.En este caso se uso 

 ``` bash 
   cached_mem = global_node_page_state(NR_FILE_PAGES) - inf.bufferram;
    cached_mem *= PAGE_SIZE;
```

### Error 3: Error funciones no disponibles
`error: implicit declaration of function ‘strtok’; did you mean ‘kstrtol’? [-Werror=implicit-function-declaration]`La función strtok no está disponible en el contexto del núcleo de Linux, lo que genero un error de declaración implícita y advertencias de asignación incorrecta de tipo (de int a char *). Además, hay una advertencia sobre el tamaño del marco de la función, que es mayor a 1024 bytes.


##### Solucion 
Se reemplazó el uso de strtok con un bucle que utiliza strchr para encontrar el final de cada línea en el buffer de texto leído. Se procesaron las líneas manualmente para identificar SwapTotal: y SwapFree:, evitando así el uso de funciones no disponibles en el kernel. Esto permitió compilar la llamada  correctamente sin errores.