#include "tlsclient.h"
#include "tlscommon.h"

TlsClient::TlsClient(QObject* parent) : Client(parent) {
	TlsCommon::initialize();
}

TlsClient::~TlsClient() {
	close();
}

bool TlsClient::doOpen() {
	tcpClient_.ip_ = ip_;
	tcpClient_.port_ = port_;
	if (!tcpClient_.open()) {
		err = tcpClient_.err;
		return false;
	}

	const SSL_METHOD *method = TLS_client_method();
	if (method == nullptr) {
		SET_ERR(GErr::Fail, "TLS_client_method return null");
		return false;
	}

	ctx_ = SSL_CTX_new(method);
	if (ctx_ == nullptr) {
		SET_ERR(GErr::Fail, "SSL_CTX_new return null");
		return false;
	}

	sock_ = tcpClient_.sock_;
	ssl_ = SSL_new(ctx_);
	if (ssl_ == nullptr) {
		SET_ERR(GErr::Fail, "SSL_new return null");
		return false;
	}

	int res = SSL_set_fd(ssl_, sock_);
	if (res != 1) {
		SET_ERR(GErr::Fail, "SSL_set_fd return null");
		return false;
	}

	res = SSL_connect(ssl_);
	if (res <= 0) {
		int sslError = SSL_get_error(ssl_, res);
		SET_ERR(GErr::Fail, QString("SSL_connect return %1 SSL_get_error=%2").arg(res, sslError));
		return false;
	}

	return true;
}

bool TlsClient::doClose() {
	if (ssl_ != nullptr) {
		SSL_shutdown(ssl_);
		SSL_free(ssl_);
		ssl_ = nullptr;
	}

	if (ctx_ != nullptr) {
		SSL_CTX_free(ctx_);
		ctx_ = nullptr;
	}

	tcpClient_.close();

	return true;
}
