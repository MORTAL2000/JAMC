#pragma once

class Client;

class Manager {
public:
	Manager( );

	Client & client;

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void render() = 0;
	virtual void end() = 0;
	virtual void sec() = 0;

};