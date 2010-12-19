require "demo_reader_warsow"
require "demo_reader_defrag"
require "active_support/core_ext/hash"


class DemoReader


  # tries to parse the given demo file in order to detect first a warsow,
  # then a defrag demo
  #
  # @param filename path of the demo file to parse
  # @param opts
  #   :raise_errors if errors should raise or not, default true
  #   :hint_for_time string with a time in format m.s.ms (%d{2}\.%d{2}\.%d{3}),
  #                  default is the given filename
  #
  # @returns an instance of DemoReaderWarsow or DemoReaderDefrag or nil on failure
  #
  def self.parse(filename, opts = nil)
    opts = {} if opts.blank?
    opts.assert_valid_keys :raise_errors, :hint_for_time

    # set default options when not set
    #
    opts[:raise_errors] = true      unless opts[:raise_errors].equal?(false)
    opts[:hint_for_time] = filename unless opts[:hint_for_time].is_a?(String)

    try_warsow(filename) || try_defrag(filename, opts)
  end


  protected

  def self.try_warsow(filename)
    demo = DemoReaderWarsow.new(filename)

    demo.valid ? demo : nil
  end

  def self.try_defrag(filename, opts)
    demo = DemoReaderDefrag.new(filename, opts)

    demo.valid ? demo : nil
  end

end

