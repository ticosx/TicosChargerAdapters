#ifndef CHARGER_h
#define CHARGER_h

#include <Arduino.h>
#include "ChargerAdapter.h"
//ADC头文件
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"

typedef enum {
    BAT_NO_CHARGE = 0,  /* */
    BAT_CHARGING,
    BAT_CHARGE_DONE,
} chg_status_t;

#define BAT_TABLE_LEN       101 //电池电量
const uint16_t ChargingTable[BAT_TABLE_LEN]=
{
    1073,1116,1136,1154,1166,1171,1174,1176,1178,1181,/*0~9*/
    1185,1188,1191,1193,1196,1199,1201,1203,1206,1208,/*10~19*/
    1210,1212,1214,1215,1216,1218,1219,1220,1221,1223,/*20~29*/
    1224,1226,1227,1228,1230,1231,1233,1235,1237,1239,/*30~39*/
    1240,1243,1245,1247,1250,1252,1255,1258,1260,1263,/*40~49*/
    1266,1270,1272,1276,1279,1282,1285,1288,1291,1293,/*50~59*/
    1296,1298,1301,1303,1305,1307,1310,1312,1314,1316,/*60~69*/
    1318,1320,1322,1324,1326,1328,1331,1334,1336,1339,/*70~79*/
    1342,1346,1349,1351,1354,1357,1359,1361,1363,1365,/*80~89*/
    1366,1368,1370,1372,1375,1377,1380,1383,1386,1390,/*90~99*/
    1395                                              /*100*/
};

const uint16_t NoChargeTable[BAT_TABLE_LEN]=
{

    1391,1378,1372,1365,1360,1355,1351,1347,1343,1340,/*91~100*/
    1337,1334,1332,1331,1329,1328,1326,1325,1323,1321,/*81~90*/
    1319,1317,1315,1312,1310,1307,1305,1302,1299,1297,/*71~80*/
    1294,1292,1290,1288,1286,1284,1282,1280,1278,1275,/*61~70*/
    1273,1271,1269,1266,1264,1261,1259,1256,1253,1250,/*51~60*/
    1247,1245,1241,1238,1235,1232,1229,1226,1224,1221, /*41~50*/
    1218,1216,1213,1211,1209,1207,1205,1203,1201,1200,/*31~40*/
    1198,1196,1195,1193,1192,1190,1189,1187,1186,1184,/*21~30*/
    1182,1181,1179,1177,1175,1173,1171,1169,1167,1165,/*11~20*/
    1162,1160,1157,1154,1151,1148,1145,1142,1139,1136,/*1~10*/
    1133                                              /*0*/

};
/*!
 * @brief The class of charger
 */
class TkCharger: public ChargerAdapter{
    public:
        TkCharger(chg_info_t *info);
        /*!
        *    @brief  获取电池电量
        *    @return 初始化成功则返回电量值，单位是%（0-100%）
        */
        virtual  uint8_t getBatteryCap(void) override;
        /*!
        *    @brief  获取电池电压值
        *    @return 初始化成功则返回电压值，单位是mV
        */
        virtual  uint32_t getBatteryVolt(void) override;
        /*!
        *    @brief  获取电池Adc原始值，12bit
        *    @return 初始化成功则返回原始ADC值
        */
        virtual  uint32_t getBatteryAdcValue(void) override;
        /*!
        *    @brief  获取充电状态，充电、放电等状态
        *    @return 初始化成功则返回充电状态
        */
        virtual  uint8_t getChargerStatus(void) override;
        /*!
        *    @brief  初始化设备
        *    @return 初始化成功则返回 true
        */
        virtual bool init() override;
        /*!
        *    @brief  取消初始化设备
        *    @return 取消成功返回 true
        */
        virtual bool deinit() override;
    private:
        bool begin();
        bool end() ;
        gpio_num_t adcio;
        gpio_num_t detio;
        adc1_channel_t chn = ADC1_CHANNEL_4;
        uint16_t vef = 3300;
        uint32_t u32AdcQueue[10];
};

#endif