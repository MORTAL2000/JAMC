#pragma once
#include "Globals.h"

#include "Client.h"

class ClientMgr{
private:
	Client client;

public:
	ClientMgr();
	~ClientMgr();

	void run();

};

