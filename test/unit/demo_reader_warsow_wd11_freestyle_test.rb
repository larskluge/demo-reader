$LOAD_PATH << File.dirname(__FILE__) + '/..'
require "helper"

class DemoReaderWarsowWd11RaceTest < Test::Unit::TestCase

  game = "Warsow"
  gamemode = "freestyle"
  version = 11

  [
    %w(freestyle_freestyle88 freestyle_88),
    %w(race_wdm16_unfinished wdm16)
  ].each do |entry|

    file, map = entry

    define_method "test_warsow_wd#{version}_demo_#{file.gsub(/[^a-z_0-9]/, "_")}" do
      demo = DemoReader.parse("test/fixtures/warsow/wd#{version}/#{file}.wd#{version}")

      assert demo.valid
      assert_equal game, demo.game
      assert_equal gamemode, demo.gamemode
      assert_equal version, demo.version
      assert_equal map, demo.mapname
    end

  end

end

