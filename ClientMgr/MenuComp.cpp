#include "MenuComp.h"

#include "ImageComp.h"
#include "LabelComp.h"
#include "ClickableComp.h"
#include "OverableComp.h"

void MenuComp::MenuData::add_entry( Client & client, std::string const & label_text, PCFunc func_up ) {
	auto image = comp->add_comp(std::string( "Image" ) + std::to_string( id_entry++ ), "Image", PageComponentLoader::func_null );
	image->anchor = { 0.0f, 0.0f };
	auto image_data = image->get_data< ImageComp::ImageData >( );
	image_data->set_texture( client, "Materials", "Details/Solid" );
	image_data->color = color_default;

	auto label = image->add_comp( "Label", "Label", func_null );
	label->anchor = { 0.5f, 0.5f };
	auto label_data = label->get_data< LabelComp::LabelData >( );
	label_data->text = label_text;
	label_data->size_text = 12;
	label_data->alignment_h = label_data->AH_Center;
	label_data->alignment_v = label_data->AV_Center;

	auto clickable = image->add_comp( "Clickable", "Clickable", func_null );
	auto clickable_data = clickable->get_data< ClickableComp::ClickableData >( );

	clickable_data->func_down = [ &client = client, image_data, this ] ( PComp * comp ) {
		image_data->color = color_down;
		comp->page->is_remesh = true;

		return 0;
	};

	clickable_data->func_up = [ &client = client, func_up, image_data, this ] ( PComp * comp ) {
		if( Directional::is_point_in_rect(
			client.input_mgr.get_mouse( ),
			comp->page->pos + comp->pos,
			comp->page->pos + comp->pos + comp->dim ) ) {
		
			image_data->color = color_over;
			comp->page->is_remesh = true;

			func_up( comp );
		}
		else { 
			image_data->color = color_default;
			comp->page->is_remesh = true;
		}

		return 0;
	};

	auto overable = image->add_comp( "Overable", "Overable", func_null );
	auto overable_data = overable->get_data< OverableComp::OverableData >( );
	overable_data->func_enter = [ &client = client, image_data, this ] ( PComp * comp ) {
		if( !( image_data->color == color_down ) ) { 
			image_data->color = color_over;
			comp->page->is_remesh = true;
		}


		return 0;
	};

	overable_data->func_exit = [ &client = client, image_data, this ] ( PComp * comp ) {
		if( !( image_data->color == color_down ) ) {
			image_data->color = color_default;
			comp->page->is_remesh = true;
		}

		return 0;
	};

	list_entries.push_back( image );

	reposition( );
}

void MenuComp::MenuData::reposition( ) { 
	comp->dim = { dim_entry.x + padding_entry.x * 2 + padding_menu.x * 2 , 
		( dim_entry.y + padding_entry.y * 2 ) * list_entries.size( ) + padding_menu.y * 2 };

	comp_border->dim = comp->dim;
	comp_border->offset = -comp_border->dim / 2;

	for( int i = 0; i < list_entries.size( ); ++i ) {
		list_entries[ i ]->dim = dim_entry;
		list_entries[ i ]->anchor = { 0.0f, 0.0f };
		list_entries[ i ]->offset = {
			padding_menu.x + padding_entry.x,
			padding_menu.y + padding_entry.y + ( dim_entry.y + padding_entry.y * 2 ) * i };
	}
}

MenuComp::MenuComp( Client & client ) { 
	name = "Menu";

	func_register = [ &client = client ] () {
		if( !client.resource_mgr.reg_pool< MenuData >( 128 ) ) { 

			return 1;
		}

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		auto data = comp->add_data< MenuData >( );
		data->comp = comp;
		data->id_entry = 0;

		data->list_entries.clear( );

		data->dim_entry = { 64, 20 };
		data->padding_entry = { 2, 2 };
		data->padding_menu = { 2, 2 };

		data->color_default = { 0.0f, 0.0f, 0.0f, 0.0f };
		data->color_over = { 0.0f, 0.0f, 0.0f, 0.3f };
		data->color_down = { 0.7f, 0.0f, 0.0f, 0.3f };

		data->comp_border = comp->add_comp( "Border", "BorderImage", func_null );
		data->data_border = data->comp_border->get_data< BorderImageComp::BorderImageData >( );
		data->data_border->padding_border = 4;

		data->reposition( );

		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) {
		if( client.input_mgr.is_mouse_up( 0 ) || client.input_mgr.is_mouse_up( 1 ) ) { 
			//if( !Directional::is_point_in_rect(
			//	client.input_mgr.get_mouse( ),
			//	comp->page->pos + comp->pos,
			//	comp->page->pos + comp->pos + comp->dim ) ) {
			
				comp->is_visible = false;
			//}
		}

		return 0;
	};
}

MenuComp::~MenuComp( ) { }
