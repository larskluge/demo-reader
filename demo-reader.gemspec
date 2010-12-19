# Generated by jeweler
# DO NOT EDIT THIS FILE DIRECTLY
# Instead, edit Jeweler::Tasks in Rakefile, and run the gemspec command
# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = %q{demo-reader}
  s.version = "0.2.5"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["aekym"]
  s.date = %q{2010-12-19}
  s.description = %q{A library to read warsow demo files (.wd8, .wd9, .wd10, .wd11 files) and q3 demo files (*.dm68)}
  s.email = %q{me@aekym.com}
  s.extensions = ["ext/dm68/extconf.rb"]
  s.extra_rdoc_files = [
    "LICENSE",
     "README.rdoc"
  ]
  s.files = [
    ".gitignore",
     "LICENSE",
     "README.rdoc",
     "Rakefile",
     "VERSION",
     "demo-reader.gemspec",
     "ext/dm68/README.txt",
     "ext/dm68/common.c",
     "ext/dm68/common.h",
     "ext/dm68/dm68.c",
     "ext/dm68/dump.c",
     "ext/dm68/extconf.rb",
     "ext/dm68/huff.c",
     "ext/dm68/huff.h",
     "ext/dm68/main.c",
     "ext/dm68/main.h",
     "ext/dm68/msg.c",
     "ext/dm68/msg.h",
     "ext/dm68/parse.c",
     "lib/demo-reader.rb",
     "lib/demo_reader_defrag.rb",
     "lib/demo_reader_warsow.rb",
     "test/fixtures/defrag/dm_68/cpm/pornchronostar_mdf.cpm_00.49.216_tyaz.germany.dm_68",
     "test/fixtures/defrag/dm_68/cpm/puremotion_df.cpm_00.10.600_eS-Rody.russia.dm_68",
     "test/fixtures/defrag/dm_68/vq3/runkull2_df.vq3_01.05.904_XunderBIRD.Germany.dm_68",
     "test/fixtures/defrag/dm_68/vq3/un-dead029_df.vq3_00.16.912_uN-DeaD!WiNTeR.ru.dm_68",
     "test/fixtures/warsow/wd10/racesow_0.42.b2/dinirun2_racesow_0.42.b2.wd10",
     "test/fixtures/warsow/wd10/racesow_local/boris.wd10",
     "test/fixtures/warsow/wd10/racesow_local/die11.7.wd10",
     "test/fixtures/warsow/wd10/racesow_local/dvr_antr.wd10",
     "test/fixtures/warsow/wd10/racesow_local/e-lava.wd10",
     "test/fixtures/warsow/wd10/racesow_local/j4n12.8.wd10",
     "test/fixtures/warsow/wd10/racesow_local/terror.wd10",
     "test/fixtures/warsow/wd10/racesow_local/yescomp006-2.wd10",
     "test/fixtures/warsow/wd10/racesow_local/zugo.wd10",
     "test/fixtures/warsow/wd10/trouble_making/2.wd10",
     "test/fixtures/warsow/wd10/trouble_making/boris_cab1.wd10",
     "test/fixtures/warsow/wd10/trouble_making/very_short_demo.wd10",
     "test/fixtures/warsow/wd11/freestyle_freestyle88.wd11",
     "test/fixtures/warsow/wd11/race_bardok-lick-revamped_54.882.wd11",
     "test/fixtures/warsow/wd11/race_cwrace5.wd11",
     "test/fixtures/warsow/wd11/race_gdfcomp08_14.949.wd11",
     "test/fixtures/warsow/wd11/race_ghost-eikram2_25.848.wd11",
     "test/fixtures/warsow/wd11/race_killua-hykon.wd11",
     "test/fixtures/warsow/wd11/race_snx-10_12.996.wd11",
     "test/fixtures/warsow/wd11/race_st1_07.848.wd11",
     "test/fixtures/warsow/wd11/race_wdm16_unfinished.wd11",
     "test/fixtures/warsow/wd11/race_wrc03-3_28.684.wd11",
     "test/helper.rb",
     "test/unit/demo_reader_defrag_dm_68_cpm_test.rb",
     "test/unit/demo_reader_defrag_dm_68_vq3_test.rb",
     "test/unit/demo_reader_test.rb",
     "test/unit/demo_reader_warsow_wd10_race_test.rb",
     "test/unit/demo_reader_warsow_wd11_freestyle_test.rb",
     "test/unit/demo_reader_warsow_wd11_race_test.rb"
  ]
  s.homepage = %q{http://github.com/aekym/demo-reader}
  s.rdoc_options = ["--charset=UTF-8"]
  s.require_paths = ["lib"]
  s.rubygems_version = %q{1.3.7}
  s.summary = %q{A library to read warsow and q3 demo files}
  s.test_files = [
    "test/helper.rb",
     "test/unit/demo_reader_defrag_dm_68_cpm_test.rb",
     "test/unit/demo_reader_defrag_dm_68_vq3_test.rb",
     "test/unit/demo_reader_test.rb",
     "test/unit/demo_reader_warsow_wd10_race_test.rb",
     "test/unit/demo_reader_warsow_wd11_freestyle_test.rb",
     "test/unit/demo_reader_warsow_wd11_race_test.rb"
  ]

  if s.respond_to? :specification_version then
    current_version = Gem::Specification::CURRENT_SPECIFICATION_VERSION
    s.specification_version = 3

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
    else
    end
  else
  end
end

