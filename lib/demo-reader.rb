require "demo_reader_warsow"


class DemoReader


  # tries to parse the given demo file in order to detect a warsow demo
  #
  # @returns an instance of DemoReaderWarsow or nil on failure
  #
  def self.parse(filename)
    try_warsow(filename)
  end


  protected

  def self.try_warsow(filename)
    demo = DemoReaderWarsow.new(filename)

    demo.valid ? demo : nil
  end

end

