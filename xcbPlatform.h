// XCB linux platform dependent WSI handling and event loop

#ifndef COMMON_XCB_WSI_H
#define COMMON_XCB_WSI_H

#include <functional>
#include <string>
#include <cstring>

#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <vulkan/vulkan.h>

#include "CompilerMessages.h"


TODO( "Easier to use, but might prevent platform co-existence. Could be namespaced. Make all of this a class?" )
struct PlatformWindow{ xcb_connection_t* connection; xcb_window_t window; xcb_visualid_t visual_id; };

std::string getPlatformSurfaceExtensionName(){ return VK_KHR_XCB_SURFACE_EXTENSION_NAME; };

int messageLoop( PlatformWindow window );

bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow window );

PlatformWindow initWindow( int canvasWidth, int canvasHeight );
void killWindow( PlatformWindow window );

VkSurfaceKHR initSurface( VkInstance instance, PlatformWindow window );
// killSurface() is not platform dependent

void setSizeEventHandler( std::function<void(void)> newSizeEventHandler );
void setPaintEventHandler( std::function<void(void)> newPaintEventHandler );

void showWindow( PlatformWindow window );

// Implementation
//////////////////////////////////

void nullHandler(){}

std::function<void(void)> sizeEventHandler = nullHandler;

void setSizeEventHandler( std::function<void(void)> newSizeEventHandler ){
	if( !newSizeEventHandler ) sizeEventHandler = nullHandler;
	sizeEventHandler = newSizeEventHandler;
}

std::function<void(void)> paintEventHandler = nullHandler;

void setPaintEventHandler( std::function<void(void)> newPaintEventHandler ){
	if( !newPaintEventHandler ) paintEventHandler = nullHandler;
	paintEventHandler = newPaintEventHandler;
}

void showWindow( PlatformWindow window ){
	xcb_map_window( window.connection, window.window );
	xcb_flush( window.connection );
}

int width = -1;
int height = -1;
bool windowReady = false;

int messageLoop( PlatformWindow window ){
	bool quit = false;

	while( !quit ){
		xcb_generic_event_t* e = xcb_poll_for_event( window.connection );

		if( e ){
			switch( e->response_type & ~0x80 ){
				case XCB_EXPOSE:
					paintEventHandler();
					break;

				case XCB_CONFIGURE_NOTIFY:{
					xcb_configure_notify_event_t* ce = (xcb_configure_notify_event_t*)e;
					if( ce->width != width || ce->height != height ){
						width = ce->width;
						height = ce->height;

						sizeEventHandler();

						windowReady = true;
					}

					break;
				}

				case XCB_KEY_PRESS:{
					xcb_key_press_event_t* kpe = (xcb_key_release_event_t*)e;

					xcb_key_symbols_t* keysyms = xcb_key_symbols_alloc( window.connection );

					switch(  xcb_key_press_lookup_keysym( keysyms, kpe, 0 )  ){
						case XK_Escape:
							quit = true;
					}

					xcb_key_symbols_free( keysyms );

					/*
					switch( kpe->detail ){
						case 9: // ESC
							quit = true;
					}
					*/
					break;
				}

				case XCB_CLIENT_MESSAGE:{
					xcb_client_message_event_t* cme = (xcb_client_message_event_t*)e;

					xcb_intern_atom_cookie_t wmdelCookie = xcb_intern_atom( window.connection, 1, 16, "WM_DELETE_WINDOW" );
					xcb_intern_atom_reply_t* wmdelReply = xcb_intern_atom_reply( window.connection, wmdelCookie, 0 );
					xcb_atom_t ATOM_WM_DELETE_WINDOW = wmdelReply->atom;
					free( wmdelReply );

					if( cme->data.data32[0] == ATOM_WM_DELETE_WINDOW ){
						quit = true;
					}
					break;
				}

				//default:
				//	throw "Unrecognized event type!";
			}

			free( e );
		}
		else if( windowReady ){ // no events pending
			paintEventHandler();
		}
	}

	return 0;
}


bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow window ){
	return vkGetPhysicalDeviceXcbPresentationSupportKHR( device, queueFamilyIndex, window.connection, window.visual_id ) == VK_TRUE;
}

