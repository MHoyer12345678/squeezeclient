/*
 * SlimProtoTest.h
 *
 *  Created on: 23.01.2021
 *      Author: joe
 */

#ifndef SRC_TEST_SLIMPROTOTEST_H_
#define SRC_TEST_SLIMPROTOTEST_H_

#include <glib.h>

#include "SqueezeClient.h"

#include "GstPlayerConfig.h"

namespace squeezeclient {

class SlimProtoTest : public SqueezeClient::IEventInterface, GstPlayerAlsaSinkConfig {
private:

	GIOChannel *keyEvents;

	static SlimProtoTest* instance;

	SqueezeClient *squeezeClient;

	GMainLoop *mainloop;

	int returnCode;

	SlimProtoTest();

	~SlimProtoTest();

	static gboolean UnixSignalHandler(gpointer user_data);

	static gboolean	OnKeyEvent(GIOChannel *source,
	            GIOCondition condition,
	            gpointer data);

	void OnKeyPressed(uint16_t type, uint16_t code, uint32_t value);
public:
	static SlimProtoTest* Instance();

	bool Init(int argc, char *argv[]);

	void DeInit();

	void Run();

	int GetReturnCode();

	void OnPlayerNameRequested(char name[1024]);

	void OnUIDRequested(char uid[16]);

	void OnMACAddressRequested(uint8_t mac[6]);

	void OnServerSetsNewPlayerName(const char *newName);

	void OnClientStateChanged(SqueezeClient::SqueezeClientStateT newState);

	void OnVolumeChanged(unsigned int volL, unsigned int volR);

	const char *GetAlsaDeviceName();
};

} /* namespace squeezeclient */

#endif /* SRC_TEST_SLIMPROTOTEST_H_ */
