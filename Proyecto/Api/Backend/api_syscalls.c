
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <microhttpd.h>
#include <cjson/cJSON.h>
#include <stdlib.h> // Agregar esta línea

#define PORT 8888

#define SYS_get_ram_stats 469
#define SYS_get_swap_info 470
#define SYS_get_page_faults_info 471
#define SYS_get_memory_pages_info 472
#define SYS_procesos_info 473

// Función que realiza la llamada al sistema y convierte el resultado en JSON
cJSON *get_system_info(int syscall_num) {
    char buffer[912];
    memset(buffer, 0, sizeof(buffer));

    // Llamada al sistema
    if (syscall(syscall_num, buffer, sizeof(buffer)) == 0) {
        // Crear el objeto JSON
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "data", buffer);
        return json;
    } else {
        // Manejo de errores
        perror("syscall error");
        return NULL;
    }
}

// Función para manejar RAM stats y convertir el string en JSON
cJSON *handle_ram_stats() {
    cJSON *ram_stats_json = get_system_info(SYS_get_ram_stats);
    
    if (ram_stats_json != NULL) {
        char *ram_stats_str = cJSON_GetObjectItem(ram_stats_json, "data")->valuestring;

        // Crear un nuevo objeto JSON para los resultados de RAM
        cJSON *ram_stats = cJSON_CreateObject();

        // Extraer los valores del string
        unsigned long total, used, free_memory, cache, percentage_used; // Cambiado 'free' a 'free_memory'
        sscanf(ram_stats_str, "\"ram_stats\": {\n  \"total\": %lu,\n  \"used\": %lu,\n  \"free\": %lu,\n  \"cache\": %lu,\n  \"percentage_used\": %lu\n}", 
               &total, &used, &free_memory, &cache, &percentage_used);

        // Agregar los valores al nuevo objeto JSON
        cJSON_AddNumberToObject(ram_stats, "total", total);
        cJSON_AddNumberToObject(ram_stats, "used", used);
        cJSON_AddNumberToObject(ram_stats, "free", free_memory); // Usar 'free_memory' aquí
        cJSON_AddNumberToObject(ram_stats, "cache", cache);
        cJSON_AddNumberToObject(ram_stats, "percentage_used", percentage_used);

        // Crear un objeto JSON padre y agregar el objeto de RAM
        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddItemToObject(response_json, "ram_stats", ram_stats);

        cJSON_Delete(ram_stats_json); // Liberar el JSON original
        return response_json; // Devolver el nuevo JSON
    } else {
        printf("Failed to retrieve RAM stats.\n");
        return NULL;
    }
}

cJSON *handle_swap_info() {
    cJSON *swap_info_json = get_system_info(SYS_get_swap_info);
    if (swap_info_json != NULL) {
        char *swap_info_str = cJSON_GetObjectItem(swap_info_json, "data")->valuestring;

        // Crear un nuevo objeto JSON para los resultados de RAM
        cJSON *swap_info = cJSON_CreateObject();

        // Extraer los valores del string
        unsigned long total, used, free_memory; // Cambiado 'free' a 'free_memory'
        sscanf(swap_info_str, "\"swap_info\": {\n  \"total\": %lu,\n  \"used\": %lu,\n  \"free\": %lu\n}", 
               &total, &used, &free_memory);

        // Agregar los valores al nuevo objeto JSON
        cJSON_AddNumberToObject(swap_info, "total", total);
        cJSON_AddNumberToObject(swap_info, "used", used);
        cJSON_AddNumberToObject(swap_info, "free", free_memory);

        // Crear un objeto JSON padre y agregar el objeto de RAM
        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddItemToObject(response_json, "swap_info", swap_info);

        cJSON_Delete(swap_info_json); // Liberar el JSON original
        return response_json; // Devolver el nuevo JSON
    } else {
        printf("Failed to retrieve RAM stats.\n");
        return NULL;
    }
}

cJSON *handle_page_faults_info() {
    cJSON *system_info_json = get_system_info(SYS_get_page_faults_info);
    if (system_info_json != NULL) {
        char *system_info_str = cJSON_GetObjectItem(system_info_json, "data")->valuestring;

        // Crear un nuevo objeto JSON para los resultados de RAM
        cJSON *system_info = cJSON_CreateObject();

        // Extraer los valores del string
        unsigned long minor_faults, major_faults; 
        sscanf(system_info_str, "\"page_faults_info\": {\n  \"minor_faults\": %lu,\n  \"major_faults\": %lu\n}", 
               &minor_faults, &major_faults);

        // Agregar los valores al nuevo objeto JSON
        cJSON_AddNumberToObject(system_info, "minor_faults", minor_faults);
        cJSON_AddNumberToObject(system_info, "major_faults", major_faults);

        // Crear un objeto JSON padre y agregar el objeto de RAM
        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddItemToObject(response_json, "page_faults_info", system_info);

        cJSON_Delete(system_info_json); // Liberar el JSON original
        return response_json; // Devolver el nuevo JSON
    } else {
        printf("Failed to retrieve page faults info.\n");
        return NULL;
    }
}

