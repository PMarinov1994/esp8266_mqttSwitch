#include "WorkingStation.h"
#include "ConfigurationFile.h"



/*
*	\brief Initialize the component
*
*/
bool CWorkingStation::Init()
{
    pinMode(SWITCH_PIN, OUTPUT);
    digitalWrite(SWITCH_PIN, m_switchState);

    pinMode(BUILD_IN_LED, OUTPUT);
    digitalWrite(BUILD_IN_LED, BUILD_IN_LED_OFF);

    CConfigurationFile configFile;
    configFile.ParseConfiguration();

    int ssidLen = strlen(configFile.m_ssid) + 1;
    int pskLen = strlen(configFile.m_psk) + 1;

    if (m_ssid)
        delete[] m_ssid;

    m_ssid = new char[ssidLen];

    if (m_psk)
        delete[] m_psk;

    m_psk = new char[pskLen];

    memcpy(m_ssid, configFile.m_ssid, ssidLen);
    memcpy(m_psk, configFile.m_psk, pskLen);

    ConnectToWifi();

    IPAddress mqttServerIPAddr;
    mqttServerIPAddr.fromString(configFile.m_mqttServerIP);

    m_client.setServer(mqttServerIPAddr, configFile.m_mqttServerPort);
    
    MQTT_CALLBACK_SIGNATURE = std::bind(
        &CWorkingStation::MQTT_Callback,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    m_client.setCallback(callback);

    // Connect to MQTT Broker.
    ReconnectMQTT();

    while (!m_client.subscribe(outside_switch_topic, MQTTQOS0))
    {
        Println("Subscribe failed. Trying again...");
        delay(500);
    };

    // Check the supply voltage.
    float espVcc = (ESP.getVcc() / 1000.0) + 0.005; // In Volts
    Print("Battery VCC: ");
    Println(espVcc);

    if (espVcc < ESP_MIN_OPERATE_VOLTAGE)
    {
        Println("Low on power. Not going to work");

        // Ender Deep Sleep Mode for 15 mins.
        this->DeepSleepTask(60 * 15);

        return false;
    }

    return true;
}

uint8_t uiLedState = LOW;

/*
*	\brief This will loop in main
*/
void CWorkingStation::Work()
{
    if (!m_client.loop())
    {
        digitalWrite(BUILD_IN_LED, BUILD_IN_LED_OFF);
        Println("Client is disconnected. Will try to reconnect in a moment.");
        
        int mqttState = m_client.state();
        Print("The Mqtt state was: ");
        Println(mqttState);

        this->ReconnectMQTT();
    }

    delay(10);
}

/*
********************** MQTT ERRORS **********************
* 
* TODO: This needs to be made into map and returned somewhere.
    -4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
    -3 : MQTT_CONNECTION_LOST - the network connection was broken
    -2 : MQTT_CONNECT_FAILED - the network connection failed
    -1 : MQTT_DISCONNECTED - the client is disconnected cleanly
    0 : MQTT_CONNECTED - the client is connected
    1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
    2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
    3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
    4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
    5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
*/

/*
*	\brief This will loop in main
*/
bool CWorkingStation::ReconnectMQTT()
{
    while (!m_client.connected())
    {
        if (m_client.connect(stationID))
        {
            Println("Client connected!");
            PublishSwitchState();            

            digitalWrite(BUILD_IN_LED, BUILD_IN_LED_ON);
            return true;
        }
        else
        {
            Print("failed, rc=");
            Print(m_client.state());
            Println(" try again in 2 seconds");
            // Wait 2 seconds before retrying
            delay(2000);
        }
    }

    return false;
}



void CWorkingStation::ConnectToWifi()
{
    WiFi.mode(WIFI_STA);
    delay(20);

    WiFi.begin(m_ssid, m_psk);

    Print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Print(".");
    }

    Println("");
    Print("Connected, IP address: ");
    Println(WiFi.localIP());
}

void CWorkingStation::PublishSwitchState()
{
    if (!m_client.connected())
        return;

    Println("Tell the world what the switch is up to.");

    String pinValue(m_switchState);
    bool pubStateResult = m_client.publish(outside_switch_state_topic, pinValue.c_str(), false);

    Print("The publish of state was: ");
    Println(pubStateResult);
}



void CWorkingStation::MQTT_Callback(char* topic, uint8_t* payload, unsigned int length)
{
    Print("Got message! Topic: ");
    Println(topic);

    if (0 != strcmp(topic, outside_switch_topic))
    {
        Println("Not my topic.");
        return;
    }

    if (0 == length)
    {
        Println("Payload length is 0.");
        return;
    }

    String pinValue((char)payload[0]);
    Print("The pinValue in payload was: ");
    Println(pinValue);

    m_switchState = pinValue == "0" ? LOW : HIGH;
    digitalWrite(SWITCH_PIN, m_switchState);

    PublishSwitchState();
}



void CWorkingStation::DeepSleepTask(int secs)
{
    long sleepUs = 1000000 * secs; // 1 second = 1 000 000 microseconds

    Println("Entering deep sleep mode...");
    Print("Wakeup after (s): ");
    Println(secs);

    digitalWrite(1, LOW);
    SERIAL_END;

    // Wait 1 second for MQTT to finish publishing.
    delay(1000);

    // Disconnect MQTT client.
    m_client.disconnect();
    delay(500);

    ESP.deepSleep(sleepUs);
    delay(100);
}