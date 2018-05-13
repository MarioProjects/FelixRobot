# Felix Server Page


Félix es un quadrupedo creado por Ronald Jaramillo. 
[Aquí](https://burningservos.com/) podreís encontrar más información sobre Félix y como construirlo.

Este repositorio tiene como finalidad la creación de una página de configuración para Félix. Se compone de un servidor basado en Express y el paso de mensajes por SocketIo. Es motando sobre la placa [CHIP](https://getchip.com/pages/chip)

## Configuración 

### Servo PCA9685

| Cables | PCA9685 | CHIP |
| ------ | ------ | ------ |
| Blanco | VCC | 3.3V |
| Gris | GND | GND |
| Amarillo | SCL | TWI2-SCK | 
| Azul | SDA | TWI2-SDA | 

[Referencia](http://johnny-five.io/examples/servo-PCA9685/)

## Primeros Pasos

En primer lugar necesitaremos clonar el repositorio y  instalar las dependencias.
**No olvidar instalar chip-io**

```sh
$ git clone https://github.com/MarioProjects/FelixServer.git
$ cd FelixServer
$ npm install
$ npm install chip-io --save
```

Para arrancar la aplicación...

```sh
$ npm start
```

## License
----

MIT

**Free Software, Hell Yeah!**
