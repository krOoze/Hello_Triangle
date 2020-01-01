// Wayland platform dependent WSI handling and event loop

#ifndef COMMON_WAYLAND_WSI_H
#define COMMON_WAYLAND_WSI_H

#include <functional>
#include <cstring>
#include <string>
#include <vector>
#include <memory>

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <linux/input-event-codes.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#include "private/xdg-shell-client-protocol.h"
#include "private/xdg-shell-client-protocol-private.inl"
#include "CompilerMessages.h"
#include "ErrorHandling.h"


TODO( "Easier to use, but might prevent platform co-existence. Could be namespaced. Make all of this a class?" )
struct PlatformWindowImpl{
	wl_display* display = nullptr;
	wl_registry* registry = nullptr;
	wl_compositor* compositor = nullptr; uint32_t compositorName;
	xdg_wm_base* wmBase = nullptr; uint32_t wmBaseName;
	wl_surface* surface = nullptr;
	xdg_surface* xdgSurface = nullptr;
	xdg_toplevel* toplevel = nullptr;

	wl_seat* seat = nullptr; uint32_t seatName;
	xkb_context* xkbContext = nullptr;
	xkb_keymap* keymap = nullptr;
	xkb_state* xkbState = nullptr;
	wl_keyboard* keyboard = nullptr;
	bool alt = false;
	wl_pointer* pointer = nullptr;

	bool quit = false;
	std::string title;
	bool inited = false;
	bool hasSwapchain = false;

	bool maximized = false, fullscreen = false;
	uint32_t width, height;
	uint32_t restoredWidth, restoredHeight;
};

struct PlatformWindow{
	std::shared_ptr<PlatformWindowImpl> impl;
};

std::string getPlatformSurfaceExtensionName(){ return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME; };

int messageLoop( PlatformWindow window );

bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow window );

PlatformWindow initWindow( const std::string& name, uint32_t canvasWidth, uint32_t canvasHeight );
void killWindow( PlatformWindow window );

VkSurfaceKHR initSurface( VkInstance instance, PlatformWindow window );
// killSurface() is not platform dependent

void setSizeEventHandler( std::function<void(void)> newSizeEventHandler );
void setPaintEventHandler( std::function<void(void)> newPaintEventHandler );

void showWindow( PlatformWindow window );

uint32_t getWindowWidth( PlatformWindow window ){ return window.impl->width; }
uint32_t getWindowHeight( PlatformWindow window ){ return window.impl->height; }

// Implementation
//////////////////////////////////

bool nullHandler(){ return false; }

std::function<bool(void)> sizeEventHandler = nullHandler;

void setSizeEventHandler( std::function<bool(void)> newSizeEventHandler ){
	if( !newSizeEventHandler ) sizeEventHandler = nullHandler;
	sizeEventHandler = newSizeEventHandler;
}

std::function<void(void)> paintEventHandler = nullHandler;

void setPaintEventHandler( std::function<void(void)> newPaintEventHandler ){
	if( !newPaintEventHandler ) paintEventHandler = nullHandler;
	paintEventHandler = newPaintEventHandler;
}

bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow window ){
	return vkGetPhysicalDeviceWaylandPresentationSupportKHR( device, queueFamilyIndex, window.impl->display ) == VK_TRUE;
}

void registryGlobalHandler( void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version ) noexcept{
	//logger << "registry: " << interface << std::endl;
	PlatformWindowImpl* window = (PlatformWindowImpl*)data;

	if(  std::strcmp( interface, "wl_compositor" ) == 0 && !window->compositor  ){
		window->compositor = (wl_compositor*)wl_registry_bind( registry, name, &wl_compositor_interface, 1 );
		window->compositorName = name;
	}
	else if(  std::strcmp( interface, "xdg_wm_base" ) == 0 && !window->wmBase  ){
		window->wmBase = (xdg_wm_base*)wl_registry_bind( registry, name, &xdg_wm_base_interface, 1 );
		window->wmBaseName = name;
	}
	else if(  std::strcmp( interface, "wl_seat" ) == 0 && !window->seat ){
		window->seat = (wl_seat*)wl_registry_bind( registry, name, &wl_seat_interface, 1 );
		window->seatName = name;
	}

	TODO( "Add Wayland server side decorations when supported." );
}

