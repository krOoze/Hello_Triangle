// Reusable error handling primitives for Vulkan

#ifndef COMMON_ERROR_HANDLING_H
#define COMMON_ERROR_HANDLING_H

#include <iostream>
#include <string>
#include <sstream>

#include <vulkan/vulkan.h>

#include "VulkanIntrospection.h"

struct VulkanResultException{
	const char* file;
	unsigned line;
	const char* func;
	const char* source;
	VkResult result;

	VulkanResultException( const char* file, unsigned line, const char* func, const char* source, VkResult result )
	: file( file ), line( line ), func( func ), source( source ), result( result ){}
};

#define RESULT_HANDLER( errorCode, source )  if( errorCode ) throw VulkanResultException( __FILE__, __LINE__, __func__, source, errorCode )
#define RESULT_HANDLER_EX( cond, errorCode, source )  if( cond ) throw VulkanResultException( __FILE__, __LINE__, __func__, source, errorCode )

#define RUNTIME_ASSERT( cond, source )  if( !(cond) ) throw source " failed";

// just use cout for logging now
std::ostream& logger = std::cout;

enum class Highlight{ off, on };
void genericDebugCallback( std::string flags, Highlight highlight, std::string msgCode, std::string object, const char* message );

VKAPI_ATTR VkBool32 VKAPI_CALL genericDebugReportCallback(
	VkDebugReportFlagsEXT msgFlags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t /*location*/,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* /*pUserData*/
);

VKAPI_ATTR VkBool32 VKAPI_CALL genericDebugUtilsCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* /*pUserData*/
);

enum class DebugObjectType{ debugReport, debugUtils } tag;
struct DebugObjectVariant{
	DebugObjectType tag;
	union{
		VkDebugReportCallbackEXT debugReportCallback;
		VkDebugUtilsMessengerEXT debugUtilsMessenger;
	};
};

DebugObjectVariant initDebug( const VkInstance instance, const DebugObjectType debugExtension, const VkDebugUtilsMessageSeverityFlagsEXT debugSeverity, const VkDebugUtilsMessageTypeFlagsEXT debugType );
void killDebug( VkInstance instance, DebugObjectVariant debug );

VkDebugReportFlagsEXT translateFlags( const VkDebugUtilsMessageSeverityFlagsEXT debugSeverity, const VkDebugUtilsMessageTypeFlagsEXT debugType );

// Implementation
//////////////////////////////////

void genericDebugCallback( std::string flags, Highlight highlight, std::string msgCode, std::string object, const char* message ){
	using std::endl;
	using std::string;

	const string report = flags + ": " + object + ": " + msgCode + ", \"" + message + '"';

	if( highlight != Highlight::off ){
			const string border( 80, '!' );

			logger << border << endl;
			logger << report << endl;
			logger << border << endl << endl;
	}
	else{
		logger << report << endl;
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL genericDebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t /*location*/,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* /*pUserData*/
){
	using std::to_string;
	using std::string;

	Highlight highlight;
	if( (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) || (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) || (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) ){
		highlight = Highlight::on;
	}
	else highlight = Highlight::off;


	genericDebugCallback(  dbrflags_to_string( flags ), highlight, string(pLayerPrefix) + ", " + to_string( messageCode ), to_string( objectType ) + "(" + to_string_hex( object ) + ")", pMessage  );

	return VK_FALSE; // no abort on misbehaving command
}

VKAPI_ATTR VkBool32 VKAPI_CALL genericDebugUtilsCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* /*pUserData*/
){
	using std::to_string;
	using std::string;

	Highlight highlight;
	if( (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) || (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)){
		highlight = Highlight::on;
	}
	else highlight = Highlight::off;

	string objects;
	bool first = true;
	for( uint32_t i = 0; i < pCallbackData->objectCount; ++i ){
		const auto& obj = pCallbackData->pObjects[i];

		if( first ) first = false;
		else objects += ", ";
		objects +=  to_string( obj.objectType ) + "(" + to_string_hex( obj.objectHandle ) + ")";
	}
	objects = "[" + objects + "]";

	genericDebugCallback(  dbutype_to_string( messageTypes ) + "+" + to_string( messageSeverity ), highlight, string(pCallbackData->pMessageIdName) + "(" + to_string( pCallbackData->messageIdNumber ) + ")", objects, pCallbackData->pMessage  );

	return VK_FALSE; // no abort on misbehaving command
}

VkDebugReportFlagsEXT translateFlags( const VkDebugUtilsMessageSeverityFlagsEXT debugSeverity, const VkDebugUtilsMessageTypeFlagsEXT debugType ){
	VkDebugReportFlagsEXT flags = 0;
	if( (debugType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) || (debugType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ){
		if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
		if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
		if( (debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) && (debugType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) ) flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ) flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
		if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ) flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	}

	return flags;
}

DebugObjectVariant initDebug( const VkInstance instance, const DebugObjectType debugExtension, const VkDebugUtilsMessageSeverityFlagsEXT debugSeverity, const VkDebugUtilsMessageTypeFlagsEXT debugType ){
	DebugObjectVariant debug;
	debug.tag = debugExtension;

	if( debugExtension == DebugObjectType::debugUtils ){
		const VkDebugUtilsMessengerCreateInfoEXT dmci = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			nullptr, // pNext
			0, // flags
			debugSeverity,
			debugType,
			::genericDebugUtilsCallback,
			nullptr // pUserData
		};

		const VkResult errorCode = vkCreateDebugUtilsMessengerEXT( instance, &dmci, nullptr, &debug.debugUtilsMessenger ); RESULT_HANDLER( errorCode, "vkCreateDebugUtilsMessengerEXT" );
	}
	else if( debugExtension == DebugObjectType::debugReport ){
		const VkDebugReportCallbackCreateInfoEXT debugCreateInfo{
			VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			nullptr, // pNext
			translateFlags( debugSeverity, debugType ),
			::genericDebugReportCallback,
			nullptr // pUserData
		};

		const VkResult errorCode = vkCreateDebugReportCallbackEXT( instance, &debugCreateInfo, nullptr, &debug.debugReportCallback ); RESULT_HANDLER( errorCode, "vkCreateDebugReportCallbackEXT" );
	}
	else{
		throw "initDebug: unknown debug extension";
	}

	return debug;
}

void killDebug( const VkInstance instance, const DebugObjectVariant debug ){
	if( debug.tag == DebugObjectType::debugUtils ){
		vkDestroyDebugUtilsMessengerEXT( instance, debug.debugUtilsMessenger, nullptr );
	}
	else if( debug.tag == DebugObjectType::debugReport ){
		vkDestroyDebugReportCallbackEXT( instance, debug.debugReportCallback, nullptr );
	}
	else{
		throw "initDebug: unknown debug extension";
	}
}

#endif //COMMON_ERROR_HANDLING_H
