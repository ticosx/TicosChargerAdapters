#include "TkCharger.h"
#include <stdio.h>
#include "sdkconfig.h"
//ADC头文件
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
// ADC斜率曲线
static esp_adc_cal_characteristics_t  chg_adc_chars;
#define ADC_QUEUE_SIZE           10


TkCharger::TkCharger(chg_info_t *info) : ChargerAdapter(info) {

}

bool TkCharger::begin(){

    esp_err_t err = ESP_OK;
    err = adc1_config_width(ADC_WIDTH_BIT_12);// 12位分辨率
    if (err) {
        printf("adc1_config_width failed: %d", err);
        return false;
    }
    err = adc1_config_channel_atten(ADC_CHN, ADC_ATTEN_DB_11);// 电压输入衰减
    if (err) {
        printf("adc1_config_channel_atten failed: %d", err);
        return false;
    }
    // 为斜率曲线分配内存
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, ADC_VEF, &chg_adc_chars);
    
    printf("esp_adc_cal_characterize: %d", val_type);
    //set det charger io
    gpio_pad_select_gpio(DET_PIN); //
    gpio_set_direction(DET_PIN, GPIO_MODE_INPUT);// 设置GPIO为输入
   // pinMode(DET_PIN, INPUT_PULLUP);
    return true;
}

bool TkCharger::end(){
    gpio_reset_pin(DET_PIN);
    gpio_reset_pin(ADC_PIN);
    return true;
}

bool TkCharger::init() {

    ADC_CHN = (adc1_channel_t)info->adc_chn;    
    //DET_PIN = static_cast<gpio_num_t>(info->det_pin);
   // ADC_PIN = static_cast<gpio_num_t>(info->adc_pin);
    DET_PIN = (gpio_num_t)info->det_pin;
    ADC_PIN = (gpio_num_t)info->adc_pin;
    ADC_VEF = info->adc_vref;
    printf("TkCharger_init ,ADC_CHN = %d,ADC_VEF= %d,DET_PIN:%d\n",ADC_CHN,ADC_VEF,DET_PIN);

    begin();

    return true;
}
bool TkCharger::deinit() {
    end();
    printf("charger deinit \n");
    return true;
}


/*****************************************************************************
*函数名  : getBatteryVolt
*函数功能 :获取ADC转换电压值
*输入参数 : 无
*输出参数 : 无
*返回值   : vol_average电压值
*****************************************************************************/
uint32_t TkCharger::getBatteryVolt(void)
{
    uint8_t i =0;
    uint32_t ret =0;

    uint32_t adc_max = 0;
    uint32_t adc_min = 0;
    uint32_t adc_sum = 0;
    uint32_t adc_average = 0;
    uint32_t vol_average = 0; //电压值

    for(i=0;i<ADC_QUEUE_SIZE;i++)//一次读取10个
    {
        u32AdcQueue[i]= adc1_get_raw(ADC_CHN);// 采集ADC原始值
    }  

    adc_max = u32AdcQueue[0];
    adc_min = u32AdcQueue[0];

    for(i=0; i<ADC_QUEUE_SIZE; i++)
    {
        if(adc_max < u32AdcQueue[i])
            adc_max = u32AdcQueue[i];
        if(adc_min > u32AdcQueue[i])
            adc_min = u32AdcQueue[i];
        adc_sum +=u32AdcQueue[i];
    }
    adc_sum = adc_sum-adc_max-adc_min;

    /*平均AD值*/
    adc_sum +=(ADC_QUEUE_SIZE-2)/2;
    adc_average = adc_sum/(ADC_QUEUE_SIZE-2);
    //  Ref	= 3.3V
    vol_average =  esp_adc_cal_raw_to_voltage(adc_average, &chg_adc_chars);//根据电阻分压计算电压

    return	vol_average;
}

/*****************************************************************************
*函数名  : getBatteryAdcValue
*函数功能 :获取ADC平均值
*输入参数 : 无
*输出参数 : 
*返回值   : adc_average  ADC平均值
*****************************************************************************/
uint32_t TkCharger::getBatteryAdcValue(void)
{
    uint8_t i =0;
    uint32_t ret =0;

    uint32_t adc_max = 0;
    uint32_t adc_min = 0;
    uint32_t adc_sum = 0;
    uint32_t adc_average = 0;
    uint32_t vol_average = 0; //电压值

    for(i=0;i<ADC_QUEUE_SIZE;i++)//一次读取10个
    {
        u32AdcQueue[i]= adc1_get_raw(ADC_CHN);// 采集ADC原始值
    }  

    adc_max = u32AdcQueue[0];
    adc_min = u32AdcQueue[0];

    for(i=0; i<ADC_QUEUE_SIZE; i++)
    {
        if(adc_max < u32AdcQueue[i])
            adc_max = u32AdcQueue[i];
        if(adc_min > u32AdcQueue[i])
            adc_min = u32AdcQueue[i];
        adc_sum +=u32AdcQueue[i];
    }
    adc_sum = adc_sum-adc_max-adc_min;
    /*平均AD值*/
    adc_sum +=(ADC_QUEUE_SIZE-2)/2;
    adc_average = adc_sum/(ADC_QUEUE_SIZE-2);

    return	adc_average;
}
/*****************************************************************************
*函数名  : getBatteryCap
*函数功能 :获取电池容量
*输入参数 : 无
*输出参数 : 无
*返回值   : i8Cap
*****************************************************************************/
uint8_t TkCharger::getBatteryCap(void)
{
    uint32_t Vot_value = 0;
    uint8_t u8i = 0;
    uint8_t i8Cap = 0;
    uint8_t chg_status = 0;

    Vot_value = getBatteryVolt();
    chg_status = getChargerStatus();

    if(chg_status == BAT_NO_CHARGE )//放电
    {
        for(u8i=0; u8i<BAT_TABLE_LEN; u8i++)//查表
            {
                if(Vot_value >= NoChargeTable[u8i])
                {
                  break;
                }
            }
            i8Cap = 100-u8i;
          printf(" NoChargeTable ,i8Cap:%d,u8i:%d\n",i8Cap,u8i);
    }
    else//充电
    {
        for(u8i=0; u8i<BAT_TABLE_LEN; u8i++)//查表
            {
                if(Vot_value <= ChargingTable[u8i])
                {
                  break;
                }
            }
            i8Cap = u8i-1;
             printf(" ChargingTable ,i8Cap:%d,u8i:%d\n",i8Cap,u8i);
    }
  

    return i8Cap ;
}

/*****************************************************************************
*函数名  : getChargerStatus
*函数功能 :获取充电状态
*输入参数 : 无
*输出参数 : 无
*返回值   : 1：充电，0：放电
*****************************************************************************/
uint8_t TkCharger::getChargerStatus(void)
{
    uint8_t ret = 0;

    if(gpio_get_level(DET_PIN) == 0){
   // if(digitalRead(DET_PIN) == 0){
        ret = 1;
    }else{
        ret = 0;
    }
    ChgStatus = ret ;
    printf("getChargerStatus ,ChgStatus = %d\n",ChgStatus);
    return ret ;
}
