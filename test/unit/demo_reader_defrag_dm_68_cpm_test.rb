$LOAD_PATH << File.dirname(__FILE__) + '/..'
require "helper"


class DemoReaderDefragDm68CpmTest < Test::Unit::TestCase

  game = "Defrag"
  version = 68
  gamemode = "cpm"

  [
    %w(pornchronostar_mdf.cpm_00.49.216_tyaz.germany pornchronostar 00:49.216 *tyaz*),
    %w(puremotion_df.cpm_00.10.600_eS-Rody.russia puremotion 00:10.600 ^2eS-Rody),
    %w(mega_wood[mdf.cpm]_without_time_hint_\(kreator.Germany\) mega_wood 00:14.752 r^8l^7x^0|^7Kr^8e^7ator here is the time hint: 00.14.752)
  ].each do |entry|

    file, map, time, player = entry
    time_hint = entry.last if entry.size > 4

    define_method "test_defrag_dm_#{version}_demo_#{file.gsub(/[^a-z_0-9]/, "_")}" do
      demo = DemoReader.parse("test/fixtures/defrag/dm_#{version}/#{gamemode}/#{file}.dm_#{version}", time_hint)

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