std::string to_string_xcb_conn( int error ){
	switch( error ){
		case 0: return "XCB_CONN_SUCCESS";
		case XCB_CONN_ERROR: return "XCB_CONN_ERROR";
		case XCB_CONN_CLOSED_EXT_NOTSUPPORTED: return "XCB_CONN_CLOSED_EXT_NOTSUPPORTED";
		case XCB_CONN_CLOSED_MEM_INSUFFICIENT: return "XCB_CONN_CLOSED_MEM_INSUFFICIENT";
		case XCB_CONN_CLOSED_REQ_LEN_EXCEED: return "XCB_CONN_CLOSED_REQ_LEN_EXCEED";
		case XCB_CONN_CLOSED_PARSE_ERR: return "XCB_CONN_CLOSED_PARSE_ERR";
		case XCB_CONN_CLOSED_INVALID_SCREEN: return "XCB_CONN_CLOSED_INVALID_SCREEN";
		default: return "unrecognized XCB connection error";
	}
}

xcb_connection_t* initXcbConnection(){
	xcb_connection_t* connection = xcb_connect( nullptr, nullptr );

	int err = xcb_connection_has_error( connection );
	if( err ){
		xcb_disconnect( connection );
		throw std::string( "Failed to create XCB connection: " ) + to_string_xcb_conn( err );
	}

	return connection;
}

void killXcbConnection( xcb_connection_t* connection ){
	xcb_disconnect( connection );
}

PlatformWindow initWindow( int canvasWidth, int canvasHeight ){
	xcb_connection_t* connection = initXcbConnection();

	xcb_screen_t* screen = xcb_setup_roots_iterator(  xcb_get_setup( connection )  ).data;

	uint32_t masks =
		/*  XCB_CW_BACK_PIXMAP
		| XCB_CW_BACK_PIXEL
		| XCB_CW_BORDER_PIXMAP
		| XCB_CW_BORDER_PIXEL
		| XCB_CW_BIT_GRAVITY
		| XCB_CW_WIN_GRAVITY
		| XCB_CW_BACKING_STORE
		| XCB_CW_BACKING_PLANES
		| XCB_CW_BACKING_PIXEL
		| XCB_CW_OVERRIDE_REDIRECT
		| XCB_CW_SAVE_UNDER
		|*/ XCB_CW_EVENT_MASK
		/*| XCB_CW_DONT_PROPAGATE
		| XCB_CW_COLORMAP
		| XCB_CW_CURSOR*/
	;

	uint32_t values[] = {
		XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY
	};


	xcb_window_t window = xcb_generate_id( connection );
	xcb_create_window(
		connection,
		XCB_COPY_FROM_PARENT,
		window,
		screen->root,
		0, 0, // x, y
		static_cast<uint16_t>( canvasWidth ), static_cast<uint16_t>( canvasHeight ),
		1, //border_width
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		screen->root_visual,
		masks,
		values
	); 

	const char* title = "Hello Vulkan Triangle";

  	xcb_change_property(  connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen( title ), title  );
  	xcb_change_property(  connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8, strlen( title ), title  );


	xcb_intern_atom_cookie_t wmprotCookie = xcb_intern_atom( connection, 1, 12, "WM_PROTOCOLS" );
	xcb_intern_atom_reply_t* wmprotReply = xcb_intern_atom_reply( connection, wmprotCookie, 0 );
	xcb_atom_t ATOM_WM_PROTOCOLS = wmprotReply->atom;
	free( wmprotReply );

	xcb_intern_atom_cookie_t wmdelCookie = xcb_intern_atom( connection, 0, 16, "WM_DELETE_WINDOW" );
	xcb_intern_atom_reply_t* wmdelReply = xcb_intern_atom_reply( connection, wmdelCookie, 0 );
	xcb_atom_t ATOM_WM_DELETE_WINDOW = wmdelReply->atom;
	free( wmdelReply );

	// undocumented magic from random mailing list everybody seems to use
	xcb_change_property( connection, XCB_PROP_MODE_REPLACE, window, ATOM_WM_PROTOCOLS, XCB_ATOM_ATOM, 32, 1, &ATOM_WM_DELETE_WINDOW );

	xcb_flush( connection );
	return { connection, window, screen->root_visual };
}

void killWindow( PlatformWindow window ){
	xcb_destroy_window( window.connection, window.window );
	xcb_flush( window.connection );

	killXcbConnection( window.connection );
}

VkSurfaceKHR initSurface( VkInstance instance, PlatformWindow window ){
	VkXcbSurfaceCreateInfoKHR surfaceInfo{
		VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		window.connection,
		window.window
	};


	VkSurfaceKHR surface;
	VkResult errorCode = vkCreateXcbSurfaceKHR( instance, &surfaceInfo, nullptr, &surface ); RESULT_HANDLER( errorCode, "vkCreateXcbSurfaceKHR" );

	return surface;
}


#endif //COMMON_XCB_WSI_H
