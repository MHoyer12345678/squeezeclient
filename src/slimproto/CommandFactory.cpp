/*
 * CommandFactory.cpp
 *
 *  Created on: 24.01.2021
 *      Author: joe
 */

#include "slimproto/CommandFactory.h"

#include <string.h>
#include <netinet/in.h>
#include <cpp-app-utils/Logger.h>

#include "slimproto/Utils.h"

using CppAppUtils::Logger;

uint8_t MAC_ADDRESS[6]={0x5C,0xE0, 0xC5, 0x49, 0x54, 0xAD};

#define AN_UID "Dies ist eine id"
#define COUNTRY_CODE "DE"

using namespace slimprotolib;
#pragma pack(push, 1)

// client commands
typedef struct CliCmdBaseT
{
	char cmd[4];
	uint32_t messageSize;
} CliCmdBaseT;

typedef struct HeloCliCmdT
{
	CliCmdBaseT cmdBase;
	char deviceId;
	char revision;
	char macAdress[6];
	char UID[16];
	uint16_t wlanChannelList;
	uint32_t dataBytesReceivedH;
	uint32_t dataBytesReceivedL;
	char countryCode[2];
} HeloCliCmdT;

typedef struct ByeCliCmdT
{
	CliCmdBaseT cmdBase;
	uint8_t leavingForUpdate;
} ByeCliCmdT;

typedef struct StatCliCmdT {
	CliCmdBaseT cmdBase;
	char event[4];
	uint8_t  numCrlf;
	uint8_t  masInitialized;
	uint8_t  masMode;
	uint32_t streamBufferSize;
	uint32_t streamBufferFullness;
	uint32_t bytesReceivedH;
	uint32_t bytesReceivedL;
	uint16_t signalStrength;
	uint32_t jiffies;
	uint32_t outputBufferSize;
	uint32_t outputBufferFullness;
	uint32_t elapsedSeconds;
	uint16_t voltage;
	uint32_t elapsedMilliseconds;
	uint32_t serverTimestamp;
	uint16_t errorCode;
} StatCliCmdT;

typedef struct SetdCliCmdT {
	CliCmdBaseT cmdBase;
	char id;
	char name[];
} SetdCliCmdT;

// server commands
typedef struct StrmSrvCmdT {
	char  cmd[4];
	char  subCmd;
	uint8_t  autostart;
	uint8_t  format;
	uint8_t  pcmSampleSize;
	uint8_t  pcmSampleRate;
	uint8_t  pcmChannels;
	uint8_t  pcmEndianness;
	uint8_t  threshold;
	uint8_t  spdifEnable;
	uint8_t  transitionPeriod;
	uint8_t  transitionType;
	uint8_t  flags;
	uint8_t  outputThreshold;
	uint8_t  slaves;
	uint32_t replayGain;
	uint16_t serverPort;
	uint32_t serverIp;
	char requestString[];
} StrmSrvCmdT;

typedef struct SetdSrvCmdT {
	char  cmd[4];
	uint8_t id;
	char playerName[];
} SetdSrvCmdT;

typedef struct AudeSrvCmdT {
	char  cmd[4];
	uint8_t spdifEnabled;
	uint8_t dacEnabled;
} AudeSrvCmdT;

typedef struct AudgSrvCmdT {
	char  cmd[4];
	uint32_t oldVolLeft;
	uint32_t oldVolRight;
	uint8_t dvcEnabled;
	uint8_t preAmp;
	uint32_t newVolLeft;
	uint32_t newVolRight;
	uint32_t sequence;
} AudgSrvCmdT;

typedef struct RespCliCmdT
{
	CliCmdBaseT cmdBase;
	char responseStr[];
} RespCliCmdT;

typedef struct IRCliCmdT {
	CliCmdBaseT cmdBase;
	uint32_t jiffies;
	uint8_t  format; // ignored by server
	uint8_t  bits;   // ignored by server
	uint32_t ir_code;
} IRCliCmdT;

#pragma pack(pop)

CommandFactory::CommandFactory(LMSConnection *connection, IServerCmdListener *cmdListener) :
		lmsConnection(connection),
		cmdListener(cmdListener)
{
}

CommandFactory::~CommandFactory()
{
}

void CommandFactory::SetCmdBase(CliCmdBaseT *cmdBasePtr, const void *cmd, uint32_t messageSize)
{
	memcpy(cmdBasePtr->cmd,cmd,4);
	cmdBasePtr->messageSize=htonl(messageSize-sizeof(CliCmdBaseT));
}

