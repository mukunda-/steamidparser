
// oh baby a testing thing

SteamID.SetParseRawDefault( true );

// ****************************************************************************
function PrintLine( text ) {
	console.log( text );  
}

// ****************************************************************************
function PrintSubTest( text ) {
	console.log( "  " + text );  
}
 
// ****************************************************************************
function Test( name, method ) {
	this.name = name;
	this.method = method; 
}

Test.prototype.Run = function() {
	return this.method();
};

// ****************************************************************************
Tests = new function() {

	this.tests = [];
	
	this.Run = function() {
		PrintLine( "*** SteamID Parser/JS Testing Suite ***" ); 
		PrintLine( "Running all tests." );
		this.tests.forEach( function ( e ) {
		
			PrintLine( e.name ); 
			if( e.Run() ) {
				PrintLine( "--- Passed. ---" );
			} else {
				PrintLine( "*** Failed! ***" );
			}
		});
	};
	
	this.Add = function( name, method ) {
		this.tests.push( new Test( name, method ) );
	}
}

// ****************************************************************************
function random( min, max ) {
	return Math.round(Math.random() * (max-min)) + min;
}

// ****************************************************************************
Tests.Add( "Conversion Test", function() {
	
	var steamid;
	var steamid2;
	
	PrintSubTest( "32-bit detect" );
	steamid = SteamID.Parse( " STEAM_1:1:54499221 " );
	if( steamid === null ) return false;
	PrintSubTest( "32-bit direct" );
	steamid = SteamID.Parse( "STEAM_0:1:54499221", SteamID.Format.STEAMID32 );
	if( steamid === null ) return false;
	PrintSubTest( "32-bit as 64 error" );
	steamid = SteamID.Parse( "STEAM_1:1:54499221", SteamID.Format.STEAMID64 );
	if( steamid !== null ) return false;
	
	PrintSubTest( "64-bit detect" );
	steamid = SteamID.Parse( "76561198069264171" );
	if( steamid === null ) return false;
	PrintSubTest( steamid.Format( SteamID.Format.STEAMID32 ) );
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "64-bit direct" );
	steamid = SteamID.Parse( "76561198069264171", SteamID.Format.STEAMID64 );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "64-bit errornous" );
	steamid = SteamID.Parse( "76533611981069263334171", SteamID.Format.STEAMID64 );
	if( steamid !== null ) return false;
	
	PrintSubTest( "64-bit as raw" );
	steamid = SteamID.Parse( "76561198069264171", SteamID.Format.RAW );
	if( steamid !== null ) return false;
	
	PrintSubTest( "v3 detect" );
	steamid = SteamID.Parse( "[U:1:108998443]" );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "v3 direct" );
	steamid = SteamID.Parse( "[U:1:108998443]", SteamID.Format.STEAMID3 );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "s32 detect" );
	steamid = SteamID.Parse( "108998443" );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "s32 direct" );
	steamid = SteamID.Parse( "108998443", SteamID.Format.S32 );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "raw direct" );
	steamid = SteamID.Parse( "108998443", SteamID.Format.RAW );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "community URL 1" );
	steamid = SteamID.Parse( "http://www.steamcommunity.com/profiles/76561198069264171" );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	PrintSubTest( "community URL 2" );
	steamid = SteamID.Parse( "http://steamcommunity.com/profiles/76561198069264171" );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	PrintSubTest( "community URL 3" );
	steamid = SteamID.Parse( "www.steamcommunity.com/profiles/76561198069264171" );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	PrintSubTest( "community URL 4" );
	steamid = SteamID.Parse( "steamcommunity.com/profiles/76561198069264171" );
	if( steamid === null ) return false;
	if( steamid.Format( SteamID.Format.STEAMID32 ) != "STEAM_1:1:54499221" ) return false;
	
	PrintSubTest( "random conversions..." );
	for( var i = 0; i < 5000; i++ ) {
		// get a 32bit unsigned value
		var raw = random( 0, 4294967295 ); 
		steamid = SteamID.Parse( raw, SteamID.Format.RAW );
		
		for( var j = 0; j < 8; j++ ) {
			var format = random( 1, 5 );
			var formatted = steamid.Format( format );
			steamid2 = SteamID.Parse( formatted );
			if( steamid2 === null || steamid2.Format( format ) != formatted ) {
				PrintSubTest( "failure:" );
				console.log( steamid );
				return false;
			}
			steamid = steamid2;
		}
	}
	return true;
});

// ****************************************************************************
Tests.Add( "Large SteamID conversions", function () {

	var a, steamid;
	for( var i = 0; i < 255; i++ ) {
	
		// get a 64bit-ish value
		a = random( 1, SteamID.MAX_VALUE-100 ); 
	
		steamid = SteamID.Parse( a, SteamID.Format.RAW );
		
		for( var j = 0; j < 8; j++ ) {
			var format = random( 1, 5 );
			var formatted = steamid.Format( format );
			if( format == SteamID.Format.S32 ) {
			
				// for S32 check for proper failure.
				if( formatted === null ) {
					if( steamid.value >= 4294967296 ) {
						
						continue;
					}
				}
			}
			
			var steamid2 = SteamID.Parse( formatted );
			if( steamid2 === null || steamid2.Format( format ) != formatted ) {
				PrintSubTest( "failure: (format="+format+")" );
				console.log( steamid );
				return false;
			}
			steamid = steamid2;
		}
	}
	
	return true;
});

// ****************************************************************************
Tests.Run();
