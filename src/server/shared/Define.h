/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_DEFINE_H
#define TRINITY_DEFINE_H

#include "CompilerDefs.h"

#include <ace/Basic_Types.h>
#include <ace/ACE_export.h>

#include <cstddef>
#include <algorithm>

#define TRINITY_LITTLEENDIAN 0
#define TRINITY_BIGENDIAN    1

#if !defined(TRINITY_ENDIAN)
#  if defined (ACE_BIG_ENDIAN)
#    define TRINITY_ENDIAN TRINITY_BIGENDIAN
#  else //ACE_BYTE_ORDER != ACE_BIG_ENDIAN
#    define TRINITY_ENDIAN TRINITY_LITTLEENDIAN
#  endif //ACE_BYTE_ORDER
#endif //TRINITY_ENDIAN

#if PLATFORM == PLATFORM_WINDOWS
#  define TRINITY_PATH_MAX MAX_PATH
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#  ifndef DECLSPEC_DEPRECATED
#    define DECLSPEC_DEPRECATED __declspec(deprecated)
#  endif //DECLSPEC_DEPRECATED
#else //PLATFORM != PLATFORM_WINDOWS
#  define TRINITY_PATH_MAX PATH_MAX
#  define DECLSPEC_NORETURN
#  define DECLSPEC_DEPRECATED
#endif //PLATFORM

#if !defined(COREDEBUG)
#  define TRINITY_INLINE inline
#else //COREDEBUG
#  if !defined(TRINITY_DEBUG)
#    define TRINITY_DEBUG
#  endif //TRINITY_DEBUG
#  define TRINITY_INLINE
#endif //!COREDEBUG

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F, V) __attribute__ ((format (printf, F, V)))
#  define ATTR_DEPRECATED __attribute__((deprecated))
#  define TRINITY_CONSTEXPR constexpr
#else //COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F, V)
#  define ATTR_DEPRECATED
#if _MSC_VER >= 1900
#  define TRINITY_CONSTEXPR constexpr
#else
#  define TRINITY_CONSTEXPR
#endif
#endif //COMPILER == COMPILER_GNU

#if COMPILER_HAS_CPP11_SUPPORT
#  define OVERRIDE override
#  define FINAL final
#else
#  define OVERRIDE
#  define FINAL
#endif //COMPILER_HAS_CPP11_SUPPORT

#define UI64FMTD ACE_UINT64_FORMAT_SPECIFIER
#define UI64LIT(N) ACE_UINT64_LITERAL(N)

#define SI64FMTD ACE_INT64_FORMAT_SPECIFIER
#define SI64LIT(N) ACE_INT64_LITERAL(N)

#define SIZEFMTD ACE_SIZE_T_FORMAT_SPECIFIER

#ifdef TRINITY_API_USE_DYNAMIC_LINKING
#  if TRINITY_COMPILER == TRINITY_COMPILER_MICROSOFT
#    define TC_API_EXPORT __declspec(dllexport)
#    define TC_API_IMPORT __declspec(dllimport)
#  elif TRINITY_COMPILER == TRINITY_COMPILER_GNU
#    define TC_API_EXPORT __attribute__((visibility("default")))
#    define TC_API_IMPORT
#  else
#    error compiler not supported!
#  endif
#else
#  define TC_API_EXPORT
#  define TC_API_IMPORT
#endif

#ifdef TRINITY_API_EXPORT_GAME
#  define TC_GAME_API TC_API_EXPORT
#else
#  define TC_GAME_API TC_API_IMPORT
#endif

typedef ACE_INT64 int64;
typedef ACE_INT32 int32;
typedef ACE_INT16 int16;
typedef ACE_INT8 int8;
typedef ACE_UINT64 uint64;
typedef ACE_UINT32 uint32;
typedef ACE_UINT16 uint16;
typedef ACE_UINT8 uint8;

#define DEFINE_ENUM_BIT_OPS(NAME, BASE)                 \
	inline NAME operator~(NAME v)                       \
{														\
	return static_cast<NAME>(~static_cast<BASE>(v));	\
};														\
	inline NAME operator|(NAME one, NAME two)           \
{														\
	return static_cast<NAME>(							\
	static_cast<BASE>(one)								\
	| static_cast<BASE>(two)							\
	);													\
};														\
	inline NAME operator&(NAME one, NAME two)           \
{														\
	return static_cast<NAME>(							\
	static_cast<BASE>(one)								\
	& static_cast<BASE>(two)							\
	);													\
};														\
	inline NAME& operator|=(NAME& one, NAME two)        \
{														\
	one = one | two;									\
	return one;											\
};														\
	inline NAME& operator&=(NAME& one, NAME two)        \
{														\
	one = one & two;									\
	return one;											\
};

enum DBCFormer
{
    FT_NA='x',                                              //not used or unknown, 4 byte size
    FT_NA_BYTE='X',                                         //not used or unknown, byte
    FT_STRING='s',                                          //char*
    FT_FLOAT='f',                                           //float
    FT_INT='i',                                             //uint32
    FT_BYTE='b',                                            //uint8
    FT_SORT='d',                                            //sorted by this field, field is not included
    FT_IND='n',                                             //the same, but parsed to data
    FT_LOGIC='l',                                           //Logical (boolean)
    FT_SQL_PRESENT='p',                                     //Used in sql format to mark column present in sql dbc
    FT_SQL_ABSENT='a'                                       //Used in sql format to mark column absent in sql dbc
};
namespace zonaxtrema {
		template<class ArgumentType, class ResultType>
		struct unary_function
		{
			typedef ArgumentType argument_type;
			typedef ResultType result_type;
		};
		template< class RandomIt >
		void random_shuffle(RandomIt first, RandomIt last) {
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(first, last, g);
		};
	}
#endif //TRINITY_DEFINE_H