bool CommandFactory::SendMessage(void *data, ssize_t dataSize)
{
	CliCmdBaseT *cmdBase=(CliCmdBaseT *)data;

	if (!this->lmsConnection->IsConnected())
	{
		Logger::LogError("Requested to send a command without being connected to a server. Skipping it.");
		return false;
	}

	Logger::LogDebug("CommandFactory::SendMessage - Going to send cmd \'%.4s\', Size: %ld",cmdBase->cmd, dataSize);

	return this->lmsConnection->SendCmd(data,dataSize);
}

bool slimprotolib::CommandFactory::SendHeloCmd()
{
	//TODO: configure or read out MAC address
	//TODO: configure or create UID
	//TODO: Thing about wlanChannelList -> readout ??
	//TODO: What's about dataBytesReceived?
	//TODO: Thing about Countrycode -> readout ??


	HeloCliCmdT cmd;
	this->SetCmdBase(&cmd.cmdBase,"HELO", sizeof(HeloCliCmdT));
	cmd.deviceId=12; // 12 -> squeeze player
	cmd.revision=1;
	memcpy(cmd.macAdress, MAC_ADDRESS, sizeof(cmd.macAdress));
	memcpy(cmd.UID, AN_UID, sizeof(cmd.UID));
	cmd.wlanChannelList=0x1FFF;
	cmd.dataBytesReceivedH=0;
	cmd.dataBytesReceivedL=0;
	memcpy(cmd.countryCode, COUNTRY_CODE, sizeof(cmd.countryCode));

	return this->SendMessage(&cmd, sizeof(cmd));
}

bool CommandFactory::SendStatCmd(const char *event,
		const IPlayer::PlayerStatusT *playerStatus, uint32_t serverTimestamp)
{
	StatCliCmdT cmd;
	bool result;
	uint32_t elapsedMsLocal=playerStatus->elapsedMilliseconds;

	memset(&cmd, 0, sizeof(cmd));

	this->SetCmdBase(&cmd.cmdBase,"STAT", sizeof(StatCliCmdT));
	memcpy(cmd.event, event, sizeof(cmd.event));
	//numCrlf,masInitialized,masMode;
	cmd.streamBufferSize=htonl(playerStatus->streamBufferSize);
	cmd.streamBufferFullness=htonl(playerStatus->streamBufferFullness);
	cmd.bytesReceivedH=htonl((uint64_t)playerStatus->bytesReceived >> 32);
	cmd.bytesReceivedL=htonl((uint64_t)playerStatus->bytesReceived & 0xffffffff);
	cmd.signalStrength=0xFFFF;
	cmd.jiffies=htonl(Utils::GetTimeMS());
	cmd.outputBufferSize=htonl(playerStatus->outputBufferSize);
	cmd.outputBufferFullness=htonl(playerStatus->outputBufferFullness);

	//player not really informed about the track change yet on both of these commands
	if (strcmp(event,"STMs")==0 || strcmp(event,"STMc")==0)
		elapsedMsLocal=0;

	cmd.elapsedSeconds=htonl(elapsedMsLocal/1000);
	//voltage;
	cmd.elapsedMilliseconds=htonl(elapsedMsLocal);
	cmd.serverTimestamp=serverTimestamp;
	//errorCode;

	result=this->SendMessage(&cmd, sizeof(cmd));

	Logger::LogDebug("CommandFactory::SendStatCmd - Sub command: %s", event);

	return result;
}

bool CommandFactory::SendByeCmd()
{
	ByeCliCmdT cmd;
	this->SetCmdBase(&cmd.cmdBase,"BYE!", sizeof(ByeCliCmdT));
	cmd.leavingForUpdate=0;
	return this->SendMessage(&cmd, sizeof(cmd));
}

bool CommandFactory::SendPlayerName(const char *playerName)
{
	SetdCliCmdT *cmd;
	int cmdLen;

	cmdLen=sizeof(SetdCliCmdT)+strlen(playerName)+1;
	cmd=(SetdCliCmdT *)alloca(cmdLen);

	this->SetCmdBase(&(cmd->cmdBase),"SETD", cmdLen);
	cmd->id=0;
	strcpy(cmd->name,playerName);

	return this->SendMessage(cmd, cmdLen);
}

bool slimprotolib::CommandFactory::SendConnectResponse(const char *responseStr)
{
	RespCliCmdT *cmd;
	int cmdLen;

	cmdLen=sizeof(RespCliCmdT)+strlen(responseStr)+1;
	cmd=(RespCliCmdT *)alloca(cmdLen);

	this->SetCmdBase(&(cmd->cmdBase),"RESP", cmdLen);
	strcpy(cmd->responseStr,responseStr);

	return this->SendMessage(cmd, cmdLen);
}