void registryGlobalRemoveHandler( void* data, wl_registry* registry, uint32_t name ) noexcept{
	//logger << "registry remove " << name << std::endl;
	PlatformWindowImpl* window = (PlatformWindowImpl*)data;

	if( name == window->compositorName ) throw "Needed wl_compositor removed from registry.";
	if( name == window->wmBaseName ) throw "Needed xdg_wm_base removed from registry.";
	if( name == window->seatName ) throw "Needed wl_seat removed from registry.";
}

void seatCapsHandler( void* data, wl_seat* seat, uint32_t caps ) noexcept{
	//logger << "wl_seat caps: " << caps << std::endl;

	RUNTIME_ASSERT( caps & WL_SEAT_CAPABILITY_POINTER, "Obtaining pointer wl_seat" );
	RUNTIME_ASSERT( caps & WL_SEAT_CAPABILITY_KEYBOARD, "Obtaining keyboard wl_seat" );
}

void keyboardKeymapHandler( void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size ) noexcept{
	//logger << "wl_keyboard format: " << format << ", keymap size: " << size << std::endl;

	RUNTIME_ASSERT( format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, "Obtaining XKB V1 keymap" );

	PlatformWindowImpl* wnd = (PlatformWindowImpl*)data;
	assert( wnd->xkbContext );

	xkb_state_unref( wnd->xkbState );
	xkb_keymap_unref( wnd->keymap );

	const char* keymapBuffer = (const char*)mmap( nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0 ); RUNTIME_ASSERT( keymapBuffer != MAP_FAILED, "mmap of keymap")
	wnd->keymap = xkb_keymap_new_from_string( wnd->xkbContext, keymapBuffer, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS ); RUNTIME_ASSERT( wnd->keymap, "xkb_keymap_new_from_string" );
	{const auto err = munmap( (void*)keymapBuffer, size ); RUNTIME_ASSERT( !err, "munmap of keymap" );}
	{const auto err = close( fd ); RUNTIME_ASSERT( !err, "close of keymap fd" );}
	wnd->xkbState  = xkb_state_new( wnd->keymap );
}

void keyboardKeyHandler( void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state ) noexcept{
	//logger << "wl_keyboard key: key=" << key << ", state=" << state << std::endl;

	PlatformWindowImpl* wnd = (PlatformWindowImpl*)data;

	const xkb_keycode_t keycode = key + 8; // +8 per Wayland spec
	const xkb_keysym_t keysym = xkb_state_key_get_one_sym( wnd->xkbState, keycode );

	if( state == WL_KEYBOARD_KEY_STATE_PRESSED ){
		switch( keysym ){
			case XKB_KEY_Escape:
				wnd->quit = true;
				break;
			case XKB_KEY_Alt_L:
				wnd->alt = true;
				break;
			case XKB_KEY_Return:
				if( wnd->alt ){
					if( wnd->fullscreen ) xdg_toplevel_unset_fullscreen( wnd->toplevel );
					else xdg_toplevel_set_fullscreen( wnd->toplevel, nullptr );
				}
				break;
		}
	}
	else if( state == WL_KEYBOARD_KEY_STATE_RELEASED ){
		switch( keysym ){
			case XKB_KEY_Alt_L:
				wnd->alt = false;
				break;
		}
	}
}

