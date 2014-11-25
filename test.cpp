
// oh baby a testing thing

#include <iostream>
#include <chrono>

#include "lib/steamid.hpp"

// ****************************************************************************
void PrintLine( const std::string &text ) {
	std::cout << text << std::endl;
}

// ****************************************************************************
void PrintSubTest( const std::string &text ) {
	std::cout << "  " << text << std::endl;
}


// ****************************************************************************
class Test {

public:
	std::string m_name;
	std::function<bool()> m_method;
	
	bool operator()() {
		return m_method();
	}
	
	Test( const std::string &name, std::function<bool()> method ) {
		m_name = name;
		m_method = method;
	}
};

// ****************************************************************************
class Tests {

	std::vector<Test> tests;

public:
	void Run() {
		PrintLine( "---" );
		PrintLine( "Running all tests." );

		auto begin = std::chrono::high_resolution_clock::now();
		for( auto & test: tests ) {
			PrintLine( "" );
			std::cout << '"' << test.m_name << '"' << std::endl;
			
			if( test() ) {
				PrintLine( "--- Passed. ---" );
			} else {
				PrintLine( "*** Failed! ***" );
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << (double)(std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count())/1000.0 << "ms" << std::endl;
	}
	
	Tests& operator <<( Test& test ) {
		tests.push_back(test);
		return *this;
	}
};

// ****************************************************************************
long long random( long long min, long long max ) {
	double a = rand();
	a = a * (double)(max-min);
	a = a / (double)RAND_MAX;

	return min + (long long)floor(a+0.5);
}

void RunTests() {

	Tests tests;

	tests
	// ************************************************************************
	<< Test( "Conversion Test", [] {
	
		SteamID steamid;
		PrintSubTest( "32-bit detect" );
		steamid = SteamID::Parse( " STEAM_1:1:54499221 " );
		if( steamid.Empty() ) return false;
		PrintSubTest( "32-bit direct" );
		steamid = SteamID::Parse( "STEAM_0:1:54499221", SteamID::Formats::STEAMID32 );
		if( steamid.Empty() ) return false;
		PrintSubTest( "32-bit as 64 error" );
		steamid = SteamID::Parse( "STEAM_1:1:54499221", SteamID::Formats::STEAMID64 );
		if( *steamid ) return false;
	
		PrintSubTest( "64-bit detect" );
		steamid = SteamID::Parse( "76561198069264171" );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "64-bit direct" );
		steamid = SteamID::Parse( "76561198069264171", SteamID::Formats::STEAMID64 );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "64-bit errornous" );
		steamid = SteamID::Parse( "76533611981069263334171", SteamID::Formats::STEAMID64 );
		if( *steamid ) return false;
	
		PrintSubTest( "64-bit as raw" );
		steamid = SteamID::Parse( "76561198069264171", SteamID::Formats::RAW );
		if( *steamid ) return false;
	
		PrintSubTest( "v3 detect" );
		steamid = SteamID::Parse( "[U:1:108998443]" );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "v3 direct" );
		steamid = SteamID::Parse( "[U:1:108998443]", SteamID::Formats::STEAMID3 );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "s32 detect" );
		steamid = SteamID::Parse( "108998443" );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "s32 direct" );
		steamid = SteamID::Parse( "108998443", SteamID::Formats::S32 );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "raw direct" );
		steamid = SteamID::Parse( "108998443", SteamID::Formats::RAW );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "community URL 1" );
		steamid = SteamID::Parse( "http://www.steamcommunity.com/profiles/76561198069264171" );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
		PrintSubTest( "community URL 2" );
		steamid = SteamID::Parse( "http://steamcommunity.com/profiles/76561198069264171" );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
		PrintSubTest( "community URL 3" );
		steamid = SteamID::Parse( "www.steamcommunity.com/profiles/76561198069264171" );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
		PrintSubTest( "community URL 4" );
		steamid = SteamID::Parse( "steamcommunity.com/profiles/76561198069264171" );
		if( steamid.Empty() ) return false;
		if( steamid[ SteamID::Formats::STEAMID32 ] != "STEAM_1:1:54499221" ) return false;
	
		PrintSubTest( "random conversions..." );
		for( int i = 0; i < 5000; i++ ) {
			// get a 32bit unsigned value
			long long raw = random( 0, 4294967295 );
			
			steamid = SteamID::Parse( std::to_string(raw), SteamID::Formats::RAW );
		
			for( int j = 0; j < 8; j++ ) {
				SteamID::Formats format = (SteamID::Formats)random( 1, 5 );
				std::string formatted = steamid[ format ];
				SteamID steamid2 = SteamID::Parse( formatted );
				if( steamid2.Empty() || steamid2.Format( format ) != formatted ) {
					PrintSubTest( "failure:" );
					std::cout << *steamid; 
					return false;
				}
				steamid = steamid2;
			}
		}
	
		return true;
	})

	// ************************************************************************
	<< Test( "Large SteamID conversions", [] {

		for( int i = 0; i < 255; i++ ) {
			// get a 64bit-ish value
			long long a = random( 1, SteamID::MAX_VALUE );
	
			SteamID steamid = SteamID::Parse( std::to_string(a), SteamID::Formats::RAW );
		
			for( int j = 0; j < 8; j++ ) {
				SteamID::Formats format = (SteamID::Formats)random(1,5);
				std::string formatted = steamid[ format ];
				if( format == SteamID::Formats::S32 ) {
			
					// for S32 check for proper failure.
					if( formatted == "" ) {
						if( *steamid >= 4294967296L ) {
						
							continue;
						}
					}
				}

				SteamID steamid2 = SteamID::Parse( formatted );
				if( !steamid2 || steamid2[ format ] != formatted ) {
					PrintSubTest( "failure: (format=format)" );
					std::cout << *steamid;
					
					return false;
				}
				steamid = steamid2;
			}
		}
	
		return true;
	});
	// ************************************************************************
	
	SteamID::ParseRawDefault( true );

	try { 
		tests.Run();
	} catch( std::exception &e ) {
		std::cout << "Exception: " << e.what() << std::endl;
	} 
	
}


int main() {
	RunTests();
	getchar();
}