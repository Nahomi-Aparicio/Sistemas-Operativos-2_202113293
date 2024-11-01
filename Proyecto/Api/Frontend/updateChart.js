// Configuración de las gráficas
const ctx1 = document.getElementById('myPieChart1').getContext('2d');
let myPieChart1 = new Chart(ctx1, {
    type: 'pie',
    data: {
        labels: ['Used Memory', 'Free Memory'],
        datasets: [{
            label: '',
            data: [0, 0], // Datos iniciales en 0
            backgroundColor: [
                'rgb(255, 123, 129)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre
            ],
            borderColor: [
                'rgba(255, 99, 132, 1)',
                'rgba(75, 192, 192, 1)',
            ],
            borderWidth: 3
        }]
    },
    options: {
        responsive: false,
        plugins: {
            legend: {
                position: 'top',
                labels: {
                    font: {
                        size: 18 ,// Tamaño de los labels de la leyenda
                        weight: 'bold' 
                    }
                }
            }
        }
    }
});

// Función para obtener los datos de la API y actualizar la gráfica y el contenido de RAM
async function fetchData() {
    try {
        const response1 = await fetch('http://localhost:8888/ram_stats'); // Cambia por la URL de tu API
        if (!response1.ok) {
            throw new Error(`HTTP error! Status: ${response1.status}`);
        }

        const data1 = await response1.json();
        console.log('Datos de RAM:', data1); // Imprimir los datos obtenidos

        // Extraer los valores de ram_stats
        if (!data1.ram_stats) {
            throw new Error('ram_stats no está definido en la respuesta');
        }

        const ramStats = data1.ram_stats;

        // Convertir bytes a GB
        const freeGB = (ramStats.free / (1073741824)).toFixed(2); // RAM libre
        const totalGB = (ramStats.total / (1073741824)).toFixed(2); // RAM total
        const usedGB = (ramStats.used / (1073741824)).toFixed(2); // RAM usadausedGB
        usedGB2=ramStats.used 
        console.log(usedGB2);
        const cacheGB = (ramStats.cache / (1073741824)).toFixed(2); // Caché
        console.log('Datos de RAM:', freeGB,totalGB,usedGB,cacheGB); // Imprimir los datos obtenidos
        // Actualizar elementos HTML con los nuevos datos
        document.getElementById('ramUso').textContent = `RAM EN USO: ${usedGB} GB`;
        document.getElementById('ramLibre').textContent = `RAM LIBRE: ${freeGB} GB`;
        document.getElementById('ramTotal').textContent = `CACHE: ${cacheGB} GB`;

        // Preparar los datos para la gráfica
        const chartData = {
            labels: ['Used Memory', 'Free Memory'], // Etiquetas para el gráfico
            data: [ramStats.used, ramStats.free], // Datos para el gráfico
            backgroundColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre
            ],
            borderColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color de borde para libre
            ]
        };

        // Actualizar la gráfica con los nuevos datos
        updateChart(myPieChart1, chartData);

    } catch (error) {
        console.error('Error al obtener los datos:', error);
    }
}

// Función para actualizar la gráfica
function updateChart(chart, data) {
    chart.data.labels = data.labels;
    chart.data.datasets[0].data = data.data;
    chart.data.datasets[0].backgroundColor = data.backgroundColor;
    chart.data.datasets[0].borderColor = data.borderColor;

    chart.update();
}


// Configuración de las gráficas
const ctx2= document.getElementById('myPieChart2').getContext('2d');
let myPieChart2 = new Chart(ctx2, {
    type: 'pie',
    data: {
        labels: ['Paginas Activas ', 'Paginas Inactiva'],
        datasets: [{
            label: '',
            data: [0, 0], // Datos iniciales en 0
            backgroundColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre// Color para libre
            ],
            borderColor: [
               'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre,
            ],
            borderWidth: 3
        }]
    },
    options: {
        responsive: false,
        plugins: {
            legend: {
                position: 'top',
                labels: {
                    font: {
                        size: 18 ,// Tamaño de los labels de la leyenda
                        weight: 'bold' 
                    }
                }
            }
        }
    }
});

