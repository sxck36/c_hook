# c_hook

## Overview

A simple MinHook wrapper. Use it with the included c_hooks manager for easy management, or handle everything manually for complete control.

## Example usage

```cpp
#include <Windows.h>
#include <iostream>

#include "c_hook/c_hook.hpp"

typedef int (__stdcall* MessageBox_T)( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType );

int MessageBoxHook( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType )
{
    static MessageBox_T original = nullptr; 
    c_hooks::get<MessageBox_T>( "MessageBoxA", &original );

    return original( hWnd, "Captain Hook!", "test", uType );
}

int main()
{
    if (!c_hooks::init( ))
        return 0;

    MessageBoxA( NULL, "Before hooking!", "Test", NULL );

    auto test = c_hooks::create( "MessageBoxA", new c_hook( &MessageBoxA, &MessageBoxHook ), true );

    MessageBoxA( NULL, "After hooking!", "Test", NULL );

    c_hooks::remove( "MessageBoxA", true );

    MessageBoxA( NULL, "After restoring!", "Test", NULL );

    return 0;
}
```

## Extra

Feel free to contribute, report issues, or make suggestions/improvements!
