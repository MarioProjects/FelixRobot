/*! \mainpage
 
  
 \section intro Introduction
 
  This is the documentation of iAtros speech library, for the new Improved Automatic Trainable RecOgnition System.
 
  \section known Known Problems
  In some machines, the audio libraries only work if the OSS modules are disabled
 
  \section libraries Additional libraries
 
  The iAtros Sound System needs some external libraries:\n
  -ALSA libray\n
  -FFTW 3\n
  \n
  Both can be installed in Debian-based distributions by using:\n
  apt-get install libasound2-dev libfftw3-dev\n

  \section prep iAtros Feature Extraction and Sound system
  
  If you want to know more about feature extraction, you can visit the \subpage technical subpage.
  
  Some notes about the audio system can be found in the \subpage audio.

  \section bug Bug report
 
  If you want to notify a bug you can contact with:\n
  Audio system - vtamarit@iti.upv.es\n
  Viterbi - mlujan@iti.upv.es\n
  Anithing else - You can email both\n
  
  <b>Thanks for using iAtros!</b>
 */

/*! \page technical Technical Documentation

\section intro Introduction

The feature extraction method used in iAtros is the Mel-Frecuency cepstral coefficients (MFCCs) a well-known method that has been proved to be the most effective and widely used.
This guide describes the implementation from a programmer point of view.

<b>This section is still under construction</b>

\section how How it works?

El aparato fonador humano puede representarse como la combinación de un generador de impulsos (a partir del tono fundamental) y un generador de ruido aleatorio, ambos asociados a un interruptor que elige uno u otro (voz sonora/sorda). La onda generada ha de pasar por un filtro lineal variable con el tiempo (el tracto vocal).
La extracción de característica busca modelar el habla eliminando cualquier rasgo propio del hablante, esto es: se centra en la extracción de los filtros linealaes.

Para extraer esa información el esquema típico es el siguiente:

Señal -> preenfasis -> Venta de hamming -> FFT -> Escala de Mel -> Logaritmo -> DCT (Discrete Cosine Transform)

En las siguientes subsecciones se explican con más detalle cada uno de los pasos y su implementación en iAtros. Todo el proceso está basado en el estándar ETSI ES 201 108 V1.1.3. La sección 4 del estandar explica detalladamente el preproceso de señal para reconocimiento de habla.

\subsection senyal Signal

Para procesar la señal se desplaza una ventana sobre ella y se extraen características para cada una de esas ventanas.

\subsection pre Preenfasis
La extracción de características comienza aplicando un filtro paso-alto a la señal. La voz se manifiesta en el dominio de la frecuencia como formantes (picos debidos a resonancias del tracto vocal). Los formantes de alta frecuencia tienen una amplitud menor que la de los formantes de baja frecuencia, aunque poseen información más importante.
Para balancear la señal se realiza el proceso de preénfasis. Dada una señal vocal s(n) la señal de salida x(s) se calcula como:
\verbatim
x(n)=s(n)-alfas(n-1)
\endverbatim

La función que realiza esto es preemphasis().

\subsection hamming Hamming Window
Una vez se ha aplicado el preénfasis a la señal ésta se analiza utilizando una ventana que la recorre. El tamaño de la ventana representa una duración de la señal en la que voz se puede asumir que es estacionaria. Que sea estacionaria significa que sus parámetros (distribución de amplitud y desviación estándar) son constantes en el tiempo.
Si recorremos la señal desplazando una ventana de cualquier forma obtenemos trozos de señal cuyos bordes entorpecen la extracción de características, debido a que el corte de los bordes está añadiendo ruido que interfiere con el espectro real. Para solucionar esto se utiliza una ventana de Hamming, así dado un trozo de señal enfatizada de n elementos x(n), se obtiene una nueva señal h(n) mediante:
\verbatim
h(n)=alfa-(1-alfa)(2pi(n+0.5)/W)
\endverbatim
Donde:
- alfa vale 0.54 para la ventana de Hamming (la más común)
- n indica el número de muestra de la ventana
- W el tamaño de la ventana en muestras (típicamente 410 para una señal de 16Khz)

La venta de Hamming se calcula tras cargar el fichero de configuración, al final de la función loadConfiguration(). Allí se realiza un precálculo de ciertos valores que se mantienen constantes para todos los buffers.
La función hammingWindow() es quien aplica la ventana al buffer.

\subsection fft Fast Fourier Transform
El paso siguiente una vez obtenida la ventana es pasar el trozo de señal del dominio del tiempo al de la frecuencia. Realizar esta transformación revela más información de la señal vocal que es importante para la clasificación. La forma de realizar este cambio de dominio es mediante la transformada de Fourier discreta (DFT), aunque en la práctica se utiliza la FFT (Fast Fourier Transform) mucho más eficiente.

iAtros utiliza para realizar este cálculo la librería FFTW 3. FTW es una librería escrita en C para el cálculo de la DFT en una o más dimensiones con un tamaño de entrar arbitrario que soporta datos reales o complejos. Esta librería es software libre, tiene un buen rendimiento y está soportada sobre varias arquitecturas. Hay más información sobre ella, incluidos artículos en: http://www.fftw.org/

El tamaño de la transformada de Fourier es la mitad del tamaño de la ventana. Cada elemento de la nueva serie se representa en una escala que va desde 1 hasta F/2, donde F es la frecuencia de la señal.

<b>Ejemplo</b>:

	Si grabamos una señal a 16KHz y aplicamos una ventan de 410 elementos, la transformada de Fourier tendrá 205 elementos y representará frecuencias desde 0 hasta 8000Hz.

La aplicación de la FFT se realiza en calculateFFT().

\subsection mel Mel Filter Bank
La FFT proporciona un banco de filtros uniforme: cada valor muestra la energía concentrada en una banda de frecuencias de tamaño fijo. El oído humano no discrimina todos los rangos de frecuencia por igual, por lo que reproducir su comportamiento mejorará la calidad del reconocimiento.
Para conseguir esto se utiliza un banco de filtros en escala logarítmica (similar a la del oído humano) conocida como escala de Mel. La escala de Mel se puede calcular así:
\verbatim
Fmel(f)=2595 log10(1+f/7000)
\endverbatim
Típicamente se subdivide el espectro en 21 o 23 bandas de frecuencia. Cada una de esas bandas tiene, como se ha comentado, un tamaño fijo en la FFT, ahora lo que se hace es, primero, cambiar la escala de esas bandas de frecuencia y segundo aplicar en cada una de ellas un filtro. Este filtro pondera las frecuencias de cada banda. Normalmente se utiliza un filtro triangular o trapezoidal. iAtros puede utilizar filtros trapezoidales o triangulares.

En el caso de los filtros trapezoidales hay que incluirlos en el fichero de configuración. Cada filtro se especifica mediante la frecuencia inicial y la final y el número de frecuencias que recoge.

<b>Ejemplo</b>:

A nuestra señal del punto anterior le vamos a aplicar un filtro trapezoidal en la banda 8, que se define en el fichero de configuración como:<br>
f8	751.000	907.000	5.000<br>
Esta línea nos indica que los elementos de la FFT afectados por ese filtro son los que se encuentran entre la frecuencia 751 y 907, en total 5 valores.
El filtro trapezoidal tiene la forma:<br>
 
\verbatim
    ________
   /        \
  /          \ 
 /            \
/              \

\endverbatim
	
La fomra más sencilla de calcular el filtro es asociar los dos valores de los extremos a los lados del trapecio y el resto al centro:

\verbatim
    _________
   / |  |  | \
  /  |  |  |  \
 /|  |  |  |  |\
/ |  |  |  |  | \
  1  2  3  4  5

\endverbatim

Así el resultado final de la aplicación del filtro a la banda 8 se calcula como:
\verbatim
( fft(n+1)/2 + fft(n+2) + fft(n+3) + fft(n+4) + fft(n+5)/2 ) / 5
\endverbatim
Donde n es el número de elementos de la FFT ya utilizados en filtros anteriores.

Excepto en el primer y último filtro, los valores laterales se comparten entre filtros, así:

\verbatim
    _________    _________
   / |  |  | \  /         \
  /  |  |  |  \/           \ 
 /|  |  |  |  /\            \
/ |  |  |  | /| \            \
  1  2  3  4  5

\endverbatim

La función applyFilterBank() realiza estos cálculos.

Otra opción es utilizar los filtros triangulares. En este caso basta con especificar en el ficehro de configuración el número de filtros (23 normalmente) y el propio programa los calcula y aplica.

La función applyTriangularFilterBank() es la encargada.

Se puede decidir si se utilizan unos u otros mediante el fichero de configuración.

\subsection log Log
Aplicando el logaritmo a los canales de energía del banco de filtros se reduce su sensibilidad a los sonidos muy altos o muy bajos y se modela la sensibilidad no lineal del oído humano. Sin el logaritmo la precisión del reconocimiento de reduce drásticamente.

Este cálculo está incluido al final de la aplicación de los filtros.

\subsection cosine Discrete Cosine Transform (DCT)
El último paso para obtener los MFCC es aplicar una transformada discreta coseno al banco de filtros. Esta última transformación consigue dos cosas:

 a) Separa la variación lenta del espectro (la información del tracto vocal) de la rápida variación de la excitación del habla. Se consigue así disociar los generadores de ruido e impulsos de los filtros lineales de los que hablábamos al principio. Para el reconocimiento del habla sólo nos es útil los primeros coeficientes de esta transformada (el tracto vocal), así que sólo calculamos los primeros, normalmente los 11 o 13 primeros.

 b) El segundo objetivo conseguido es algo más complicado de explicar. Los elementos del banco de filtros muestran correlación debido a las características del espectro del habla y a la naturaleza de los filtros. Para poder modelar con precisión característica correlacionadas utilizando clasificadores estadísticos (como los HMMs) es necesario utilizar matrices de covarianzas completas, que son muy costosas de estimar computacionalmente y ademaś requieren gran cantidad de datos. Para eliminar esa correlación se utiliza la transformada de Karhunene-Loeve, que necesita ser estimada a partir de un conjunto de entrenamiento. Sin embargo una buena aproximación a esta transformada es la DCT. Esto significa que aplicar la DCT permite que el vector de característica pueda ser utilizado para estimar matrices de covarianza diagonales en los clasificadores estadísticos.

 La DCT se obtiene con extractCC().

\subsection energy Frame Energy
Un último elemento a tener en cuenta es la medida de la energía de cada ventana de habla, que produce una mejora significativa en el reconocimiento del habla. El primer elemento de la DCt (el 0) es la suma de las log-energías de cada canal del banco de filtros y puede ser considerado como una medida geométrica de la energía de la ventana. 

Otra alternativa es calcular la log energía LnEi de las ventanas en el dominio del tiempo, sin preénfasis. En la práctica ambas medidas son parecidas, y sólo es necesario calcular una de ellas para que se incluya en el vector de características.


\section bib Bibliography
Sheng-Hua Tan, Borge Lindberg, Automatic Speech Recognition on Mobile Device and over communication networks, Chapter 6, Springer 2008
European Telecommunications Standards Institute, Speech Processing, Transmission and Quality Aspects (STQ); Distributed speech recognition; Front-end feature extraction algorithm; Compression algorithms, Standard ETSI ES 201 108 V1.1.3 (2003-09)
*/