// Función para obtener los datos de la API y actualizar la gráfica y el contenido de RAM
async function fetchData2() {
    try {
        const response1 = await fetch('http://localhost:8888/memory_pages_info'); // Cambia por la URL de tu API
        if (!response1.ok) {
            throw new Error(`HTTP error! Status: ${response1.status}`);
        }

        const data1 = await response1.json();
        console.log('Datos de pagina', data1); // Imprimir los datos obtenidos

        // Extraer los valores de ram_stats
        if (!data1.memory_pages_info) {
            throw new Error('memory_pages_info no está definido en la respuesta');
        }

        const ramStats = data1.memory_pages_info;

        // Preparar los datos para la gráfica
        const chartData = {
            labels: ['Paginas Activas', 'Paginas inactivas'], // Etiquetas para el gráfico
            data: [ramStats.active, ramStats.inactive], // Datos para el gráfico
            backgroundColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre// Color para libre
            ],
            borderColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre// Color de borde para libre
            ]
        };

        // Actualizar la gráfica con los nuevos datos
        updateChart2(myPieChart2, chartData);

    } catch (error) {
        console.error('Error al obtener los datos:', error);
    }
}

// Función para actualizar la gráfica
function updateChart2(chart, data) {
    chart.data.labels = data.labels;
    chart.data.datasets[0].data = data.data;
    chart.data.datasets[0].backgroundColor = data.backgroundColor;
    chart.data.datasets[0].borderColor = data.borderColor;

    chart.update();
}




const ctx3 = document.getElementById('BarrasFallo').getContext('2d');
let BarrasFallo = new Chart(ctx3, {
    type: 'bar',
    data: {
        labels: ['Fallos Mayores', 'Fallos menores'],
        datasets: [{
            
            data: [0, 0], // Datos iniciales en 0
            backgroundColor: [
               'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre // Color para libre
            ],
            borderColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre
            ],
            borderWidth: 3
        }]
    },
    options: {
        responsive: false,
        plugins: {
            legend: {
                position: 'top',
                labels: {
                    font: {
                        size: 18 ,// Tamaño de los labels de la leyenda
                        weight: 'bold' 
                    }
                }
            }
        }
    }
});

// Función para obtener los datos de la API y actualizar la gráfica y el contenido de RAM
async function BarrasFallo1() {
    try {
        const response1 = await fetch('http://localhost:8888/page_faults_info'); // Cambia por la URL de tu API
        if (!response1.ok) {
            throw new Error(`HTTP error! Status: ${response1.status}`);
        }

        const data1 = await response1.json();
        console.log('Datos de fallos de pagina', data1); // Imprimir los datos obtenidos

        // Extraer los valores de ram_stats
        if (!data1.page_faults_info) {
            throw new Error('ram_stats no está definido en la respuesta');
        }

        const ramStats = data1.page_faults_info;
        const chartData = {
            labels: ['Fallos Mayores', 'Fallos menores'], // Etiquetas para el gráfico
            data: [ramStats.major_faults, ramStats.minor_faults], // Datos para el gráfico
            backgroundColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre // Color para libre
            ],
            borderColor: [
                'rgb(44, 194, 149)', // Color para usado
                'rgb(0, 123, 129)', // Color para libre // Color de borde para libre
            ]
        };

        // Actualizar la gráfica con los nuevos datos
        updateChart3(BarrasFallo, chartData);

    } catch (error) {
        console.error('Error al obtener los datos:', error);
    }
}

// Función para actualizar la gráfica
function updateChart3(chart, data) {
    chart.data.labels = data.labels;
    chart.data.datasets[0].data = data.data;
    chart.data.datasets[0].backgroundColor = data.backgroundColor;
    chart.data.datasets[0].borderColor = data.borderColor;

    chart.update();
}




