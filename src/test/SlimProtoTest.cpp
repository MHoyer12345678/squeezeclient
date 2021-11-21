/*
 * SlimProtoTest.cpp
 *
 *  Created on: 23.01.2021
 *      Author: joe
 */

#include "SlimProtoTest.h"

#include <cpp-app-utils/Logger.h>
#include <glib-unix.h>
#include <linux/input.h>

using namespace CppAppUtils;

namespace slimprotolib {

SlimProtoTest* SlimProtoTest::instance=NULL;

SlimProtoTest::SlimProtoTest() :
		returnCode(0),
		keyEvents(NULL)
{
	this->mainloop=g_main_loop_new(NULL,FALSE);
	this->squeezeClient=SqueezeClient::NewWithGstPlayer();
}

SlimProtoTest::~SlimProtoTest()
{
	SqueezeClient::Destroy(this->squeezeClient);
	g_main_loop_unref (this->mainloop);
}

SlimProtoTest* SlimProtoTest::Instance()
{
	if (SlimProtoTest::instance==NULL)
		SlimProtoTest::instance=new SlimProtoTest();

	return SlimProtoTest::instance;
}

bool SlimProtoTest::Init(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Logger::LogDebug("SlimProtoTest::Init - Initalizing SlimProtoTest");

	if (!this->squeezeClient->Init())
		return false;

    g_unix_signal_add(1, &UnixSignalHandler, this);
    g_unix_signal_add(2, &UnixSignalHandler, this);
    g_unix_signal_add(15, &UnixSignalHandler, this);
    this->keyEvents = g_io_channel_new_file("/dev/input/by-path/platform-i8042-serio-0-event-kbd", "r", nullptr);
    g_io_channel_set_encoding(this->keyEvents, NULL, NULL);
    g_io_add_watch_full(this->keyEvents, G_PRIORITY_HIGH, G_IO_IN, SlimProtoTest::OnKeyEvent, this, NULL);

   	return true;
}

void SlimProtoTest::DeInit()
{
	this->squeezeClient->DeInit();
	delete this;
	SlimProtoTest::instance=NULL;

	Logger::LogDebug("SlimProtoTest::DeInit - DeInitalized SlimProtoTest");
}

gboolean SlimProtoTest::UnixSignalHandler(gpointer user_data)
{
	SlimProtoTest *instance=(SlimProtoTest *)user_data;
	g_main_loop_quit(instance->mainloop);
	return TRUE;
}

void SlimProtoTest::Run()
{
    this->squeezeClient->KickOff();
	Logger::LogDebug("SlimProtoTest::Run -> Going to enter main loop.");
	g_main_loop_run(this->mainloop);
	Logger::LogDebug("SlimProtoTest::Run -> Main loop left. Shutting down.");
}

gboolean SlimProtoTest::OnKeyEvent(GIOChannel *source,
		GIOCondition condition, gpointer data)
{
	SlimProtoTest *instance=(SlimProtoTest *)data;
	struct input_event inputEv;
	g_io_channel_read_chars(source, (gchar *)&inputEv, sizeof(struct input_event),NULL, nullptr);
	instance->OnKeyPressed(inputEv.type, inputEv.code, inputEv.value);
	return TRUE;
}

void SlimProtoTest::OnKeyPressed(uint16_t type, uint16_t code, uint32_t value)
{
	if (type != EV_KEY || value != 0) return;

	switch(code)
	{
	case 106:
		//left -> next
		this->squeezeClient->SignalNextButtonPressed();
		break;
	case 105:
		//right -> prev
		this->squeezeClient->SignalPreviousButtonPressed();
		break;
	case 103:
		//top -> volUp
		this->squeezeClient->SignalVolUpButtonPressed();
		break;
	case 108:
		//down -> volDown
		this->squeezeClient->SignalVolDownButtonPressed();
		break;
	case 25:
		//p -> power
		this->squeezeClient->SignalPowerButtonPressed(SqueezeClient::POWER_TOGGLE);
		break;
	case 38:
		//l -> power off
		this->squeezeClient->SignalPowerButtonPressed(SqueezeClient::POWER_OFF);
		break;
	case 39:
		//ö -> power on
		this->squeezeClient->SignalPowerButtonPressed(SqueezeClient::POWER_ON);
		break;
	case 16:
		//q -> fake faster
		this->squeezeClient->SignalFakeFaster();
		break;

	case 30:
		//a -> fake slower
		this->squeezeClient->SignalFakeSlower();
		break;

/*	default:
		Logger::LogError("Keycode: %d",code);*/
	}
}

int SlimProtoTest::GetReturnCode()
{
	return this->returnCode;
}

} /* namespace slimprotolib */
