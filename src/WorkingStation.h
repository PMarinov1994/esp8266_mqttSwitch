#pragma once
#include <WString.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiSTA.h>
#include <FS.h>
#include <Wire.h>

#include <PubSubClient.h>
#include "GlobalDefinitions.h"


enum WorkMode
{
	NORMAL = 0,
	POWER_SAVE = 1
};

class CWorkingStation
{
public:
	CWorkingStation()
		: m_client(m_espClient)
		, m_switchState(LOW)
	{
	};

	virtual ~CWorkingStation()
	{
		if (m_ssid)
			delete[] m_ssid;

		if (m_psk)
			delete[] m_psk;
	};

	bool Init();
	void Work();

private:

	bool ReconnectMQTT();
	void DeepSleepTask(int secs);

	void ConnectToWifi();
	void PublishSwitchState();

	void MQTT_Callback(char* topic, uint8_t* payload, unsigned int length);

private:

	char* m_ssid;
	char* m_psk;

	PubSubClient m_client;
	WiFiClient m_espClient;

	uint8_t m_switchState;
};