// Función para obtener los datos de la API y actualizar la gráfica y el contenido de RAM
async function Procesos() {
    try {
        const response1 = await fetch('http://localhost:8888/procesos_info'); // Cambia por la URL de tu API
        if (!response1.ok) {
            throw new Error(`HTTP error! Status: ${response1.status}`);
        }

        const proc = await response1.json();
        console.log('Datos de PROCESOS:', proc); // Imprimir los datos obtenidos

        // Extraer los valores de ram_stats
        if (!proc.Procesos_que_mas_memoria_usan) {
            throw new Error('Procesos_que_mas_memoria_usan no está definido en la respuesta');
        }

        const proc_info = proc.Procesos_que_mas_memoria_usan;
        console.log('Datos de prc:', proc_info); // Imprimir los datos obtenidos

        // Actualizar la tabla con los nuevos datos
        updateTable(proc_info);

        // Actualizar la gráfica con los nuevos datos

    } catch (error) {
        console.error('Error al obtener los datos:', error);
    }
}

// Función para actualizar la tabla con los datos de procesos
function updateTable(proc_info) {
    const tableBody = document.getElementById('processTable').querySelector('tbody');
    tableBody.innerHTML = ''; // Limpiar contenido previo

    proc_info.forEach(process => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${process.pid}</td>
            <td>${process.name}</td>
            <td>${process.rss}</td>
            <td>${process.cpu_time}</td>
        `;
        tableBody.appendChild(row);
    });
}

// Inicializar el gráfico
const ctx4 = document.getElementById('lineasswap').getContext('2d');
let lineasswap = new Chart(ctx4, {
    type: 'line',
    data: {
        labels: ['Inicio'], // Etiquetas iniciales
        datasets: [
            {
                label: 'Física Usada', // Etiqueta para la primera línea
                data: [0], // Datos iniciales en 0
                backgroundColor: 'rgb(44, 194, 149)', // Color para usado
                borderColor: 'rgb(44, 194, 149)', // Color de borde para usado
                borderWidth: 3,
                fill: false // No llenar el área debajo de la línea
            },
            {
                label:  'Swap Usada',// Etiqueta para la segunda línea
                data: [0], // Datos iniciales en 0
                backgroundColor: 'rgba(75, 192, 192, 0.6)', // Color para libre
                borderColor: 'rgba(75, 192, 192, 1)', // Color de borde para libre
                borderWidth: 3,
                fill: false // No llenar el área debajo de la línea
            }
        ]
    },
    options: {
        responsive: false,
        plugins: {
            legend: {
                position: 'top',
                labels: {
                    font: {
                        size: 18 ,// Tamaño de los labels de la leyenda
                        weight: 'bold' 
                    }
                }
            }
        }
        }
    
});

// Función para obtener los datos de la API y actualizar la gráfica
async function lineas() {
    try {
        const response1 = await fetch('http://localhost:8888/swap_info'); // Cambia por la URL de tu API
        if (!response1.ok) {
            throw new Error(`HTTP error! Status: ${response1.status}`);
        }

        const data1 = await response1.json();
       
        console.log('Datos de swap_info', data1.swap_info); // Imprimir los datos obtenidos

        // Extraer los valores de swap_info
        if (!data1.swap_info) {
            throw new Error('swap_info no está definido en la respuesta');
        }

        const swap_info = data1.swap_info;
        console.log('Datos de swap_info', swap_info.used, usedGB2); // Imprimir los datos obtenidos

        // Agregar nuevos datos a las líneas existentes
        lineasswap.data.labels.push(`Nuevo (${lineasswap.data.labels.length})`); // Agregar una nueva etiqueta
        lineasswap.data.datasets[0].data.push(usedGB2); // Agregar dato a la primera línea
        lineasswap.data.datasets[1].data.push(swap_info.used); // Agregar dato a la segunda línea

        // Actualizar la gráfica
        lineasswap.update();

    } catch (error) {
        console.error('Error al obtener los datos:', error);
    }
}

// Llamar a la función para obtener datos y actualizar el gráfico cada cierto tiempo (por ejemplo, cada 5 segundos)
setInterval(lineas, 5000);












// Función para llamar a fetchData periódicamente
function startFetchingData() {
    fetchData(); // Llama a la función para obtener los datos
    fetchData2();
    BarrasFallo1();
    Procesos();
    lineas();
    setTimeout(startFetchingData, 5000); // Llama cada 5 segundos
}

// Inicia la llamada a la función al cargar la página
window.onload = function() {
    startFetchingData();
};














