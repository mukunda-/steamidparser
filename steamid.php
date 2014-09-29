<?php

/*!
 * SteamID Parser
 *
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

/**
 * Exception thrown on resolution failure
 * (only used when resolving vanity URLs.)
 */
class SteamIDResolutionException extends Exception {
	const UNKNOWN = 0;            // Unknown error.
	const CURL_FAILURE = 1;       // cURL/network related error.
	const VANITYURL_NOTFOUND = 2; // The vanity URL given was invalid.
	const VANITYURL_FAILED = 3;   // Steam failure when trying to resolve vanity URL.
	
	public $reason;
	
	function __construct( $reason, $text ) {
		$this->reason = $reason;
		parent::__construct( $text );
	}
}

/** ---------------------------------------------------------------------------
 * SteamID
 *
 * Contains a User Steam ID.
 *
 * @author Mukunda Johnson
 */
class SteamID {
	
	private $value;     // RAW value as a string.
	private $converted; // array of converted values.
	
	const FORMAT_AUTO  = 0; // Auto-detect format --- this also supports other
							// unlisted formats such as profile URLs.
	const FORMAT_32BIT = 1; // Classic STEAM_x:y:zzzzzz | x = 0/1
	const FORMAT_64BIT = 2; // SteamID64: 7656119xxxxxxxxxx
	const FORMAT_V3    = 3; // SteamID3 format: [U:1:xxxxxx]
	const FORMAT_S32   = 4; // Raw 32-bit SIGNED format. 
							// this is a raw steamid index that overflows
							// into negative bitspace.
	const FORMAT_RAW   = 5; // Raw index. like 64-bit minus the base value.
	
	
	private STEAMID64_BASE = '76561197960265728';
	
	// max allowed value. (sanity check)
	// 2^36; update this in approx 2,400,000 years
	const MAX_VALUE = '68719476736';
	
	private static steam_api_key = FALSE;
	
	/** -----------------------------------------------------------------------
	 * Set an API key to use for resolving Custom URLs. If this isn't set
	 * custom URL resolution will be done by parsing the profile XML.
	 *
	 * @param string $key API Key
	 * @see http://steamcommunity.com/dev/apikey
	 */
	public static function SetSteamAPIKey( $key ) {
		if( empty($key) ) $steam_api_key = FALSE;
		$steam_api_key = $key;
	}
	
	/** -----------------------------------------------------------------------
	 * Construct an instance. 
	 *
	 * @param string $raw Raw value of Steam ID.
	 */
	private __construct( $raw ) {
		$this->value = $raw;
		$this->converted[ FORMAT_RAW ] = $raw;
	}
	
	/** -----------------------------------------------------------------------
	 * Make a cURL request and return the contents.
	 *
	 * @param string $url URL to request.
	 * @return string|false Contents of result or FALSE if the request failed.
	 */
	private function Curl( $url ) {
		
		$ch = curl_init(); 
		curl_setopt( $ch, CURLOPT_URL, $url );
		curl_setopt( $ch, CURLOPT_RETURNTRANSFER, 1 );
		curl_setopt( $ch, CURLOPT_FOLLOWLOCATION, 1 );
		
		$data = curl_exec($ch);  
		curl_close($ch);

		return $data;
	}
	