void pointerButtonHandler( void* data, wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state ){
	PlatformWindowImpl* wnd = (PlatformWindowImpl*)data;

	if( state == WL_POINTER_BUTTON_STATE_PRESSED ){
		switch( button ){
			case BTN_LEFT:
				xdg_toplevel_move( wnd->toplevel, wnd->seat, serial );
				break;
			case BTN_RIGHT:
				xdg_toplevel_resize( wnd->toplevel, wnd->seat, serial, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT );
				break;
		}
	}
}

void xdgSurfaceConfigureHandler( void* data, xdg_surface* xdg_surface, uint32_t serial ) noexcept{
	//logger << "xdg_surface config" << std::endl;
	PlatformWindowImpl* window = (PlatformWindowImpl*)data;
	assert( window->inited );

	xdg_surface_ack_configure( xdg_surface, serial );
}

const char* state_to_string( uint32_t state ){
	switch( state ){
		case XDG_TOPLEVEL_STATE_MAXIMIZED:    return "maximized";
		case XDG_TOPLEVEL_STATE_FULLSCREEN:   return "fullscreen";
		case XDG_TOPLEVEL_STATE_RESIZING:     return "resizing";
		case XDG_TOPLEVEL_STATE_ACTIVATED:    return "activated";
		// v2:
		case XDG_TOPLEVEL_STATE_TILED_LEFT:   return "tiled_left";
		case XDG_TOPLEVEL_STATE_TILED_RIGHT:  return "tiled_right";
		case XDG_TOPLEVEL_STATE_TILED_TOP:    return "tiled_top";
		case XDG_TOPLEVEL_STATE_TILED_BOTTOM: return "tiled_bottom";
		default: return "unrecognized state";
	}
}

bool hasState( const std::vector<uint32_t>& states, const uint32_t state ){
	for( const auto s : states ) if( s == state ) return true;
	return false;
}

void toplevelConfigureHandler( void* data, xdg_toplevel* toplevel, int32_t width, int32_t height, wl_array* states ) noexcept{
	//logger << "toplevel config: w=" << width << ", h=" << height << ", states={";

	std::vector<uint32_t> statesv;
	for( uint32_t* state = (uint32_t*)states->data; (const uint8_t*) state < ((const uint8_t*) states->data + states->size); ++state ){
		statesv.push_back( *state );
	}

/*
	bool first = true;
	for( const auto state : statesv ){
		if( !first ) logger << ", ";
		else first = false;
		logger << state_to_string( state );
	}
	logger << "}\n";
*/

	PlatformWindowImpl* wnd = (PlatformWindowImpl*)data;
	assert( wnd->inited );

	if( width != wnd->width || height != wnd->height ){
		if( !(width == 0 && height == 0) ){
			wnd->width = static_cast<uint32_t>( width );
			wnd->height = static_cast<uint32_t>( height );

			if(  !hasState( statesv, XDG_TOPLEVEL_STATE_MAXIMIZED )
			  && !hasState( statesv, XDG_TOPLEVEL_STATE_FULLSCREEN )  ){
				wnd->restoredWidth = static_cast<uint32_t>( width );
				wnd->restoredHeight = static_cast<uint32_t>( height );
			}
		}
		else{
			wnd->width = wnd->restoredWidth;
			wnd->height = wnd->restoredHeight;
		}

		wnd->hasSwapchain = sizeEventHandler();
	}

	if(  hasState( statesv, XDG_TOPLEVEL_STATE_FULLSCREEN )  ) wnd->fullscreen = true;
	else wnd->fullscreen = false;

	if(  hasState( statesv, XDG_TOPLEVEL_STATE_MAXIMIZED )  ) wnd->maximized = true;
	else wnd->maximized = false;
}

void toplevelCloseHandler( void* data, xdg_toplevel* xdg_toplevel ) noexcept{
	//logger << "toplevel close" << std::endl;

	PlatformWindowImpl* window = (PlatformWindowImpl*)data;
	window->quit = true;
}

void wmBasePingHandler( void* data, xdg_wm_base* xdg_wm_base, uint32_t serial ) noexcept{
	//logger << "wm_base ping" << std::endl;
	xdg_wm_base_pong( xdg_wm_base, serial );
}

