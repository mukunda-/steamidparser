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
 
/** ---------------------------------------------------------------------------
 * SteamID
 *
 * Contains a User Steam ID.
 *
 * @param string raw_value Raw value of Steam ID.
 *
 * @author Mukunda Johnson
 */
window.SteamID = function( raw ) {
	// RAW Steam ID value.
	this.value = raw;
	
	// Array of converted values. Indexed by FORMAT_xxx
	// this is a cache of formatted values, filled in
	// by Format or Parse.
	this.formatted = {};
	
	this.formatted[SteamID.Format.RAW] = raw;
};

/** ---------------------------------------------------------------------------
 * Parsable formats.
 */
SteamID.Format = {
	AUTO: 0,		// Auto-detect format --- this also supports 
					// other unlisted formats such as 
					// full profile URLs.
	STEAMID32: 1,	// Classic STEAM_x:y:zzzzzz | x = 0/1
	STEAMID64: 2,	// SteamID64: 7656119xxxxxxxxxx
	STEAMID3: 3,	// SteamID3 format: [U:1:xxxxxx]
	S32: 4,			// Raw 32-bit SIGNED format. 
					// this is a raw steamid index that overflows
					// into negative bitspace.
					// This is the format that SourceMod returns
					// with GetSteamAccountID, and will always
					// fit into a 32-bit signed variable. (e.g.
					// a 32-bit PHP integer).
	RAW: 5			// Raw index. like 64-bit minus the base value.
};

//-----------------------------------------------------------------------------

// The base constant for 64-bit steam IDs.	
SteamID.STEAMID64_BASE  = 76561197960265728;

// a shorter version because javascript numbers cannot operate
// on the normal base precisely.
SteamID.STEAMID64_SBASE =     1197960265728;
SteamID.STEAMID64_SBASE_PREFIX = "7656";

// max allowed value. (sanity check)
// 2^36; update this in approx 2,400,000 years
SteamID.MAX_VALUE = 68719476736;

SteamID.default_detect_raw = false;

/** ---------------------------------------------------------------------------
 * Set the default setting for $detect_raw for Parse()
 *
 * @param bool parseraw Default detect_raw value, see Parse function.
 */
SteamID.SetParseRawDefault = function( parseraw ) {
	this.default_detect_raw = true;
};
 
/** ---------------------------------------------------------------------------
 * Parse a Steam ID.
 *
 * @param string input  Input to parse.
 * @param int format    Input formatting, see SteamID.Format constants.
 *                      Defaults to Format.AUTO which detects the format.  
 * 
 * @param bool detect_raw 
 *                      Detect and parse RAW values. Only used with
 *                      Format.AUTO. e.g "123" will resolve to the
 *                      SteamID with the raw value 123. 
 *                      Default option set with SetParseRawDefault.
 *
 * @returns SteamID|null SteamID instance or null if the input is invalid
 *                       or unsupported.
 */
SteamID.Parse = function( input, format, detect_raw ) {
	
	if( typeof input === "number" ) input = input.toString();
	if( typeof format === "undefined" ) format = this.Format.AUTO;
	
	if( typeof detect_raw === "undefined" )
			detect_raw = this.default_detect_raw;
	
	switch( format ) {
		case this.Format.STEAMID32:
			
			// validate STEAM_0/1:y:zzzzzz
			var matches = input.match( /^STEAM_[0-1]:([0-1]):([0-9]+)$/ );
			if( !matches ) return null;
			
			// convert to raw.
			var raw = parseInt(matches[2]) * 2 + parseInt(matches[1]);
			
			var result = new this( raw );
			result.formatted[ this.Format.STEAMID32 ] = input;
			return result;
			
		case this.Format.STEAMID64:
			// allow digits only
			if( !input.match( /^7656[0-9]+$/ ) ) return null;
			
			// convert to raw (subtract base)
			var raw = parseInt(input.substring(4)) - this.STEAMID64_SBASE;
			
			// sanity range check.
			if( raw < 0 || raw > this.MAX_VALUE ) return null;
			
			var result = new this( raw );
			result.formatted[ this.Format.STEAMID64 ] = input;
			return result;
			
		case this.Format.STEAMID3:
		
			// validate [U:1:xxxxxx]
			var matches = input.match( /^\[U:1:([0-9]+)\]$/ );
			if( !matches ) return null;
			var raw = parseInt(matches[1]);
			
			// sanity range check.
			if( raw > this.MAX_VALUE ) return null;
			
			var result = new this( raw );
			result.formatted[ this.Format.STEAMID3 ] = input;
			return result;
			
		case this.Format.S32:
			
			// validate signed 32-bit format
			if( !input.match( /^(-?[0-9]+)$/ ) ) return null;
			var raw = parseInt(input);
			
			// 32-bit range check
			if( raw > 2147483647 || raw < -2147483648 ) return null;
			if( raw < 0 ) raw += 4294967296;
			
			var result = new this( raw );
			result.formatted[ this.Format.FORMAT_S32 ] = raw;
			return result;
			
		case this.Format.RAW:
			
			// validate digits only
			if( !input.match( /^[0-9]+$/ ) ) return null;
			var raw = parseInt(input);
			
			// sanity range check
			if( raw > this.MAX_VALUE ) return null;
			return new this( raw );
	}
	
	// Auto detect format:
	input = input.trim();
	
	var result = this.Parse( input, this.Format.STEAMID32 );
	if( result !== null ) return result;
	var result = this.Parse( input, this.Format.STEAMID64 );
	if( result !== null ) return result;
	var result = this.Parse( input, this.Format.STEAMID3 );
	if( result !== null ) return result;
	
	var matches = input.match( /^(?:https?:\/\/)?(?:www.)?steamcommunity.com\/profiles\/([0-9]+)$/ );
	if( matches ) {
		result = this.Parse( matches[1], this.Format.STEAMID64 );
		if( result !== null ) return result;
	}
	
	if( detect_raw ) {
		result = this.Parse( input, this.Format.S32 );
		if( result !== null ) return result;
		result = this.Parse( input, this.Format.RAW );
		if( result !== null ) return result;
	}
	
	// unknown stem
	return null;
};

/** ----------------------------------------------------------------------- 
 * Format this SteamID to a string.
 *
 * @param int format Output format. See FORMAT_xxx constants.
 * @return string|null  Formatted Steam ID. null if an invalid format is
 *                      given or the desired format cannot contain the 
 *                      SteamID.
 */
SteamID.prototype.Format = function( format ) {

	if( this.formatted.hasOwnProperty( format ) ) {
		return this.formatted[format];
	}
	
	switch( format ) {
		case SteamID.Format.STEAMID32: 
			var z = Math.floor(this.value / 2);
			var y = this.value & 1;
			var formatted = "STEAM_1:" + y + ":" + z;
			this.formatted[format] = formatted;
			return formatted;
			
		case SteamID.Format.STEAMID64:
			var formatted = SteamID.STEAMID64_SBASE_PREFIX + 
							(this.value + SteamID.STEAMID64_SBASE).toString();
			this.formatted[format] = formatted;
			return formatted;
			
		case SteamID.Format.STEAMID3:
			var formatted = "[U:1:" + this.value + "]";
			this.formatted[format] = formatted;
			return formatted;
			
		case SteamID.Format.S32:
			if( this.value >= 4294967296 ) return null;
			
			if( this.value >= 2147483648 ) {
				var formatted = this.value - 4294967296;
			} else {
				var formatted = this.value;
			}
			formatted = formatted.toString();
			this.formatted[format] = formatted;
			return formatted;
	}
	return null;
};
//
