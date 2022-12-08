#include "vpnclient.h"

VpnClient::VpnClient(QObject* parent) : GStateObj(parent) {
}

VpnClient::~VpnClient() {
	close();
}

bool VpnClient::doOpen() {
	GIntf* intf = GNetInfo::instance().intfList().findByName(realIntfName_);
	if (intf == nullptr) {
		SET_ERR(GErr::ObjectIsNull, QString("intf(%1)null").arg(realIntfName_));
		return false;
	}
	GIp ip = intf->ip();
	if (ip == 0) {
		SET_ERR(GErr::ValueIsNotZero, QString("ip is zero(%1)").arg(realIntfName_));
		return false;
	}
	tcpClient_.localIp_ = ip;
	tcpClient_.localPort_ = 0;
	tcpClient_.ip_ = ip_;
	tcpClient_.port_ = port_;
	if (!tcpClient_.open()) {
		err = tcpClient_.err;
		return false;
	}

	dummyPcapDevice_.intfName_ = dummyIntfName_;
	if (!dummyPcapDevice_.open()) {
		err = dummyPcapDevice_.err;
		return false;
	}

	if (!socketWrite_.open()) {
		err = socketWrite_.err;
		return false;
	}

	captureAndSendThread_.start();
	readAndReplyThread_.start();

	return true;
}

bool VpnClient::doClose() {
	tcpClient_.close();
	dummyPcapDevice_.close();
	socketWrite_.close();

	captureAndSendThread_.quit();
	captureAndSendThread_.wait();
	readAndReplyThread_.quit();
	readAndReplyThread_.wait();

	return true;
}

void VpnClient::CaptureAndSendThread::run() {
	qDebug() << ""; // gilgil temp 2022.12.07
	VpnClient* client = PVpnClient(parent());
	GSyncPcapDevice* dummyPcapDevice = &client->dummyPcapDevice_;
	TcpClient* tcpClient = &client->tcpClient_;

	while (client->active()) {
		GEthPacket packet;
		GPacket::Result res = dummyPcapDevice->read(&packet);
		if (res == GPacket::Eof || res == GPacket::Fail) break;
		if (res == GPacket::None) continue;

		//QThread::sleep(1); // gilgil temp 2022.12.08

		GIpHdr* ipHdr = packet.ipHdr_;
		if (ipHdr == nullptr) continue;

		GTcpHdr* tcpHdr = packet.tcpHdr_;
		if (tcpHdr != nullptr)
			tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		GUdpHdr* udpHdr = packet.udpHdr_;
		if (udpHdr != nullptr)
			udpHdr->sum_ = htons(GUdpHdr::calcChecksum(ipHdr, udpHdr));

		uint16_t len = sizeof(GEthHdr) + ipHdr->len();
		char buf[MaxBufSize];
		memcpy(buf, "PQ", 2);
		*reinterpret_cast<uint16_t*>(&buf[2]) = htons(len);
		memcpy(&buf[4], packet.buf_.data_, len);

		int writeLen = tcpClient->write(buf, 4 + len);
		if (writeLen == -1) break;
		qWarning() << QString("session write %1").arg(4 + len);
	}
	qDebug() << ""; // gilgil temp 2022.12.07
}

void VpnClient::ReadAndReplyThread::run() {
	qDebug() << ""; // gilgil temp 2022.12.07
	VpnClient* client = PVpnClient(parent());
	TcpClient* tcpClient = &client->tcpClient_;
	GRawIpSocketWrite* socketWrite = &client->socketWrite_;

	while (client->active()) {
		char buf[MaxBufSize];
		int readLen = tcpClient->readAll(buf, 4); // header size
		if (readLen != 4) break;
		if (buf[0] != 'P' || buf[1] != 'Q') {
			qWarning() << QString("invalid header %1 %2").arg(uint32_t(buf[0])).arg(uint32_t(buf[1]));
			break;
		}
		uint16_t len = ntohs(*reinterpret_cast<uint16_t*>(&buf[2]));
		qDebug() << "len=" << len; // gilgil temp 2022.12.08
		if (len > 10000) {
			qWarning() << "too big len" << len;
		}
		readLen = tcpClient->readAll(buf, len);
		if (readLen != len) {
			qWarning() << QString("readLen=%1 len=%2").arg(readLen).arg(len);
			break;
		}

		//QThread::sleep(1); // gilgil temp 2022.12.08

		GEthPacket packet;
		packet.buf_.data_ = pbyte(buf);
		packet.buf_.size_ = len;
		packet.parse();

		GPacket::Result res;
		GTcpHdr* tcpHdr = packet.tcpHdr_;
		if (tcpHdr != nullptr)
			res = socketWrite->write(&packet);
		else
			res = client->dummyPcapDevice_.write(&packet);

		//GPacket::Result
		if (res != GPacket::Ok) {
			qWarning() << QString("pcapDevice_.write(&packet) return %1").arg(int(res));
		}
		qWarning() << QString("pcap write %1").arg(packet.buf_.size_);
	}
	qDebug() << ""; // gilgil temp 2022.12.07
}
