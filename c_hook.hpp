#pragma once

#ifndef c_hook_h
#define c_hook_h

// comment out to disable console output
#define c_verbose

#ifdef c_verbose
	#define c_print(...) printf( __VA_ARGS__ );
#else
	#define c_print(...) ((void)0);
#endif

#if !(defined _M_IX86) && !(defined _M_X64) && !(defined __i386__) && !(defined __x86_64__)
	#error c_hook only supports x86/x64.
#endif

#include <Windows.h>
#include <string>
#include <optional>
#include <type_traits>
#include <unordered_map>

#include "minhook/include/MinHook.h"

class c_hook 
{
public:
	// create new hook
	c_hook( void* target, void* hook ); 
	// check if hook created successfully
	bool created( ); 
	// enable the hook
	bool enable( );
	// disable the hook
	bool disable( );
	// return last error as string
	std::string error( );

	// get original
	template <typename T>
	std::enable_if_t<std::is_pointer_v<T>, T> get( )
	{
		return static_cast<T>( this->m_original );
	}

	// get original
	template <typename T>
	std::enable_if_t<std::is_lvalue_reference_v<T>, T> get( )
	{
		return *static_cast<std::add_pointer_t<std::remove_reference_t<T>>>( this->m_original );
	}

	// get original
	template <typename T>
	std::enable_if_t<std::is_same_v<T, std::uintptr_t>, T> get( )
	{
		return reinterpret_cast<std::uintptr_t>( this->m_original );
	}

private:
	bool m_active;
	bool m_created;
	void* m_target;
	void* m_hook;
	void* m_original;
	MH_STATUS m_lasterror;
};

struct c_entry
{
	std::string name;
	c_hook* hook;
};

class c_hooks
{
public:
	// init minhook
    static void init( );
	// create hook and insert it into manager, optionally enables it directly
    static std::optional<c_hook*> create( const std::string& name, c_hook* hook, const bool enable = false );
	// enable all hooks ( won't continue if one fails to enable )
    static bool enable_all( );
	// disable all hooks ( skips hooks which didn't create successfully )
    static bool disable_all( );
	// removes a hook from the list ( disables hook by default )
	static bool remove( const std::string& name, const bool disable = true );
	// returns the corresponding c_hook* class
	static std::optional<c_hook*> get( const std::string& name );

	// returns the original, doesn't search if provided original != nullptr
    template <typename T>
    static T get( const std::string& name, T* original )
    {
		if (!original)
			return T( );

        if (*original != nullptr)
            return *original;

        auto it = m_hooks.find( name );
        if (it == m_hooks.end( ))
            return T( );

        c_hook* hook = it->second;
        *original = hook->get<T>( );
        return *original;
    }

private:
    static std::unordered_map<std::string, c_hook*> m_hooks;
};

#endif // c_hook_h
