#include "Globals.h"

#include "ClientMgr.h"

#include "glm\glm.hpp"
#include "tinyxml2-master\tinyxml2.h"

#include "SharedMesh.h"

int main() {
	ClientMgr client_mgr;	
	client_mgr.run();

	return 0;
}