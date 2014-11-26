/*!
 * SteamID Parser
 *
 * Copyright 2014 Mukunda Johnson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#pragma once
#ifndef _STEAMID_
#define _STEAMID_

#include <string>
#include <cctype>

/** ---------------------------------------------------------------------------
 * SteamID
 *
 * Contains a User Steam ID.
 *
 * @author Mukunda Johnson
 */
class SteamID {

public:
	enum class Formats : int {
		AUTO = 0,		// Auto-detect format --- this also supports 
						// other unlisted formats such as 
						// full profile URLs.
		STEAMID32 = 1,	// Classic STEAM_x:y:zzzzzz | x = 0/1
		STEAMID64 = 2,	// SteamID64: 7656119xxxxxxxxxx
		STEAMID3 = 3,	// SteamID3 format: [U:1:xxxxxx]
		S32 = 4,		// Raw 32-bit SIGNED format. 
						// this is a raw steamid index that overflows
						// into negative bitspace.
						// This is the format that SourceMod returns
						// with GetSteamAccountID, and will always
						// fit into a 32-bit signed variable. (e.g.
						// a 32-bit PHP integer).
		RAW = 5,		// Raw index. like 64-bit minus the base value.
		
	};
	
	// 64-bit type.
	using bigint = long long;
	using uint = unsigned int;

	// base constant of 64-bit Steam IDs
	static const bigint STEAMID64_BASE = 76561197960265728L;
	
	// max allowed value. (sanity check)
	// 2^36; update this in approx 2,400,000 years
	static const bigint MAX_VALUE = 68719476736L;
	 
	/** -----------------------------------------------------------------------
	 * Parse a Steam ID.
	 *
	 * @param input      Input to parse.
	 *
	 * @param format     Input formatting, see Format constants.
	 *                   Defaults to Format::AUTO which detects the format. 
	 *
	 * @param detect_raw Detect and parse RAW values. (only used with
	 *                   Format::AUTO. e.g "123" will resolve to the
	 *                   SteamID with the raw value 123.
	 *                   Default option set with ParseRawDefault.
	 *
	 * @returns SteamID instance or an empty SteamID if the parsing fails.
	 */
	static SteamID Parse( const std::string &input, 
						  Formats format = Formats::AUTO,
						  int detect_raw = ParseRawDefault() ) {
		
		if( input.empty() ) return SteamID(); // no input...

		try {
			switch( format ) {
			
			//-----------------------------------------------------------------
			case Formats::STEAMID32: {
				
				// regex is slow as fuck for some reason.
				if( input.size() < 11  || input[0] != 'S' || input[1] != 'T' 
					|| input[2] != 'E' || input[3] != 'A' || input[4] != 'M'
					|| input[5] != '_' || !Is01(input,6)  || input[7] != ':'
					|| !Is01(input,8)  || input[9] != ':' ) return SteamID();

				// STEAM_X:Y:Z'
//				static const std::regex r( 
//					R"--(^STEAM_[0-1]:[0-1]:[0-9]+$)--", 
//					rc::icase | rc::optimize );
//
//				if( !std::regex_match( input, r ) ) return SteamID();

				bigint z = std::stoll( input.substr( 10 ) );
				z = (z << 1) + (input[8] - '0');
				
				SteamID result(z);
				//result.Cache( Format::STEAMID32, input );
				return result;
			
			//-----------------------------------------------------------------
			} case Formats::STEAMID64: {
			
				// allow digits only
				if( !IsDigits( input ) ) return SteamID();
				
				// convert to raw (subtract base)
				SteamID result( std::stoll( input ) - STEAMID64_BASE );
				//result.Cache( Format::STEAMID64, input );
				return result;

			//-----------------------------------------------------------------
			} case Formats::STEAMID3: {
			
				// [U:1:xxxxxx]
				if( input.size() < 7   || input[0] != '[' || input[1] != 'U'
					|| input[2] != ':' || input[3] != '1' || input[4] != ':'
					|| input[input.size()-1] != ']' 
					|| !IsDigits( input, 5, input.size() - 1 - 5 ) ) {
					
					return SteamID();
				}

				// slow.
//				static const std::regex r( R"--(^\[U:1:[0-9]+\]$)--", 
//						rc::optimize );
//				if( !std::regex_match( input, r ) ) return SteamID();
				
				SteamID result( std::stoll( 
						input.substr( 5, input.size() - 1 - 5 )));

				//result.Cache( Format::STEAMID3, input );
				return result;

			//-----------------------------------------------------------------
			} case Formats::S32: {
				
				// signed digits
				if( !IsDigits( input, input[0] == '-' ? 1:0 ) ) {
					return SteamID();
				}
			
				bigint a = std::stoll( input );
				if( a < 0 ) a += 4294967296L;

				SteamID result( a );
				//result.Cache( Format::S32, input );
				return result;

			//-----------------------------------------------------------------
			} case Formats::RAW: {
			
				// validate digits only
				if( !IsDigits( input ) ) return SteamID();
				
				return SteamID( std::stoll( input ));
			}}
		} catch( std::out_of_range& ) {

			// integer conversion out of range...
			return SteamID();
		}
		// Auto detect format:
		
		std::string cleaned = TrimString(input);
		SteamID result;
		result = Parse( cleaned, Formats::STEAMID32 );
		if( *result ) return result;
		result = Parse( cleaned, Formats::STEAMID64 );
		if( *result ) return result;
		result = Parse( cleaned, Formats::STEAMID3 );
		if( *result ) return result;
		
		result = TryConvertProfileURL( cleaned );
		if( *result ) return result;
			
	//		static const std::regex r_url( 
	//			R"--(^(?:https?:\/\/)?(?:www.)?steamcommunity.com\/profiles\/([0-9]+)$)--",
	//			rc::icase | rc::optimize );

	//		std::smatch matches;
	//		if( std::regex_match( cleaned, matches, r_url ) ) {
	//			result = Parse( matches[1], Formats::STEAMID64  );
	//			if( *result ) return result;
	//		} 
		
		if( detect_raw ) {
			result = Parse( input, Formats::S32 );
			if( *result ) return result;
			result = Parse( input, Formats::RAW );
			if( *result ) return result;
		}
		
		// unknown stem
		return SteamID();
	}
	
