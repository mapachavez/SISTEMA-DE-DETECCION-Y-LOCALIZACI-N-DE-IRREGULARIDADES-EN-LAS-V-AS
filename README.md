

# Sistema de Detección de Irregularidades en las Vías 🚦

## Descripción

Este proyecto detecta y clasifica irregularidades en las calles, como baches o daños en la superficie, mediante un sistema embebido con ESP y sensores de movimiento. Los datos se almacenan en la nube y se visualizan en un mapa de calor de Guayaquil, permitiendo monitorear el estado de las vías en tiempo real y facilitar su mantenimiento.

---

## Características ✨

* Detección de movimientos en la vía:

  * **Quieto**: Sin movimiento anormal.
  * **Movimiento**: Paso normal de vehículos.
  * **Irregularidad**: Posible daño en la superficie.
  * **Posible golpe (bache)**: Indicación de bache u obstáculo.
* Visualización en tiempo real mediante mapa de calor.
* Almacenamiento de datos en la nube para análisis histórico.
* Sistema embebido basado en ESP con sensores de movimiento.

---

## Tecnologías Usadas 🛠️

* **Hardware**: ESP32, sensores de movimiento y acelerómetros.
* **Software Embebido**: PlatformIO, C/C++, Librerías.
* **Visualización**: Mapbox para mapas de calor.
* **Base de Datos en la Nube**: Firebase u otros servicios de almacenamiento remoto.