bool CommandFactory::SendIRCmd(uint32_t irCode)
{
	IRCliCmdT cmd;
	memset(&cmd, 0, sizeof(cmd));
	this->SetCmdBase(&cmd.cmdBase,"IR  ", sizeof(IRCliCmdT));
	cmd.jiffies=htonl(Utils::GetTimeMS());
	cmd.ir_code=htonl(irCode);
	return this->SendMessage(&cmd, sizeof(cmd));
}


bool CommandFactory::CheckSizeMin(const char *cmd, uint16_t sizeReceived, size_t sizeExpected)
{
	if (sizeReceived>=sizeExpected) return true;
	Logger::LogError("Received corrupt command \'%s\'."
			" Expecting at least %d bytes, received: %ld bytes.", cmd, sizeReceived, sizeExpected);
	return false;
}

bool CommandFactory::CheckSizeExact(const char *cmd, uint16_t sizeReceived, size_t sizeExpected)
{
	if (sizeReceived==sizeExpected) return true;
	Logger::LogError("Received corrupt command \'%s\'."
			" Expecting %d bytes, received: %ld bytes.", cmd, sizeReceived, sizeExpected);
	return false;
}

bool CommandFactory::SendSTMaCmd(
		const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMa", playerStatus, 0);
}

bool CommandFactory::SendSTMcCmd(
		const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMc", playerStatus, 0);
}

bool CommandFactory::SendSTMdCmd(
		const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMd", playerStatus, 0);
}

bool CommandFactory::SendSTMeCmd(
		const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMe", playerStatus, 0);
}

bool CommandFactory::SendSTMfCmd(
		const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMf", playerStatus, 0);
}

bool CommandFactory::SendSTMhCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMh", playerStatus, 0);
}

bool CommandFactory::SendSTMlCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMl", playerStatus, 0);
}

bool CommandFactory::SendSTMnCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMn", playerStatus, 0);
}

bool CommandFactory::SendSTMoCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMo", playerStatus, 0);
}

bool CommandFactory::SendSTMpCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMp", playerStatus, 0);
}

bool CommandFactory::SendSTMrCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMr", playerStatus, 0);
}

bool CommandFactory::SendSTMsCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMs", playerStatus, 0);
}

bool CommandFactory::SendSTMtCmd(const IPlayer::PlayerStatusT *playerStatus,
		uint32_t serverTimestamp)
{
	return this->SendStatCmd("STMt", playerStatus, serverTimestamp);
}

bool CommandFactory::SendSTMuCmd(const IPlayer::PlayerStatusT *playerStatus)
{
	return this->SendStatCmd("STMu", playerStatus, 0);
}

void CommandFactory::DoProcessReceivedCommand(void *data, uint16_t cmdSize)
{
	if (this->cmdListener==NULL) return;

	if (strncmp((char *)data, "strm",4)==0)
	{
		if (CommandFactory::CheckSizeMin("strm", cmdSize, sizeof(StrmSrvCmdT)))
			this->DoProcessStrmCommand((StrmSrvCmdT *)data, cmdSize);
	}
	else if (strncmp((char *)data, "setd",4)==0)
	{
		if (CommandFactory::CheckSizeMin("setd", cmdSize, sizeof(SetdSrvCmdT)))
		{
			//no data send -> player name requested; else new name send by lms
			if (cmdSize==sizeof(SetdSrvCmdT))
				this->cmdListener->OnSrvRequestedPlayerName();
			else
				this->cmdListener->OnSrvProvidedNewPlayerName(((SetdSrvCmdT *)data)->playerName);
		}
	}
	else if (strncmp((char *)data, "aude",4)==0)
	{
		AudeSrvCmdT *cmd=(AudeSrvCmdT *)data;
		if (CommandFactory::CheckSizeExact("aude", cmdSize, sizeof(AudeSrvCmdT)))
			this->cmdListener->OnSrvRequestedAudioEnabledChange(cmd->spdifEnabled!=0, cmd->dacEnabled!=0);
	}
	else if (strncmp((char *)data, "audg",4)==0)
	{
		AudgSrvCmdT *cmd=(AudgSrvCmdT *)data;
		if (CommandFactory::CheckSizeExact("audg", cmdSize, sizeof(AudgSrvCmdT)))
			this->cmdListener->OnSrvRequestedVolumeChange(ntohl(cmd->newVolLeft), ntohl(cmd->newVolRight), cmd->dvcEnabled!=0);
	}
	else
		Logger::LogInfo("CommandFactory::DoProcessCommand - Received unknown command: %.4s, Size: %d", (char *)data, cmdSize);
}

