$LOAD_PATH << File.dirname(__FILE__) + '/..'
require "helper"

class DemoReaderTest < Test::Unit::TestCase

  def test_interface
    assert DemoReader.respond_to?(:parse)
  end

end

