#include <mbed.h>
#include <MQTTClientMbedOs.h>
#include <vector>
#include <string>

#ifdef TARGET_DISCO_L475VG_IOT01A
#include "VL53L0X.h"
#endif


/* Interface definition */
#ifdef TARGET_DISCO_L475VG_IOT01A
static DevI2C devI2c(PB_11,PB_10);
#else // X-Nucleo-IKS01A2 or SensorTile
#ifdef TARGET_SENSOR_TILE
#define SPI_TYPE_LPS22HB   LPS22HBSensor::SPI3W
#define SPI_TYPE_LSM6DSL   LSM6DSLSensor::SPI3W
SPI devSPI(PB_15, NC, PB_13);  // 3-wires SPI on SensorTile  
static Serial ser(PC_12,PD_2); // Serial with SensorTile Cradle Exp. Board + Nucleo   
#define printf(...) ser.printf(__VA_ARGS__)     
#else  // Nucleo-XXX + X-Nucleo-IKS01A2 
static DevI2C devI2c(D14,D15);
#endif
#endif

/* Range sensor - B-L475E-IOT01A2 only */
#ifdef TARGET_DISCO_L475VG_IOT01A
static DigitalOut shutdown_pin(PC_6);
static VL53L0X range(&devI2c, &shutdown_pin, PC_7);
#endif


struct Names {
  enum type { toa, bank, inn, o, por, menghorng, michael, wari, aoff, test};
};

DigitalOut led(LED1), led2(LED2);
WiFiInterface *wifi;
TCPSocket* socket;
MQTTClient* mqttClient;

std::vector<std::string> CLIENT_ID = {  "ff56abf3-f01e-4d50-bfb3-8090a87bd846" };

std::vector<std::string> NETPIE_TOKEN = { "y2eyNmnZwNnxhVev1X9mqKjnvWm4HSqS" };

std::vector<std::string> MQTT_TOPIC = { "@msg/taist2020/board/F1_1"};

int8_t name = Names::toa;
int8_t device_id = name;
unsigned long seq = 1;

int main() {

    
    // WiFi connection
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    int16_t ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }

    // Socket connection
    socket = new TCPSocket();
    socket->open(wifi);
    SocketAddress addr;
    wifi->gethostbyname("mqtt.netpie.io", &addr);
    addr.set_port(1883);
    socket->connect(addr);
    if (ret != 0) {
        printf("rc from TCP connect is %d\r\n", ret);
        return -1;
    }

    // MQTT connection
    mqttClient = new MQTTClient(socket); 
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID.cstring = (char*)CLIENT_ID[name].c_str();
    data.username.cstring = (char*)NETPIE_TOKEN[name].c_str();
    //data.password.cstring = (char*)NETPIE_SECRET;
    ret = mqttClient->connect(data);
    if (ret != 0) {
        printf("rc from MQTT connect is %d\r\n", ret);
        return -1;
    }

    /***
    // Start Read sensor
    float sensor_value = 0;
    int16_t pDataXYZ[3] = {0};
    float pGyroDataXYZ[3] = {0};
	
    printf("Sensor init : start\n");
    BSP_GYRO_Init();
    BSP_GYRO_LowPower(0);
    BSP_ACCELERO_Init();
    BSP_ACCELERO_LowPower(0);
    printf("Sensor init : complete\n");
    ***/
    printf("MQTT init : start\n");
    MQTT::Message message;
    char buf[100];
    printf("MQTT init : complete\n");
#ifdef TARGET_DISCO_L475VG_IOT01A
    range.init_sensor(VL53L0X_DEFAULT_ADDRESS);
#else // X-NUCLEO-IKS01A2 or SensorTile
    accelerometer.init(NULL);
#endif
    Timer t;
    t.start();
    float l_time[9], previous_t=0.0;
    int32_t rpy[3] = {0};
    t.reset();
    previous_t=t.read();
    while(1) { //P'Bo code
    #ifdef TARGET_DISCO_L475VG_IOT01A
        uint32_t distance;
        int status = range.get_distance(&distance);
        if (status == VL53L0X_ERROR_NONE) {
            printf("\n\rVL53L0X [mm]:            %6ld\r\n", distance);
        } else {
            printf("\n\rVL53L0X [mm]:                --\r\n");
        }

        if(distance >= 1500){
            printf("\n\rAvailable");
            sprintf(buf, "{\"data\":\"available\"}");
        } else { 
            printf("\n\rOccupied");
            sprintf(buf, "{\"data\":\"occupied\"}");
            
        }
    #endif
        message.payload = (void*)buf;
        message.payloadlen = strlen(buf)+1;
        message.qos = MQTT::QOS1;
        message.retained = false;
        message.dup = false;

        if( t.read()-previous_t > 1.0 ) {
          ret = mqttClient->publish(MQTT_TOPIC[name].c_str(), message);
          printf("Published: %s\n\r", buf);
          if (ret != 0)
              printf("rc from publish was %d\r\n", ret);
          seq=seq+1;
          led2=1;
          previous_t = t.read();
        }
        else if( t.read()-previous_t > 1.0 ) //second
          led2=0;
        led = !led;
    } //while
    t.stop();

} //main

