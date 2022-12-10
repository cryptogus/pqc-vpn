#include <csignal>
#include <iostream>
#include <thread>

#include <GApp>

#include "vpnclient.h"

struct Param {
	QString dummyIntfName_;
	QString realIntfName_;
	GIp ip_;
	int port_;

	bool parse(int argc, char** argv) {
		if (argc != 5) return false;
		dummyIntfName_ = argv[1];
		realIntfName_ = argv[2];
		ip_ = GIp(argv[3]);
		port_ = std::stoi(argv[4]);
		return true;
	}

	static void usage() {
		printf("syntax : vpnclient-test <dummy interface> <real interface> <ip> <port>\n");
		printf("sample : vpnclient-test dum0 wlan0 127.0.0.1 12345\n");
	}
};

VpnClient vc;
struct Obj : QObject {
	Q_OBJECT

public:
	Obj() : QObject(nullptr) {
		QMetaObject::Connection conn = QObject::connect(&vc, &GStateObj::closed, this, &Obj::processClosed);
		qDebug() << conn;
	}

public slots:
	void processClosed() {
		qDebug() << "bef vc.close()";
		vc.close();
		qDebug() << "aft vc.close()";
	}
} obj;

void signalHandler(int signo) {
	qWarning() << QString("signal occured %1").arg(signo);
	vc.close();
	qWarning() << "vpnclient terminated successfully";
	exit(0);
}

int main(int argc, char* argv[]) {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	GApp a(argc, argv);

	Param param;
	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}

	vc.dummyIntfName_ = param.dummyIntfName_;
	vc.realIntfName_ = param.realIntfName_;
	vc.ip_ = param.ip_;
	vc.port_ = param.port_;
	if (!vc.open()) {
		std::cerr << qPrintable(vc.err->msg()) << std::endl;
		return -1;
	}

	while (vc.active())
		QThread::sleep(1);

	vc.close();
}

#include "vpnclient-test.moc"
