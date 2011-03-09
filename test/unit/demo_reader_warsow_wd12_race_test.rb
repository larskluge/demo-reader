$LOAD_PATH << File.dirname(__FILE__) + '/..'
require "helper"

class DemoReaderWarsowWd12RaceTest < Test::Unit::TestCase

  game = "Warsow"
  gamemode = "race"
  version = 12

  [
    %w(freestyle_wctf1 wctf1),
    %w(race_bardok-w3sp_18.000 bardok-w3sp 00:18.000),
    %w(race_bardok-egypt_10.690 bardok-egypt 00:10.690),
    %w(race_cjscomp008 cjscomp008 00:17.932),
  ].each do |entry|

    file, map, time = entry

    define_method "test_warsow_wd#{version}_demo_#{file.gsub(/[^a-z_0-9]/, "_")}" do
      demo = DemoReader.parse("test/fixtures/warsow/wd#{version}/#{file}.wd#{version}")

      assert demo.valid
      assert_equal version, demo.version
      assert_equal game, demo.game
      assert_equal gamemode, demo.gamemode
      assert_equal map, demo.mapname
      assert_equal time, demo.time if time
    end

  end

end