cJSON *handle_memory_pages_info() {
    cJSON *memory_info_json = get_system_info(SYS_get_memory_pages_info);
    if (memory_info_json != NULL) {
        char *memory_info_str = cJSON_GetObjectItem(memory_info_json, "data")->valuestring;

        // Crear un nuevo objeto JSON para los resultados de RAM
        cJSON *memory_info = cJSON_CreateObject();

        // Extraer los valores del string
        unsigned long active, inactive; 
        sscanf(memory_info_str, "\"memory_pages_info\": {\n  \"active\": %lu,\n  \"inactive\": %lu\n}", 
               &active, &inactive);

        // Agregar los valores al nuevo objeto JSON
        cJSON_AddNumberToObject(memory_info, "active", active);
        cJSON_AddNumberToObject(memory_info, "inactive", inactive);

        // Crear un objeto JSON padre y agregar el objeto de RAM
        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddItemToObject(response_json, "memory_pages_info", memory_info);

        cJSON_Delete(memory_info_json); // Liberar el JSON original
        return response_json; // Devolver el nuevo JSON
    } else {
        printf("Failed to retrieve memory pages info.\n");
        return NULL;
    }
}
cJSON *handle_procesos_info() {
    cJSON *process_info_json = get_system_info(SYS_procesos_info);
    if (process_info_json != NULL) {
        char *memory_info_str = cJSON_GetObjectItem(process_info_json, "data")->valuestring;
        
        if (memory_info_str == NULL) {
            printf("No se encontró el string 'data' en el JSON.\n");
            cJSON_Delete(process_info_json);
            return NULL;
        }
        
        printf("String de entrada: %s\n", memory_info_str);  // Depuración

        cJSON *parsed_json = cJSON_Parse(memory_info_str);
        if (!parsed_json) {
            printf("Error al parsear el string de entrada como JSON.\n");
            cJSON_Delete(process_info_json); // Liberar el JSON original
            return NULL;
        }

        // Verificar la estructura del JSON parseado
        printf("JSON parseado: %s\n", cJSON_Print(parsed_json));  // Depuración
        
        cJSON *process_array = cJSON_GetObjectItem(parsed_json, "Procesos_que_mas_memoria_usan");
        if (!cJSON_IsArray(process_array)) {
            printf("No se encontró un array de procesos válido.\n");
            cJSON_Delete(parsed_json);
            cJSON_Delete(process_info_json); // Liberar el JSON original
            return NULL;
        }

        // Crear un nuevo array para los procesos
        cJSON *new_process_array = cJSON_CreateArray();
        cJSON *process;
        cJSON_ArrayForEach(process, process_array) {
            cJSON *new_process = cJSON_CreateObject();
            cJSON_AddNumberToObject(new_process, "pid", cJSON_GetObjectItem(process, "pid")->valuedouble);
            cJSON_AddStringToObject(new_process, "name", cJSON_GetObjectItem(process, "name")->valuestring);
            cJSON_AddNumberToObject(new_process, "rss", cJSON_GetObjectItem(process, "rss")->valuedouble);
            cJSON_AddNumberToObject(new_process, "cpu_time", cJSON_GetObjectItem(process, "cpu_time")->valuedouble);
            cJSON_AddItemToArray(new_process_array, new_process);
        }

        // Crear el JSON final con título y lista de procesos
        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddItemToObject(response_json, "Procesos_que_mas_memoria_usan", new_process_array);

        cJSON_Delete(parsed_json);
        cJSON_Delete(process_info_json);
        return response_json;
    } else {
        printf("Failed to retrieve memory pages info.\n");
        return NULL;
    }
}



// Función manejadora de las solicitudes HTTP
enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    cJSON *json_response = NULL;

    // Crear la respuesta
    struct MHD_Response *response = NULL;

    // Si el método es OPTIONS, responder inmediatamente
    if (strcmp(method, "OPTIONS") == 0) {
        const char *response_str = "";
        response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, OPTIONS");
        MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
        MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return MHD_YES;
    }

    // Identificar la URL solicitada y ejecutar la syscall correspondiente
    if (strcmp(url, "/ram_stats") == 0) {
        json_response = handle_ram_stats();
    } else if (strcmp(url, "/swap_info") == 0) {
        json_response = handle_swap_info();
    } else if (strcmp(url, "/page_faults_info") == 0) {
        json_response = handle_page_faults_info();
    } else if (strcmp(url, "/memory_pages_info") == 0) {
        json_response = handle_memory_pages_info();
    } else if (strcmp(url, "/procesos_info") == 0) {
        json_response = handle_procesos_info(); // Cambiado a la nueva función manejadora
    } else {
        json_response = cJSON_CreateObject();
        cJSON_AddStringToObject(json_response, "error", "Invalid endpoint");
    }

    // Crear respuesta JSON
    if (json_response) {
        char *json_string = cJSON_Print(json_response);
        response = MHD_create_response_from_buffer(strlen(json_string), (void *)json_string, MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Content-Type", "application/json");
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, OPTIONS");
        MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
        MHD_queue_response(connection, MHD_HTTP_OK, response);
        free(json_string);
        cJSON_Delete(json_response);
    } else {
        response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
    }

    if (response) {
        MHD_destroy_response(response);
    }

    return MHD_YES;
}

// Función principal del servidor
int main() {
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_END);
    if (daemon == NULL) {
        return 1;
    }
    getchar(); // Espera la entrada del usuario para detener el servidor
    MHD_stop_daemon(daemon);
    return 0;
}
