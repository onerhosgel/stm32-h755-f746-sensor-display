\# STM32H755 Sensor Node + STM32F746G TouchGFX Display



This project uses two STM32 boards for sensor reading and display.



The STM32H755 reads several sensors over I2C and sends the measurements to an STM32F746G-DISCO board over UART. The F746G receives the data and displays the values on its LCD using TouchGFX.



The main purpose of the project was to combine sensor communication, UART communication between two MCUs and a graphical interface in one system.



\## Hardware



STM32H755 side:



\- TH09C temperature and humidity sensor

\- BMP180 pressure sensor

\- TCS34725 color sensor

\- I2C communication

\- UART transmission



STM32F746G-DISCO side:



\- UART reception

\- Packet parsing

\- TouchGFX

\- Integrated LCD



The general data flow is:



```text

TH09C

BMP180

TCS34725

&#x20;  |

&#x20;  | I2C

&#x20;  v

STM32H755

&#x20;  |

&#x20;  | UART

&#x20;  v

STM32F746G-DISCO

&#x20;  |

&#x20;  v

TouchGFX LCD

```



\## How it works



The sensors are connected to the STM32H755 through the same I2C bus.



The H755 reads temperature, humidity, pressure and color values. These values are collected and converted into a UART packet.



The packet format is:



```text

$TEMP,HUM,PRESS,R,G,B,CLEAR\*

```



For example, a packet contains the following values:



```text

Temperature

Humidity

Pressure

Red

Green

Blue

Clear

```



UART transmission on the H755 side is handled with DMA.



On the STM32F746G side, USART6 receives the incoming bytes. When a complete packet is received, the values are parsed and passed to the TouchGFX application.



The LCD cycles through the received sensor values.



\## Code structure



I tried to keep the sensor and communication code separate from `main.c`.



Most of the H755 application code is under:



```text

STM32H755\_SensorNode/CM7/Core/

```



The main files are:



```text

bmp180.c / bmp180.h

th09c.c / th09c.h

tcs34725.c / tcs34725.h

sensor\_manager.c / sensor\_manager.h

uart\_packet.c / uart\_packet.h

frequency\_meter.c / frequency\_meter.h

```



`sensor\_manager` is used to collect the sensor readings in one place.



`uart\_packet` is responsible for preparing and sending the data packet.



The individual sensor files contain the I2C communication and conversion code for each sensor.



The F746 project is located under:



```text

STM32F746G\_Display/

```



The TouchGFX screen code is under:



```text

STM32F746G\_Display/TouchGFX/gui/

```



\## Frequency measurement



There is also a timer-based frequency measurement part in the H755 project.



The related code is kept in:



```text

frequency\_meter.c

frequency\_meter.h

```



This part uses an STM32 timer separately from the sensor communication code.



\## Repository structure



```text

stm32-h755-f746-sensor-display/

|

|-- STM32H755\_SensorNode/

|

|-- STM32F746G\_Display/

|

|-- docs/

|   |-- images/

|   `-- video/

|

`-- README.md

```



\## Wiring



The wiring diagram will be added here together with the project photos.



The important connections are:



```text

Sensors -> STM32H755 I2C



STM32H755 UART TX -> STM32F746G USART6 RX



STM32H755 GND -> STM32F746G GND

```



The I2C bus uses pull-up resistors on SDA and SCL.



\## Demo



A short hardware demo will also be added.



The demo will show the STM32H755 reading the sensors and sending the values to the STM32F746G display in real time.



\## Tools



\- STM32CubeMX

\- STM32CubeIDE

\- STM32 HAL

\- TouchGFX Designer

\- FreeRTOS

\- Git



\## Author



Öner Hoşgel  

Electrical-Electronics Engineering

