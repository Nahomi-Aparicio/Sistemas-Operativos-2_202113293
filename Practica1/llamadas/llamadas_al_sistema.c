#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define SYS_uptime 467
#define SYS_get_current_time 466
#define SYS_retrieve_last_five_logs 468
#define LOG_BUFFER_SIZE 1024

int main() {
    // Consultar el tiempo de actividad del sistema
    long system_uptime = syscall(SYS_uptime);
    if (system_uptime == -1) {
        perror("Error al obtener el tiempo de actividad");
        return 1;
    }
    printf("Tiempo desde el último arranque: %ld segundos\n", system_uptime);

    // Consultar el tiempo actual en segundos desde el inicio de la época
    long current_time = syscall(SYS_get_current_time);
    if (current_time == -1) {
        perror("Error al obtener el tiempo actual");
        return 1;
    }
    printf("Tiempo actual en segundos desde el eponch: %ld\n", current_time);

    // Obtener los últimos cinco mensajes del registro
    char log_buffer[LOG_BUFFER_SIZE];
    ssize_t log_size = syscall(SYS_retrieve_last_five_logs, log_buffer, LOG_BUFFER_SIZE);
    if (log_size < 0) {
        perror("Error al recuperar los últimos cinco mensajes de registro");
        return 1;
    }
    printf("Últimos 5 mensajes del registro:\n%.*s\n", (int)log_size, log_buffer);

    return 0;
}

