#pragma once
#include "Globals.h"

class Manager {
protected:
	Client & client;

public:
	Manager( Client & client ) : 
		client( client ) {}

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void render() = 0;
	virtual void end() = 0;
	virtual void sec() = 0;

};