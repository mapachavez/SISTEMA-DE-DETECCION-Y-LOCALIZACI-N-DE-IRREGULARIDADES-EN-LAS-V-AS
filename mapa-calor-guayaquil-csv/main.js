// Inicializar mapa con estilo outdoors
const map = new mapboxgl.Map({
    container: 'map',
    style: 'mapbox://styles/mapbox/outdoors-v12', // ESTILO CAMBIADO A OUTDOORS
    center: [-79.9, -2.17], // Centro en Guayaquil
    zoom: 11,
    maxZoom: 16,
    minZoom: 9
});