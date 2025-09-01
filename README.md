

# Sistema de Detección de Irregularidades en las Vías 🚦

## Descripción

Este proyecto detecta y clasifica irregularidades en las calles, como baches o daños en la superficie, mediante un sistema embebido con ESP y sensores de movimiento. Los datos se almacenan en la nube y se visualizan en un mapa de calor de Guayaquil, permitiendo monitorear el estado de las vías en tiempo real y facilitar su mantenimiento.
<p align="center">
Interfaz diseñada
</p>
<p align="center">
<img width="850" height="550" alt="image" src="https://github.com/user-attachments/assets/4abac9a0-946b-44d1-94af-68e649d6e6f6" />
</p>
<p align="center">
Maqueta propuesta
</p>
<p align="center">
<img width="850" height="550" alt="image" src="https://github.com/user-attachments/assets/5af3be66-1148-4c43-8d92-b1e02131be85" />
</p>
<p align="center">
Diseño PCB
 </p>
 <p align="center">
<img width="500" height="500" alt="image" src="https://github.com/user-attachments/assets/0164177b-8a17-495f-8887-8d227b827e98" />
</p>


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


