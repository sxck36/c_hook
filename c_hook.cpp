#include "c_hook.hpp"

#pragma region c_hook

c_hook::c_hook( void* target, void* hook )
{
	this->m_active = false;
	this->m_created = false;
	this->m_target = target;
	this->m_hook = hook;
	this->m_original = nullptr;
	this->m_lasterror = MH_OK;

	MH_STATUS status = MH_CreateHook( target, hook, reinterpret_cast<void**>( &this->m_original ) );
	if (status != MH_OK)
	{
		this->m_lasterror = status;
		return;
	}
	
	this->m_created = true;
}

bool c_hook::created( )
{
	return this->m_created;
}

bool c_hook::enable( )
{
	if (!this->m_created)
		return false;

	if (this->m_active)
		return true;

	MH_STATUS status = MH_EnableHook( this->m_target );
	if (status != MH_OK)
	{
		this->m_lasterror = status;
		return false;
	}

	this->m_active = true;
	this->m_lasterror = MH_OK;
	return true;
}

bool c_hook::disable( )
{
	if (!this->m_created || !this->m_active)
		return true;

	MH_STATUS status = MH_DisableHook( this->m_target );
	if (status != MH_OK)
	{
		this->m_lasterror = status;
		return false;
	}
	
	this->m_active = false;
	return true;
}

std::string c_hook::error( )
{
	return MH_StatusToString( this->m_lasterror );
}

#pragma endregion


#pragma region c_hooks

std::unordered_map<std::string, c_hook*> c_hooks::m_hooks;

bool c_hooks::init( )
{
	MH_STATUS status = MH_Initialize( );
	if (status != MH_OK)
	{
		c_print( "c_hooks >> Failed to init, error: %s\n", MH_StatusToString( status ) );
		return false;
	}

	return true;
}

std::optional<c_hook*> c_hooks::create( const std::string& name, c_hook* hook, const bool enable )
{
	auto it = m_hooks.find( name );
	if (it != m_hooks.end( ))
		return it->second;

	if (enable)
	{
		if (!hook->created( ))
		{
			c_print( "c_hooks >> Failed to create hook for: %s, error: %s\n", name.c_str( ), hook->error( ).c_str( ) );
			return nullptr;
		}

		if (!hook->enable( ))
		{
			c_print( "c_hooks >> Failed to enable hook for: %s, error: %s\n", name.c_str( ), hook->error( ).c_str( ) );
			return nullptr;
		}
	}

	m_hooks.emplace( name, hook );
	return hook;
}

bool c_hooks::enable_all( )
{
	for (auto& entry : m_hooks)
	{
		auto hook = entry.second;
		auto name = entry.first;

		if (!hook->created( ))
		{
			c_print( "c_hooks >> Failed to create hook for: %s, error: %s\n", name.c_str( ), hook->error( ).c_str( ) );
			return false;
		}

		if (!hook->enable( ))
		{
			c_print( "c_hooks >> Failed to enable hook for: %s, error: %s\n", name.c_str( ), hook->error( ).c_str( ) );
			return false;
		}
	}
	return true;
}

bool c_hooks::disable_all( )
{
	for (auto& entry : m_hooks)
	{
		auto hook = entry.second;
		auto name = entry.first;

		if (!hook->created( ))
			continue;

		if (!hook->disable( ))
		{
			c_print( "c_hooks >> Failed to disable hook for: %s, error: %s\n", name.c_str( ), hook->error( ).c_str( ) );
			return false;
		}
	}
	return true;
}

bool c_hooks::remove( const std::string& name, const bool disable )
{
	auto it = m_hooks.find( name );
	if (it == m_hooks.end( ))
		return true;

	if (disable)
	{
		auto hook = it->second;
		if (!hook->disable( ))
		{
			c_print( "c_hooks >> Failed to disable/remove hook for: %s, error: %s\n", name.c_str( ), hook->error( ).c_str( ) );
			return false;
		}
	}

	delete it->second;

	m_hooks.erase( it );
	return true;
}

std::optional<c_hook*> c_hooks::get( const std::string& name )
{
	auto it = m_hooks.find( name );
	if (it == m_hooks.end( ))
		return nullptr;

	return it->second;
}

#pragma endregion