	/** ----------------------------------------------------------------------- 
	 * Format this SteamID to a string.
	 *
	 * @param format Output format. See Format constants.
	 * @returns      Formatted Steam ID, or an empty string if an invalid 
	 *                      format is given or the desired format cannot 
	 *                      contain the SteamID.
	 */
	std::string Format( Formats format ) const {
		 
		switch( format ) {
		case Formats::STEAMID32: {
			bigint z = m_value >> 1;
			int y = m_value & 1;
			return std::string("STEAM_1:") + std::to_string(y) 
						+ ":" + std::to_string(z);
				
		} case Formats::STEAMID64: {
			return std::to_string( m_value + STEAMID64_BASE );
				
		} case Formats::STEAMID3: {
			return std::string( "[U:1:" ) + 
					std::to_string(m_value) + ']';
				
		} case Formats::S32: {
			if( m_value >= 4294967296L ) {
				return ""; // too large for s32.
			}
				
			if( m_value >= 2147483648L ) {
				return std::to_string( m_value - 4294967296L );
			} 

			// -->
		} case Formats::RAW: {
			return std::to_string( m_value );
		}}

		return "";
	}

	/** -----------------------------------------------------------------------
	 * Set the default setting for detect_raw for Parse()
	 *
	 * @param detect_raw Default detect_raw value, see Parse function.
	 * @returns Current or updated setting.
	 */
	static bool ParseRawDefault( int detect_raw = -1 ) {
		static int option = false;
		if( detect_raw == -1 ) return !!option;
		option = !!detect_raw;
		return !!option;
	}

	/** -----------------------------------------------------------------------
	 * Overload for Format.
	 */
	std::string operator[]( Formats format ) const {
		return Format( format );
	}
	
	/** -----------------------------------------------------------------------
	 * Get raw value. 0 = empty
	 */
	bigint Value() const {
		return m_value;
	}
	
	/** -----------------------------------------------------------------------
	 * Get raw value. 0 = empty
	 */
	bigint operator*() const {
		return m_value;
	}
	
	/** -----------------------------------------------------------------------
	 * Returns true if this SteamID is empty/invalid.
	 */
	bool operator!() {
		return m_value == 0;
	}
	
	/** -----------------------------------------------------------------------
	 * Returns true if this SteamID is empty/invalid.
	 */
	bool Empty() const {
		return m_value == 0;
	}
	
	/** -----------------------------------------------------------------------
	 * Get 64-bit Steam ID.
	 */
	bigint To64() {
		return m_value + STEAMID64_BASE;
	}
	
