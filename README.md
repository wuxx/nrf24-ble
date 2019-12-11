由于nrf24l01芯片和蓝牙BLE的物理层均使用GFSK进行调制，因此可在nrf24l01软件中稍加改造，构造出ble广播包，实现一个beacon设备，可向外持续广播自定义数据。
详见
https://www.muselab-tech.com/zhi-zuo-zui-lian-jie-de-bleyan-bo-she-bei/  

PIN CONNECT  
nrf24l01    arduino     stm32  
VCC         3.3V        3.3V  
GND         GND         GND  
CE          10          PA4  
CSN         9           PA3  
SCK         13          PA5  
MOSI        11          PA7  
MISO        12          PA6  
IRQ         NC          NC  
