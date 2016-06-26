#include "Entity.h"
#include "Client.h"

EntityLoader::EntityLoader( std::string const & name,
	EFAlloc ef_alloc, EFRelease ef_release,
	EFUpdate ef_update, EFMesh ef_mesh ) :
	name( name ),
	ef_alloc( ef_alloc ),
	ef_release( ef_release ),
	ef_update( ef_update ),
	ef_mesh( ef_mesh ) { }

Entity::Entity( ) { }
Entity::~Entity( ) { }