/*! \page audio Audio Notes

\section formats Formatos de Audio
La grabación de la señal se realiza mediante la librería ALSA (advanced Linux Sound Architecture) que abstrae los dispositivos, facilita la labor del programador y permite que se pueda trabajar con cualquier tarjeta de sonido soportado por los drivers ALSA. 

El sistema permite la lectura de señales grabadas en formatos:

- El formato <em>raw</em> (readFileRaw()) codifica cada valor de la señal como un entero con signo de 16 bits.
Es importante tener en cuenta que los procesadores x86 son Little Endian y así se guardan los bits de audio.
Corresponde a la nomenglatura SB16_LE del comando <em>aplay</em>.

- El formato <em>AD</em> (readFileAD()) es igual que el anterior pero incorpora una cabecera de 1024 bytes (que iATROS no tiene en cuenta).

- Los únicos ficheros <em>WAV</em> (readFileWAV()) compatibles con iATROS son aquellos sin ningún tipo de compresión.

Para convertir un fichero desde un WAV no compatible a raw, se puede usar <em>sox</em>:
\verbatim
sox -r rate in.wav -t raw out.raw
\endverbatim

Donde <em>rate</em> es la frecuencia de grabación (no suele ser necesaria porque sox la toma de la cabecera del fichero WAV).

\section param Parámetros en el fichero de configuración
Los parámetros que se pueden variar para la adquisición/carga y preproceso de la señal son los siguientes:

\li \c FrameSize Tamaño de la ventana.
\li \c SampleFreq Frecuencia de muestreo a la que se realiza la grabación.
\li \c SubSampleFreq Al recorrer la señal mediante una ventana se está realizando un submuestreo de la señal.
\li \c FFTlength Tamaño de la transformada de Fourier. 
\li \c FrameShift Número de elementos que se desplaza la ventana cada vez.
\li \c WindowLen Tamaño de la ventana en segundos
\li \c Factor Factor de preénfasis
\li \c WindowAlfa Alfa para la ventan de Hamming
\li \c CepCoefNumb Número de características a extraer por cada ventana
\li \c Channels	Canales de audio (1 Mono, 2 Estereo)
\li \c Bits Bits de la señal
\li \c Frames Es un valor de ALSA relacionado con la capacidad del buffer de la tarjeta de sonido. Un vlaor estándar es 32.
\li \c SilenceThreshold	Umbral de energía por debajo del cual se considera que hay silencio
\li \c SecondsSilence Segundos que han de pasar de silencio para que se considere que hay un silencio
\li \c Derivative Derivadas que se extraen 0 para ninguna, máximo 2

Hay que tener en cuenta que algunos de estos valores están ligados:
\verbatim
FrameSize = WindowLen + SampleFreq + 0.5
FrameShift = ( SampleFreq / SubSampleFreq ) + 0.5
\endverbatim

Se adjunta, a modo de ejemplo, un fichero de configuración:
\verbatim
<filter>
TriangularFilters 23
#FilterBankSize	21
#f1	0.000	94.000	2.000
#f2	94.000	188.000	3.000
#f3	188.000	282.000	3.000
#f4	282.000	376.000	3.000
#f5	376.000	501.000	4.000
#f6	501.000	626.000	4.000
#f7	626.000	751.000	4.000
#f8	751.000	907.000	5.000
#f9	907.000	1063.000	5.000
#f10	1063.000	1251.000	6.000
#f11	1251.000	1470.000	7.000
#f12	1470.000	1720.000	8.000
#f13	1720.000	2001.000	9.000
#f14	2001.000	2314.000	10.000
#f15	2314.000	2689.000	12.000
#f16	2689.000	3127.000	14.000
#f17	3127.000	3690.000	18.000
#f18	3690.000	4378.000	22.000
#f19	4378.000	5284.000	29.000
#f20	5284.000	6378.000	35.000
#f21	6378.000	7566.000	37.000
</filter>

<parameters>
FrameSize	410
SampleFreq	16000
SubSampleFreq	100
FFTlength	512
FrameShift	160
WindowLen	0.025625
Factor	0.97
WindowAlfa	0.54
CepCoefNumb	11
Channels	1
Bits	16
Frames	32
SecondsSilence	0.5
SilenceThreshold	50
Derivative	2
</parameters>

<buffers>
SizeSignal	100000
SizeCC	1000
</buffers>

<device>
InputDevice	default
OutputDevice	default
</device>
\endverbatim

*/
