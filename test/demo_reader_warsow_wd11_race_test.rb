require File.dirname(__FILE__) + '/helper'

class DemoReaderWarsowWd11RaceTest < Test::Unit::TestCase

  version = 11

  [
    %w(race_cwrace5 cwrace5),
    %w(race_killua-hykon killua-hykon),
    %w(race_st1_07.848 st1 00:07.848),
    %w(race_wdm16_unfinished wdm16),
    %w(race_bardok-lick-revamped_54.882 bardok-lick-revamped 00:54.882),
    %w(race_wrc03-3_28.684 wrc03-3 00:28.684),
    %w(race_ghost-eikram2_25.848 ghost-eikram2 00:25.848)
  ].each do |entry|

    file, map, time = entry

    define_method "test_warsow_wd#{version}_demo_#{file.gsub(/[^a-z_0-9]/, "_")}" do
      demo = DemoReader.parse("test/fixtures/warsow/wd#{version}/#{file}.wd#{version}")

      assert demo.valid
      assert_equal "race", demo.gamemode
      assert_equal version, demo.version
      assert_equal map, demo.mapname
      assert_equal time, demo.time if time
    end

  end

end