	/** -----------------------------------------------------------------------
	 * Get raw value, same as operator*.
	 */
	bigint ToRaw() {
		return m_value;
	}

	/** -----------------------------------------------------------------------
	 * Get 32-bit value cast to signed.
	 */
	int ToS32() {
		if( m_value > 0xFFFFFFFF ) {
			return 0;
		}
		return (int)m_value;
	}

	/** -----------------------------------------------------------------------
	 * Parsing shortcut.
	 */
	SteamID( const std::string &input, Formats format = Formats::AUTO,
			 int detect_raw = ParseRawDefault() ) 
			 : SteamID( Parse( input, format, detect_raw )) {
	}
	
	/** -----------------------------------------------------------------------
	 * Construct a Steam ID.
	 *
	 * @param raw RAW value of Steam ID.
	 */
	SteamID( bigint raw ) 
			: m_value( (raw > 0 && raw <= MAX_VALUE) ? raw : 0 ) {
	}
	
	/** -----------------------------------------------------------------------
	 * An empty steam id.
	 */
	SteamID() : m_value(0) {
	}

	SteamID( const SteamID& o ) = default;
	SteamID( SteamID&& o ) {
		m_value = o.m_value;
	}
	SteamID& operator=( const SteamID& o ) = default;
	SteamID& operator=( SteamID&& o ) {
		m_value = o.m_value;
		return *this;
	}
	
private:

	bigint m_value;	// RAW Steam ID value. 
	
	//-------------------------------------------------------------------------
	static bool IsDigits( const std::string &str, size_t start = 0, 
						  size_t length = 9000 ) {

		for( size_t i = start; i != (start+length) && str[i]; i++ ) {
			if( str[i] < '0' || str[i] > '9' ) return false;
		}
		return true;
	} 

	//-------------------------------------------------------------------------
	static bool Is01( const std::string &str, size_t index ) {
		return str[index] == '0' || str[index] == '1';
	}

	//-------------------------------------------------------------------------
	static std::string TrimString( const std::string &input ) {
		
		int start = 0, end = (int)input.size()-1;
		if( end < 0 ) return "";

		while( std::isspace( input[start] )) { 
			start++;
			if( start == input.size() ) return "";
		}
		
		while( std::isspace( input[end] )) {
			end--; 
		}
		
		return input.substr( start, 1+end-start );
	}

	//-------------------------------------------------------------------------
	static SteamID TryConvertProfileURL( std::string &str ) {
		if( str[0] != 'h' && str[0] != 'w' && str[0] != 's' ) return SteamID();

		int lastslash = str.find_last_of( '/' );
		if( lastslash == std::string::npos ) return SteamID();
		if( lastslash == str.size()-1 ) {
			str.pop_back();
			lastslash = str.find_last_of( '/' );
			if( lastslash == std::string::npos ) return SteamID();
		}

		if( CheckProfilePrefix( str, lastslash ) ) {
			return Parse( str.substr( lastslash+1 ) );
		}
		return SteamID();
	}

	static bool CheckProfilePrefix( std::string &str, int end ) {
		// possible prefixes:
		// 0123456789012345678901234567890123456789
		// https://www.steamcommunity.com/profiles/
		// http://www.steamcommunity.com/profiles/
		// https://steamcommunity.com/profiles/
		// http://steamcommunity.com/profiles/
		// www.steamcommunity.com/profiles/
		// steamcommunity.com/profiles/

		if( end == 39 ) {
			return str.compare( 0, 1+end, 
					"https://www.steamcommunity.com/profiles/" ) == 0;
		} else if( end == 38 ) {
			return str.compare( 0, 1+end, 
					"http://www.steamcommunity.com/profiles/" ) == 0;
		} else if( end == 35 ) {
			return str.compare( 0, 1+end, 
					"https://steamcommunity.com/profiles/" ) == 0;
		} else if( end == 34 ) {
			return str.compare( 0, 1+end, 
					"http://steamcommunity.com/profiles/" ) == 0;
		} else if( end == 31 ) {
			return str.compare( 0, 1+end, 
					"www.steamcommunity.com/profiles/" ) == 0;
		} else if( end == 27 ) {
			return str.compare( 0, 1+end, 
					"steamcommunity.com/profiles/" ) == 0;
		}
		return false;
	}
};

#endif
