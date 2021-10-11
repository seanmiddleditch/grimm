// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "platform.h"

#if !defined(UP_PLATFORM_WINDOWS)
#    error "WindowsKit.h must only be included on Windows, check for UP_PLATFORM_WINDOWS before including"
#endif

#if !defined(_UNICODE)
#    error "Always compile Windows apps in _UNICODE mode to ensure compatibility"
#endif

#undef WIN32_LEAN_AND_MEAN

#undef NOGDICAPMASKS
#undef NOVIRTUALKEYCODES
#undef NOWINSTYLES
#undef NOSYSMETRICS
#undef NOMENUS
#undef NOKEYSTATES
#undef NOSYSCOMMANDS
#undef NORASTEROPS
#undef OEMRESOURCE
#undef NOATOM
#undef NOCOLOR
#undef NODRAWTEXT
#undef NOGDI
#undef NOKERNEL
#undef NOMEMMGR
#undef NOMETAFILE
#undef NOMINMAX
#undef NOOPENFILE
#undef NOSCROLL
#undef NOSERVICE
#undef NOSOUND
#undef NOTEXTMETRIC
#undef NOWH
#undef NOCOMM
#undef NOKANJI
#undef NOHELP
#undef NOPROFILER
#undef NODEFERWINDOWPOS
#undef NOMCX

// turn off all kinds of crap before including windows.h
#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS 1
#define NOVIRTUALKEYCODES 1
#define NOWINSTYLES 1
#define NOSYSMETRICS 1
#define NOMENUS 1
#define NOKEYSTATES 1
#define NOSYSCOMMANDS 1
#define NORASTEROPS 1
#define OEMRESOURCE 1
#define NOATOM 1
#define NOCOLOR 1
#define NODRAWTEXT 1
#define NOGDI 1
#define NOKERNEL 1
#define NOMEMMGR 1
#define NOMETAFILE 1
#define NOMINMAX 1
#define NOOPENFILE 1
#define NOSCROLL 1
#define NOSERVICE 1
#define NOSOUND 1
#define NOTEXTMETRIC 1
#define NOWH 1
#define NOCOMM 1
#define NOKANJI 1
#define NOHELP 1
#define NOPROFILER 1
#define NODEFERWINDOWPOS 1
#define NOMCX 1

// ensure that things we need will be included, though
#undef NOMSG
#undef NOICONS
#undef NONLS
#undef NOUSER
#undef NOSHOWWINDOW
#undef NOWINMESSAGES
#undef NOMB
#undef NOCLIPBOARD
#undef NOCTRLMGR
#undef NOWINOFFSETS

// include Windows headers after the configuration
#include <windows.h>

// undef all the stupid shit that Windows turns into macros
#undef SendMessage
#undef createWindow
#undef LoadLibrary
#undef OutputDebugString
#undef openFile
#undef GetCurrentDirectory
#undef CreateFile
#undef GetFileAttributesEx
#undef CreateDirectory
#undef copyFileContents