	/** -----------------------------------------------------------------------
	 * Parse a Steam ID.
	 *
	 * @param string $input Input to parse.
	 * @param int $format   Input formatting, see FORMAT_ constants.
	 *                      Defaults to FORMAT_AUTO which detects the format.  
	 *
	 * @return SteamID|false SteamID instance or FALSE if the input is invalid 
	 *                       or unsupported.
	 */
	public static function Parse( $input, $format = FORMAT_AUTO ) {
		if( $format == FORMAT_32BIT ) {
		
			// validate STEAM_0/1:y:zzzzzz
			if( !preg_match( 
					'/^STEAM_[0-1]:([0-1]):([0-9]+)$/', 
					$input, $matches ) ) {
					
				return FALSE;
			}
			
			// convert to raw.
			$a = bcmul( $matches[2], 2 );
			$a = bcadd( $a, $matches[1] );
			
			$result = new self( $a );
			$result->converted[ self::FORMAT_32BIT ] = $input;
			return $result;
		} else if( $format == self::FORMAT_64BIT ) {
		
			// allow digits only
			if( !preg_match( '/^[0-9]+$/', $input ) ) return FALSE;
			
			// convert to raw (subtract base)
			$a = bcsub( $input, self::STEAMID64_BASE );
			
			// sanity range check.
			if( bccomp( $a, 0 ) < 0 ) return FALSE;
			if( bccomp( $a, self::MAX_VALUE ) > 0 ) return FALSE
			
			$result = new self( $a );
			$result->converted[ self::FORMAT_64BIT ] = $input;
			return $result;
		} else if( $format == self::FORMAT_V3 ) {
		
			// validate [U:1:xxxxxx]
			if( !preg_match( '/^[U:1:([0-9]+)]$/', $input, $matches ) ) {
				return FALSE;
			}
			
			$a = $matches[1];
			
			// sanity range check.
			if( bccomp( $a, self::MAX_VALUE ) > 0 ) return FALSE;
			$result = new self( $a );
			$result->converted[ self::FORMAT_V3 ] = $input;
			return $result;
		} else if( $format == self::FORMAT_S32 ) {
			
			// validate signed 32-bit format
			if( !preg_match( '/^(-?[0-9]+)$/', $input ) ) {
				return FALSE;
			}
			
			$a = $input;
			
			// 32-bit range check
			if( bccomp( $a, '2147483647' ) > 0 ) return FALSE;
			if( bccomp( $a, '-2147483648' ) < 0 ) return FALSE;
			if( bccomp( $a, '0' ) < 0 ) {
				$a = bcadd( $a, '4294967296' );
			}
			$result = new self( $a );
			$result->converted[ self::FORMAT_S32 ] = $input;
			return $result;
		} else if( $format == self::FORMAT_RAW ) {
		
			// validate digits only
			if( !preg_match( '/^[0-9]+$/', $input ) ) {
				return FALSE;
			}
			
			// sanity range check
			if( bccomp( $input, self::MAX_VALUE ) > 0 ) return FALSE;
			return new self( $input );
		}
		
		// Auto detect format:
		
		$input = trim( $input );
		$result = self::Parse( $input, self::FORMAT_32BIT );
		if( $result !== FALSE ) return $result;
		$result = self::Parse( $input, self::FORMAT_64BIT );
		if( $result !== FALSE ) return $result;
		$result = self::Parse( $input, self::FORMAT_V3 );
		if( $result !== FALSE ) return $result;
		$result = self::Parse( $input, self::FORMAT_S32 );
		if( $result !== FALSE ) return $result;
		$result = self::Parse( $input, self::FORMAT_RAW );
		if( $result !== FALSE ) return $result;
		
		if( preg_match( 
				'/^(?:https?:\/\/)?(?:www.)?steamcommunity.com/profiles/([0-9]+)$/',
				$input, $matches ) ) {
			$result = self::Parse( $matches[1], self::FORMAT_64BIT );
			if( $result !== FALSE ) return $result;
		}
		
		// TODO find out what characters are valid in customURLs.
		if( preg_match( 
				'/^(?:https?:\/\/)?(?:www.)?steamcommunity.com/id/([a-zA-Z0-9]+)$/',
				$input, $matches ) ) {
				
			$result = self::ConvertVanityURL( $matches[1] );
			if( $result !== FALSE ) return $result;
		}
		
		// unknown stem
		return FALSE;
	}
	
	/** ----------------------------------------------------------------------- 
	 * Convert a vanity URL into a SteamID instance.
	 *
	 * @param string $vanity_url_name The text part of the person's vanity URL.
	 *                     e.g http://steamcommunity.com/id/gabelogannewell 
	 *                     would use "gabelogannewell" 
	 * @return SteamID|false SteamID instance or FALSE on failure.
	 */
	public static function ConvertVanityURL( $vanity_url_name ) {
		if( empty($vanity_url_name) ) return FALSE;
		
		if( $steam_api_key !== FALSE ) {
			$result = Curl( 
				"http://api.steampowered.com/ISteamUser/ResolveVanityURL/v0001/?key=$steam_api_key&vanityurl=$vanity_url_name" );
			if( $result === FALSE ) {
				throw new SteamIDResolutionException( 
						SteamIDResolutionException::CURL_FAILURE,
						'CURL Request Failed.' );
			}
			
			if( $response == "" ) {
				throw new SteamIDResolutionException( 
						SteamIDResolutionException::VANITYURL_FAILED,
						'Steam failure.' );
			}
			
			$response = json_decode( $response );
			if( $response === FALSE ) {
				throw new SteamIDResolutionException( 
						SteamIDResolutionException::VANITYURL_FAILED,
						'Steam failure.' );
			}
			
			if( $response->success == 42 ) {
				throw new SteamIDResolutionException( 
						SteamIDResolutionException::VANITYURL_NOTFOUND,
						'Vanity URL doesn\'t exist.' );
				
			}
			
			if( $response->success != 1 ) {
				throw new SteamIDResolutionException( 
						SteamIDResolutionException::VANITYURL_FAILED,
						'Steam failure.' );
				
			}
			
			$steamid = $response->steamid;
			
		} else {
			// fallback to xml parsing method.
			
			$result = Curl( 
				"http://api.steampowered.com/ISteamUser/ResolveVanityURL/v0001/?key=$steam_api_key&vanityurl=$vanity_url_name" );
			if( $result === FALSE ) {
				throw new SteamIDResolutionException( 
						SteamIDResolutionException::CURL_FAILURE,
						'CURL Request Failed.' );
			}
			
			$parser = xml_parser_create('');
			$values = array();
			$indexes = array();
			xml_parse_into_struct( $parser, $result, $values, $indexes );
			xml_parser_free($parser);
			$steamid = $indexes['STEAMID64'];
			if( is_null( $steamid ) ) {
				throw new SteamIDResolutionException( 
						SteamIDResolutionException::VANITYURL_FAILED,
						'Invalid Vanity URL or Steam failure.' );
			}
			$steamid = $values[ $steamid[0] ]['value'];
		}
		
		return Parse( $steamid, 
	}
}

?>