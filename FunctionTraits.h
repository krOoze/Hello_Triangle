// A template helper to get the result and parameter types of a function type

#ifndef COMMON_FUNCTION_TRAITS_H
#define COMMON_FUNCTION_TRAITS_H

#include <functional>

template< typename F >
struct function_traits;

template< typename Result, typename... Params >
struct function_traits< Result( Params... ) >{
	static const size_t paramCount = sizeof...( Params );

	using result = Result;

	template< size_t i >
	using param = typename std::tuple_element<  i, std::tuple< Params... >  >::type;
};

template< typename Result, typename... Params >
struct function_traits< Result(*)( Params... ) >
	: public function_traits< Result( Params... ) > {
};

template< typename Result, typename... Params >
struct function_traits<  std::function< Result( Params... ) >  >
	: public function_traits< Result( Params... ) >{
};

// shortcuts to help avoid the weird "typename" and "template" disambiguators
template< typename T >
using function_result_t = typename function_traits<T>::result;

template< typename T, size_t i >
using function_param_t = typename function_traits<T>::template param<i>; // lol, that's evil syntax

#endif //COMMON_FUNCTION_TRAITS_H