<?php

require_once 'lib/steamid.php';

header( 'Content-Type: text/plain' );

//-----------------------------------------------------------------------------
function PrintLine( $text ) {
	echo $text . "\r\n"; 
	flush();
}

//-----------------------------------------------------------------------------
class Test {
	public $name;
	public $method;
	
	function Run() {
		// lol.
		$method = $this->method;
		return $method(); 
	}
	
	function __construct( $name, $method ) {
		$this->name = $name;
		$this->method = $method;
	}
}

//-----------------------------------------------------------------------------
final class Tests {

	public static $tests = array();
	/*
	public function __construct( $name ) {
		self::$tests[] = $this;
		$this->name = $name; 
	}*/
	
	/**
	 * Run this test return TRUE on pass FALSE on failure.
	 */
	//protected abstract function RunTest();
	/*
	public function Run() {
		PrintLine( "---" );
		PrintLine( "Running test: \"$this->name\"" );
		if( $this->RunTest() ) {
			PrintLine( "Passed." );
		} else {
			PrintLine( "*** Failed! ***" );
		}
		
	}*/
	
	public static function Run() {
		PrintLine( "---" );
		PrintLine( "Running all tests." );
		foreach( self::$tests as $test ) {
			if( $test->Run() ) {
				PrintLine( "Passed." );
			} else {
				PrintLine( "*** Failed! ***" );
			}
		}
	}
	
	public static function Add( $name, $method ) {
		self::$tests[] = new Test( $name, $method );
	}
}

//-----------------------------------------------------------------------------
Tests::Add( "Conversion Test", function() {
	
	PrintLine( "32-bit detect" );
	$steamid = SteamID::Parse( " STEAM_1:1:54499221 " );
	if( $steamid === FALSE ) return FALSE;
	PrintLine( "32-bit direct" );
	$steamid = SteamID::Parse( "STEAM_0:1:54499221", SteamID::FORMAT_32BIT );
	if( $steamid === FALSE ) return FALSE;
	PrintLine( "32-bit as 64 error" );
	$steamid = SteamID::Parse( "STEAM_1:1:54499221", SteamID::FORMAT_64BIT );
	if( $steamid !== FALSE ) return FALSE;
	
	PrintLine( "64-bit detect" );
	$steamid = SteamID::Parse( "76561198069264171" );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	
	PrintLine( "64-bit direct" );
	$steamid = SteamID::Parse( "76561198069264171", SteamID::FORMAT_64BIT );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	
	PrintLine( "64-bit errornous" );
	$steamid = SteamID::Parse( "76533611981069263334171", SteamID::FORMAT_64BIT );
	if( $steamid !== FALSE ) return FALSE;
	
	PrintLine( "64-bit as raw" );
	$steamid = SteamID::Parse( "76561198069264171", SteamID::FORMAT_RAW );
	if( $steamid !== FALSE ) return FALSE;
	
	PrintLine( "v3 detect" );
	$steamid = SteamID::Parse( "[U:1:108998443]" );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	
	PrintLine( "v3 direct" );
	$steamid = SteamID::Parse( "[U:1:108998443]", SteamID::FORMAT_V3 );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	
	PrintLine( "s32 detect" );
	$steamid = SteamID::Parse( "108998443" );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	
	PrintLine( "s32 direct" );
	$steamid = SteamID::Parse( "108998443", FORMAT_S32 );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	 
	PrintLine( "raw direct" );
	$steamid = SteamID::Parse( "108998443", FORMAT_RAW );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	
	PrintLine( "community URL 1" );
	$steamid = SteamID::Parse( "http://www.steamcommunity.com/profiles/76561198069264171" );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	PrintLine( "community URL 2" );
	$steamid = SteamID::Parse( "http://steamcommunity.com/profiles/76561198069264171" );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	PrintLine( "community URL 3" );
	$steamid = SteamID::Parse( "www.steamcommunity.com/profiles/76561198069264171" );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	PrintLine( "community URL 4" );
	$steamid = SteamID::Parse( "steamcommunity.com/profiles/76561198069264171" );
	if( $steamid === FALSE ) return FALSE;
	if( $steamid->Format( SteamID::FORMAT_32BIT ) != "STEAM_1:1:54499221" ) return FALSE;
	
	PrintLine( "random conversions..." );
	for( $i = 0; $i < 1000; $i++ ) {
		$raw = mt_rand( 0, 4294967290 );
		
		$steamid = SteamID::Parse( $raw, SteamID::FORMAT_RAW );
		
		for( $j = 0; $j < 5; $j++ ) {
			$format = mt_rand( 1, 5 );
			$formatted = $steamid->Format( $format );
			$steamid2 = SteamID::Parse( $formatted );
			if( $steamid2 === FALSE || $steamid2->Format( $format ) != $formatted ) {
				PrintLine( "failure:" );
				var_dump( $steamid );
				return FALSE;
			}
			$steamid = $steamid2;
		}
	}
	
	return TRUE;
} );
 

Tests::Run();

?>