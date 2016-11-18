#include "ClientMgr.h"

#include "BlockSet.h"

ClientMgr::ClientMgr(){}

ClientMgr::~ClientMgr(){}

void ClientMgr::run() {
	client.init();
	client.end();
}