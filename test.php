<?php

header( "Content-Type: text/plain" );

function PrintLine( $text ) {
	echo $text . "\r\n";

}

abstract class Test {

	public static $tests = array();
	
	public $name;
	
	public function __construct( $name ) {
		self::$tests[] = $this;
		$this->name = $name;
		
	}
	
	/**
	 * Run this test return TRUE on pass FALSE on failure.
	 */
	protected abstract function RunTest();
	
	public function Run() {
		PrintLine( "---" );
		PrintLine( "Running test: \"$this->name\"" );
		if( $this->RunTest() ) {
			PrintLine( "Passed." );
		} else {
			PrintLine( "*** Failed! ***" );
		}
		
	}
	
	public static function RunAll() {
		PrintLine( "---" );
		PrintLine( "Running all tests." );
		foreach( self::$tests as $test ) {
			$test->Run();
		}
	}
}

class Test1 extends Test {
	public function __construct() {
		parent::__construct( "Conversion Test" );
	}
	
	protected function RunTest() {
		
	}
}

new Test1();

Test::Add( 

Test::RunAll();



?>