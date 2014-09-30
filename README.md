SteamID Parser
===

This is a simple PHP library to parse and format Steam IDs.

Usage example:

    require_once 'lib/steamid.php';
    
    // parse a Steam ID in "SteamID3" format.
    $steamid = SteamID::Parse( 
            "[U:1:108998443]", SteamID::FORMAT_STEAMID3 );
    
    // print it in SteamID32 format
    echo $steamid->Format( SteamID::FORMAT_STEAMID32 );
    
    // (prints "STEAM_1:1:54499221")

Supports multiple formats. You can omit the parsing format for auto-detection.
