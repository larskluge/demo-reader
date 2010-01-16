require File.dirname(__FILE__) + '/helper'

class DemoReaderWarsowWd10RaceTest < Test::Unit::TestCase

  version = 10

  [
    %w(racesow_0.42.b2/dinirun2_racesow_0.42.b2 dinirun2 00:21.870),
    %w(racesow_local/boris boris_75m 00:16.336),
    %w(racesow_local/die11.7 die 00:11.737),
    %w(racesow_local/dvr_antr dvr_antr 00:09.485),
    %w(racesow_local/e-lava e-lavastore 00:06.313),
    %w(racesow_local/j4n12.8 j4n_wtf 00:12.814),
    %w(racesow_local/terror terror-box 00:18.553),
    %w(racesow_local/yescomp006-2 un-dead!020_4 00:14.971),
    %w(racesow_local/zugo zugo 00:14.894),
    %w(trouble_making/2 gu3#8-bomb 00:12.834),
    %w(trouble_making/boris_cab1 boris_cab1 16:34.514),
    %w(trouble_making/very_short_demo gu3-bored 00:02.748)
  ].each do |entry|

    file, map, time = entry

    define_method "test_warsow_wd#{version}_demo_#{file.gsub(/[^a-z_0-9]/, "_")}" do
      demo = DemoReader.parse("test/fixtures/warsow/wd#{version}/#{file}.wd#{version}")

      assert demo.valid
      assert_equal "race", demo.gamemode
      assert_equal version, demo.version
      assert_equal map, demo.mapname
      assert_equal time, demo.time
    end

  end

end

