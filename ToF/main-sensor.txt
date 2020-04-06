#include "mbed.h"
#include "VL53L0X.h"


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
static DigitalOut shutdown_pin(PC_6);
static VL53L0X range(&devI2c, &shutdown_pin, PC_7);


/* Simple main function */
#if !MBED_TEST_MODE
int main() {
  uint8_t id;
  float value1, value2;
//  char buffer1[32], buffer2[32];
  int32_t axes[3];
  
  range.init_sensor(VL53L0X_DEFAULT_ADDRESS);



  while(1) {
    printf("\r\n");

    value1=value2=0;    

    uint32_t distance;
    int status = range.get_distance(&distance);
    if (status == VL53L0X_ERROR_NONE) {
        printf("VL53L0X [mm]:            %6ld\r\n", distance);
    } else {
        printf("VL53L0X [mm]:                --\r\n");
    }

    wait(1);
  }
}
#endif