const wl_registry_listener registryListener = { registryGlobalHandler, registryGlobalRemoveHandler };
const wl_seat_listener seatListener = { seatCapsHandler, [](void*, wl_seat*, const char*){} };
TODO( "Should probably implement some of these callbacks.");
const wl_keyboard_listener keyboardListener = { keyboardKeymapHandler, [](void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*){}, [](void*, wl_keyboard*, uint32_t, wl_surface*){}, keyboardKeyHandler, [](void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t){} };
const wl_pointer_listener pointerListener = { [](void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t){}, [](void*, wl_pointer*, uint32_t, wl_surface*){}, [](void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t){}, pointerButtonHandler, [](void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t){} };
const xdg_wm_base_listener xdgWmBaseListerner = { wmBasePingHandler };
const xdg_surface_listener xdgSurfaceListener = { xdgSurfaceConfigureHandler };
const xdg_toplevel_listener xdgToplevelListener = { toplevelConfigureHandler, toplevelCloseHandler };

PlatformWindow initWindow( const std::string& name, uint32_t canvasWidth, uint32_t canvasHeight ){
	auto wnd = std::make_shared<PlatformWindowImpl>();

	wnd->width = canvasWidth;
	wnd->restoredWidth = canvasWidth;
	wnd->height = canvasHeight;
	wnd->restoredHeight = canvasHeight;

	wnd->display = wl_display_connect( nullptr ); RUNTIME_ASSERT( wnd->display, "wl_display_connect" );
	wnd->registry = wl_display_get_registry( wnd->display ); RUNTIME_ASSERT( wnd->registry, "wl_display_get_registry" );
	{const auto err = wl_registry_add_listener( wnd->registry, &::registryListener, wnd.get() ); RUNTIME_ASSERT( !err, "wl_registry_add_listener" );}

	{const auto dispatched = wl_display_roundtrip( wnd->display ); RUNTIME_ASSERT( dispatched != -1, "wl_display_roundtrip" );}
	// dispatch should be redundant to the roundtrip, but I like it explicit
	{const auto dispatched = wl_display_dispatch_pending( wnd->display ); RUNTIME_ASSERT( dispatched != -1, "wl_display_dispatch_pending" );}
	RUNTIME_ASSERT( wnd->compositor, "wl_registry_bind(\"wl_compositor\")" );
	RUNTIME_ASSERT( wnd->wmBase, "wl_registry_bind(\"xdg_wm_base\")" );
	RUNTIME_ASSERT( wnd->seat, "wl_registry_bind(\"wl_seat\")" );

	wnd->xkbContext = xkb_context_new( XKB_CONTEXT_NO_FLAGS ); RUNTIME_ASSERT( wnd->xkbContext, "xkb_context_new" );

	{const auto err = wl_seat_add_listener( wnd->seat, &seatListener, wnd.get() ); RUNTIME_ASSERT( !err, "wl_seat_add_listener" );}
	{const auto dispatched = wl_display_roundtrip( wnd->display ); RUNTIME_ASSERT( dispatched != -1, "wl_display_roundtrip" );}
	{const auto dispatched = wl_display_dispatch_pending( wnd->display ); RUNTIME_ASSERT( dispatched != -1, "wl_display_dispatch_pending" );}

	wnd->keyboard = wl_seat_get_keyboard( wnd->seat ); RUNTIME_ASSERT( wnd->keyboard, "wl_seat_get_keyboard" );
	{const auto err = wl_keyboard_add_listener( wnd->keyboard, &keyboardListener, wnd.get() ); RUNTIME_ASSERT( !err, "wl_keyboard_add_listener" );}
	wnd->pointer = wl_seat_get_pointer( wnd->seat ); RUNTIME_ASSERT( wnd->pointer, "wl_seat_get_pointer" );
	{const auto err = wl_pointer_add_listener( wnd->pointer, &pointerListener, wnd.get() ); RUNTIME_ASSERT( !err, "wl_pointer_add_listener" );}

	wnd->surface = wl_compositor_create_surface( wnd->compositor ); RUNTIME_ASSERT( wnd->surface, "wl_compositor_create_surface" );

	{const auto err = xdg_wm_base_add_listener( wnd->wmBase, &xdgWmBaseListerner, wnd.get() ); RUNTIME_ASSERT( !err, "xdg_wm_base_add_listener" );}
	wnd->xdgSurface = xdg_wm_base_get_xdg_surface( wnd->wmBase, wnd->surface ); RUNTIME_ASSERT( wnd->xdgSurface, "xdg_wm_base_get_xdg_surface" );
	wnd->toplevel = xdg_surface_get_toplevel( wnd->xdgSurface ); RUNTIME_ASSERT( wnd->toplevel, "xdg_surface_get_toplevel" );
	{const auto err = xdg_surface_add_listener( wnd->xdgSurface, &xdgSurfaceListener, wnd.get() ); RUNTIME_ASSERT( !err, "xdg_surface_add_listener" );}
	{const auto err = xdg_toplevel_add_listener( wnd->toplevel, &xdgToplevelListener, wnd.get() ); RUNTIME_ASSERT( !err, "xdg_toplevel_add_listener" );}

	wnd->title = name + " -- Wayland";
	xdg_toplevel_set_title( wnd->toplevel, "wnd->title.c_str()" );
	TODO( "Shows \"Unknown\" in Ubuntu on taskbar.")

	wl_surface_commit( wnd->surface );
	{const auto sent = wl_display_flush( wnd->display ); RUNTIME_ASSERT( sent != -1, "wl_display_flush" );}

	wnd->inited = true;
	return {wnd};
}

