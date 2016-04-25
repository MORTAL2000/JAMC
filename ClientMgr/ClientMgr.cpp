#include "ClientMgr.h"

ClientMgr::ClientMgr(){}

ClientMgr::~ClientMgr(){}

void ClientMgr::run() {
	client.init();
	client.end();
}