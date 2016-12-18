#include "PageComponent.h"

#include "Page.h"

PageComponent::PageComponent( ) { }


PageComponent::~PageComponent( ) { }

void PageComponent::reposition( ) {
	pos.x = page->dim.x * anchor.x + offset.x;
	pos.y = page->dim.y * anchor.y + offset.y;
}