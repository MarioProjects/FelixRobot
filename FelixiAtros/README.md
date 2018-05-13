# Felix iATROS Page

Este repositorio esta creado con la finalidad de dotar de la capacidad de reconocer distintas palabras y frases a Felix.
Félix es un quadrupedo creado por Ronald Jaramillo. 
[Aquí](https://burningservos.com/) podreís encontrar más información sobre Félix y como construirlo.

## Puesta en marcha 

Para poder empezar a utilizar iATROS necesitaremos descargarnos el siguiente [paquete](http://www.dsic.upv.es/~cmartine/trash/iATROS_felix.tgz).
En dicho paquete encontraremos dos directorios:

**->   iatros-v1.0**: es donde está el código fuente (C) del reconocedor iATROS; para poder compilarlo, 
debes crearte un directorio "build" dentro, cambiar a él, ejecutar "../configure" y luego "make" y "make install".

> Si en algún momento de compilar te da un error posiblemente es que te
falten bibliotecas por instalar; la salida del "configure" suele dar
pistas para eso; si lo ves imposible dímelo y quedamos para ver cuáles
son los problemas.

**->   test**: es donde te he dejado ejemplos de configuración y
funcionamiento para el reconocedor; dentro tienes los siguientes
ficheros y directorios:

  - 00README: indica cómo ejecutar reconocedor y extractor de características
  - conf.feat: fichero de configuración para extraer características del audio
  - eutranscribe: script para la transcripción fonética. Escritor en perl. Dado un interprete y entrada nos sirve para el modelo lexico (felix.lx)
  - felix.cnf: fichero de configuración para el reconocimiento
  - models: modelos acústicos (albayzin_iatros_64gs.hmm), léxicos (felix.lx) y de lenguaje (felix.gr)
    - Modelo léxico: palabra, probabilidad (generalmente 1.0) y transcripción fonética (la da "eutranscribe") con los fonemas separados
    - Modelo de lenguaje: definición de estados (i = 1 indica inicial, f = 1 indica final, c = 1 hay que ponerlo en todos) y transiciones (origen, destino, palabra y probabilidad, la suma de probabilidades con origen común debe dar 1.0)
  - samples: ficheros de audio (raw) que he grabado a modo de ejemplo
  
  
## Utilización
  
Para ver cómo funciona, primero tienes que ejecutar el reconocedor
desde el directorio "test" (primera línea del 00README); verás que te
dice que espera la creación de cierto fichero; para ello tendrás que
ejecutar (desde otra terminal) la extracción de características
(segunda línea del 00README) con el fichero .raw que elijas y dando la
salida sobre el fichero que te pide (en este caso "/tmp/felix.CC",
pero puede cambiarse en la configuración).

Cuando el fichero de salida se haya generado el reconocedor se
activará y te dará una salida que empieza por < s > y acaba por </  s > y
en medio pone lo que ha reconocido. Lo que se puede reconocer depende
de los modelos de lenguaje y léxico que tienes dentro de "models".

Estos dos modelos tendrás que modificarlos para aceptar las palabras y
frases que creas conveniente para la tarea.
