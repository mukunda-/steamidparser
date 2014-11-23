SteamID Parser
===

This is a simple library for PHP and JavaScript to parse and format Steam IDs. 

Compatible with 32-bit PHP builds.

PHP usage example:

    require_once 'lib/steamid.php';
    
    // parse a Steam ID in "SteamID3" format.
    $steamid = SteamID::Parse( 
            "[U:1:108998443]", SteamID::FORMAT_STEAMID3 );
    
    // print it in SteamID32 format
    echo $steamid->Format( SteamID::FORMAT_STEAMID32 );
    
    // (prints "STEAM_1:1:54499221")
    

JavaScript usage example (after referencing the library):

    // parse a Steam ID in "SteamID3" format.
    var steamid = SteamID.Parse( 
            "[U:1:108998443]", SteamID.Format.STEAMID3 );
    
    // print it in SteamID32 format
    console.log( steamid.Format( SteamID.Format.STEAMID32 ) );
    
    // (prints "STEAM_1:1:54499221")
    

Supports multiple formats. You can omit the parsing format for auto-detection.

The PHP version also supports Vanity URL (Custom URL) conversion. For optimal performance, you need to set a SteamAPIKey to use.

    require_once 'lib/steamid.php';
    
    // Optional: set Steam API Key
    SteamID::SetSteamAPIKey( ... );
    
    // parse directly as a Vanity URL
    $steamid = SteamID::Parse( "prayspray", SteamID::FORMAT_VANITY );
    
    // detect a vanity URL and parse it (note parameter 3 must be set)
    $steamid = SteamID::Parse( "prayspray", SteamID::FORMAT_AUTO, true );
    
    // print it in SteamID3 format
    echo $steamid->Format( SteamID::FORMAT_STEAMID3 );
    
    // (prints "[U:1:108998443]")

Get a Steam API key from http://steamcommunity.com/dev/apikey

If you don't set a Steam API key, the program will fall back to requesting the user profile as XML and parsing the Steam ID from there. (which is much less optimal.)

Formats supported by auto detection:

- SteamID32 - "STEAM_x:y:zzzzzz"
- SteamID64 - "765xxxxxxxxxx"
- SteamID3 - "[U:1:xxxxxxx]"
- RAW format - a plain number
- RAW S32 - A RAW number that may be negative to extend a signed 32-bit bitspace.
- Vanity/Custom URL - e.g. "prayspray" (PHP only)
- Full profile URL - "http://steamcommunity.com/profiles/765xxxxxx"
- Full profile Custom URL - "http://steamcommunity.com/id/[customURL]"
    