void CommandFactory::DoProcessStrmCommand(StrmSrvCmdT *cmd, uint16_t cmdSize)
{
	Logger::LogDebug("CommandFactory::OnStrmCommandReceived - Sub command: %c",cmd->subCmd);
    if (cmd->subCmd=='t')
    	// no ntohl on replayGain -> Server expects this value being send back as it was using STMt
    	this->cmdListener->OnSrvRequestedPlayerStatus(cmd->replayGain);
    else if (cmd->subCmd=='s')
    	this->DoProcessStrmSCmd(cmd);
    else if (cmd->subCmd=='p')
    	this->cmdListener->OnSrvRequestedPause(ntohl(cmd->replayGain));
    else if (cmd->subCmd=='q')
    	this->cmdListener->OnSrvRequestedStop();
    else if (cmd->subCmd=='u')
    	this->cmdListener->OnSrvRequestedUnPause(ntohl(cmd->replayGain));
    else if (cmd->subCmd=='a')
    	this->cmdListener->OnSrvRequestedSkippingFrames(ntohl(cmd->replayGain));
    else if (cmd->subCmd=='f')
    	this->cmdListener->OnSrvRequestedFlush();
    else
    {
    	uint32_t serverIp=ntohl(cmd->serverIp);
        Logger::LogDebug("PlayerController::OnStrmCommandReceived - Unknown sub command received.");
        Logger::LogDebug("----------------------------------------------------------------------------");
        Logger::LogDebug("Sub Command: %c", cmd->subCmd);
        Logger::LogDebug("Autostart: %c", cmd->autostart);
        Logger::LogDebug("Format: %c", cmd->format);
        Logger::LogDebug("PCM Sample Size: %c", cmd->pcmSampleSize);
        Logger::LogDebug("PCM Sample Rate: %c", cmd->pcmSampleRate);
        Logger::LogDebug("PCM Channels: %c", cmd->pcmChannels);
        Logger::LogDebug("PCM Endianess: %c", cmd->pcmEndianness);
        Logger::LogDebug("Threshold: %d", cmd->threshold);
        Logger::LogDebug("SPDIFENABLE: %d", cmd->spdifEnable);
        Logger::LogDebug("Trans Period: %d", cmd->transitionPeriod);
        Logger::LogDebug("Trans Type: %c", cmd->transitionType);
        Logger::LogDebug("Flags: 0x%X", cmd->flags);
        Logger::LogDebug("Out Threshold: %d", cmd->threshold);
        Logger::LogDebug("ReplayGain: %d.%d", (cmd->replayGain>>16),(cmd->replayGain & 0xFFFF));
        Logger::LogDebug("Server Port: %d", ntohs(cmd->serverPort));
        Logger::LogDebug("Server IP: %d.%d.%d.%d",
        		serverIp & 0xFF, (serverIp >> 8) & 0xFF, (serverIp >> 16) & 0xFF, (serverIp >> 24));

        if (cmdSize > sizeof(StrmSrvCmdT))
            Logger::LogDebug("HTTP request: %s",cmd->requestString);
        Logger::LogDebug("----------------------------------------------------------------------------");
    }
}

void CommandFactory::DoProcessStrmSCmd(StrmSrvCmdT *cmd)
{
	IPlayer::StreamingServerInfoT srvInfo;
	IPlayer::AudioFormatT audioFMT;

	bool autostart;

	if (cmd->serverIp!=0)
		srvInfo.ip=cmd->serverIp;
	else
	srvInfo.ip=this->lmsConnection->GetServerIp();
	srvInfo.port=ntohs(cmd->serverPort);
	srvInfo.httpRequest=cmd->requestString;

	audioFMT.streamFormat=(IPlayer::StreamFormatT)cmd->format;
	audioFMT.pcmSampleSize=(IPlayer::PCMSampleSizeT)cmd->pcmSampleSize;
	audioFMT.pcmSampleRate=(IPlayer::PCMSampleRateT)cmd->pcmSampleRate;
	audioFMT.pcmChannelCount=(IPlayer::PCMChannelCountT)cmd->pcmChannels;
	audioFMT.pcmEndian=(IPlayer::PCMEndianT)cmd->pcmEndianness;

	this->cmdListener->OnSrvRequestedLoadStream(&srvInfo,&audioFMT,autostart);
}
