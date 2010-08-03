require "demo_reader_warsow"
require "demo_reader_defrag"


class DemoReader


  # tries to parse the given demo file in order to detect first a warsow,
  # then a defrag demo
  #
  # @returns an instance of DemoReaderWarsow or DemoReaderDefrag or nil on failure
  #
  def self.parse(filename)
    try_warsow(filename) || try_defrag(filename)
  end


  protected

  def self.try_warsow(filename)
    demo = DemoReaderWarsow.new(filename)

    demo.valid ? demo : nil
  end

  def self.try_defrag(filename)
    demo = DemoReaderDefrag.new(filename)

    demo.valid ? demo : nil
  end

end

