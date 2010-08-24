require 'rubygems'
require 'test/unit'

$LOAD_PATH.unshift(File.dirname(__FILE__))
$LOAD_PATH.unshift(File.join(File.dirname(__FILE__), '..', 'lib'))
$LOAD_PATH.unshift(File.expand_path(File.join(File.dirname(__FILE__), '..', 'ext/dm68')))

require 'demo-reader'

class Test::Unit::TestCase
end

