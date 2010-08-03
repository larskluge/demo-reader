require File.dirname(__FILE__) + '/helper'

class DemoReaderDefragDm68Vq3Test < Test::Unit::TestCase

  game = "Defrag"
  version = 68
  gamemode = "vq3"

  [
    %w(runkull2_df.vq3_01.05.904_XunderBIRD.Germany runkull2 01:05.904 ^2XunderBIRD),
    %w(un-dead029_df.vq3_00.16.912_uN-DeaD!WiNTeR.ru un-dead029 00:16.912 ^2uN-DeaD!WiNTeR)
  ].each do |entry|

    file, map, time, player = entry

    define_method "test_warsow_wd#{version}_demo_#{file.gsub(/[^a-z_0-9]/, "_")}" do
      demo = DemoReader.parse("test/fixtures/defrag/dm_#{version}/#{gamemode}/#{file}.dm_#{version}")

      assert demo.valid
      assert_equal game, demo.game
      assert_equal gamemode, demo.gamemode
      assert_equal version, demo.version
      assert_equal map, demo.mapname
      assert_equal player, demo.player
      assert_equal time, demo.time
    end

  end

end

