/*
 * PlayerController.h
 *
 *  Created on: 25.01.2021
 *      Author: joe
 */

#ifndef SRC_PLAYERCONTROLLER_H_
#define SRC_PLAYERCONTROLLER_H_

#include "LMSConnection.h"
#include "SqueezeClient.h"
#include "CommandFactory.h"
#include "IPlayer.h"


namespace squeezeclient {

class PlayerController : public LMSConnection::IConnectionListener,
	public CommandFactory::IServerCmdListener, public IPlayer::IPlayerEventListener
{
private:
	SqueezeClient::SqueezeClientStateT clientState;

	SqueezeClient::IEventInterface *squeezeClientEventInterface;

	IPlayer::PlayerStatusT lmsPlayerStatusData;

	LMSConnection *lmsConnection;

	CommandFactory *commandFactory;

	IPlayer *player;

	void UpdatePlayerStatusData();

public:
	//----------- IPlayerEventListener ---------------------------------------
	virtual void OnConnectionResponseReceived(const char *responseStr);

	virtual void OnReadyToPlay();

	virtual void OnTrackEnded();

	virtual void OnPlayerStateChanged(IPlayer::PlayerStateT state);

	//----------- IConnectionListener ---------------------------------------
	virtual void OnConnectionEstablished();

	virtual void OnConnectionLost(bool isClosedByClient);

	virtual void OnCommandReceived(void *data, uint16_t cmdSize);

	//----------- IServerCmdListener ---------------------------------------
	virtual void OnSrvRequestedLoadStream(IPlayer::StreamingServerInfoT *srvInfo,
			IPlayer::AudioFormatT *audioFMT, bool autostart);

	virtual void OnSrvRequestedPlayerStatus(uint32_t replayGain);

	virtual void OnSrvRequestedPause(uint32_t pauseDuration);

	virtual void OnSrvRequestedStop();

	virtual void OnSrvRequestedUnPause(uint32_t unpauseTime);

	virtual void OnSrvRequestedSkippingFrames(uint32_t skippingDuration);

	virtual void OnSrvRequestedFlush();

	virtual void OnSrvRequestedPlayerName();

	virtual void OnSrvSetNewPlayerName(const char *playerName);

	virtual void OnSrvRequestedAudioEnabledChange(bool spdiffEnabled, bool dacEnabled);

	virtual void OnSrvRequestedVolumeChange(unsigned int volL, unsigned int volR, bool adjustLocally);

	virtual void OnSrvRequestedDisableDACSetting();

	virtual void OnSrvSetDisableDACSetting(bool value);

public:
	PlayerController(SqueezeClient::IEventInterface *evIFace, IPlayer *player);

	~PlayerController();

	bool Init();

	void DeInit();

	void KickOff();

	void SignalPowerButtonPressed(SqueezeClient::PowerSignalT powerSignal);

	void SignalPlayButtonPressed();

	void SignalPauseButtonPressed();

	void SignalNextButtonPressed();

	void SignalPreviousButtonPressed();

	void SignalVolUpButtonPressed();

	void SignalVolDownButtonPressed();

	void SignalMuteButtonPressed();

	SqueezeClient::SqueezeClientStateT GetClientState();

#warning Remove following functions later

	void SignalFakeFaster();

	void SignalFakeSlower();
};

} /* namespace squeezeclient */

#endif /* SRC_PLAYERCONTROLLER_H_ */
