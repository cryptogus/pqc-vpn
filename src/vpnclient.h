#pragma once

#include <GSyncPcapDevice>
#include <GRawIpSocketWrite>
#include "tcpclient.h"

struct VpnClient : GStateObj {
	Q_OBJECT

public:
	static const int MaxBufSize = 16384;

	VpnClient(QObject* parent = nullptr);
	~VpnClient() override;

	TcpClient tcpClient_{this};
	GSyncPcapDevice dummyPcapDevice_{this};
	GRawIpSocketWrite socketWrite_{this};

protected:
	bool doOpen() override;
	bool doClose() override;

	struct CaptureAndSendThread : GThread {
		CaptureAndSendThread(QObject* parent) : GThread(parent) {}
		~CaptureAndSendThread() override {}

		void run() override;
	} captureAndSendThread_{this};

	struct ReadAndReplyThread : GThread {
		ReadAndReplyThread(QObject* parent) : GThread(parent) {}
		~ReadAndReplyThread() override {}

		void run() override;
	} readAndReplyThread_{this};
};
typedef VpnClient *PVpnClient;
