#include "tcpclient.h"

TcpClient::TcpClient(QObject* parent) : Client(parent) {
	qDebug() << "";
}

TcpClient::~TcpClient() {
	qDebug() << "";
	close();
}

bool TcpClient::doOpen() {
	sock_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sock_ == -1) {
		SET_ERR(GErr::Fail, QString("socket return -1 %1").arg(strerror(errno)));
		return false;
	}

	struct sockaddr_in addr;

	if (localIp_ != 0 || localPort_ != 0) {
		addr.sin_family = AF_INET;
		addr.sin_port = htons(quint16(localPort_));
		addr.sin_addr.s_addr = htonl(localIp_);
		memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
		int res = ::bind(sock_, (struct sockaddr *)&addr, sizeof(addr));
		if (res == -1) {
			SET_ERR(GErr::Fail, QString("bind(%1, %2) return -1 %1").arg(QString(localIp_)).arg(localPort_).arg(strerror(errno)));
			return false;
		}
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_);
	addr.sin_addr.s_addr = htonl(ip_);
	memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = ::connect(sock_, (struct sockaddr *)&addr, sizeof(addr));
	if (res == -1) {
		SET_ERR(GErr::Fail, QString("connect return -1 %1").arg(strerror(errno)));
		return false;
	}

	return true;
}

bool TcpClient::doClose() {
	TcpSession::disconnect();
	return true;
}