void killWindow( PlatformWindow window ){
	const auto wnd = window.impl;
	wnd->inited = false;

	xkb_state_unref( wnd->xkbState );
	xkb_keymap_unref( wnd->keymap );
	xkb_context_unref( wnd->xkbContext );

	TODO( "Explicitly kill all Wayland stuff" );
	wl_display_disconnect( wnd->display );
}

void showWindow( PlatformWindow windowh ){
	auto wnd = windowh.impl;

	// wait for first size event
	{const auto dispatched = wl_display_roundtrip( wnd->display ); RUNTIME_ASSERT( dispatched != -1, "wl_display_roundtrip" );}
	// dispatch should be redundant to the roundtrip, but I like it explicit
	{const auto dispatched = wl_display_dispatch_pending( wnd->display ); RUNTIME_ASSERT( dispatched != -1, "wl_display_dispatch_pending" );}

	assert( wnd->hasSwapchain );

	// In Wayland window is mapped with first present
	paintEventHandler();
}

int messageLoop( PlatformWindow windowh ){
	auto wnd = windowh.impl;
	wnd->quit = false;

	while( !wnd->quit ){
		{const auto dispatched = wl_display_dispatch_pending( wnd->display ); RUNTIME_ASSERT( dispatched != -1, "wl_display_dispatch_pending" );}

		if( wnd->hasSwapchain ){
			TODO( "Use frame callback instead?" );
			paintEventHandler();
		}
	}

	return 0;
}

VkSurfaceKHR initSurface( VkInstance instance, PlatformWindow windowh ){
	auto wnd = windowh.impl;

	const VkWaylandSurfaceCreateInfoKHR surfaceInfo{
		VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		wnd->display,
		wnd->surface
	};

	VkSurfaceKHR surface;
	VkResult errorCode = vkCreateWaylandSurfaceKHR( instance, &surfaceInfo, nullptr, &surface ); RESULT_HANDLER( errorCode, "vkCreateWaylandSurfaceKHR" );
	return surface;
}


#endif //COMMON_WAYLAND_WSI_H