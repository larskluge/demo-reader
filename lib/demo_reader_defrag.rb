require 'yaml'
require 'dm68'

class DemoReaderDefrag
  attr_reader :filename, :version, :mapname, :time, :playernames, :scoreboards, :game, :gamemode, :player, :basegamedir, :gamedir, :valid



  def initialize(filename)
    @filename = filename

    @game = "Defrag"
    @version = -1
    @mapname = nil
    @time = nil
    @time_in_msec = nil
    @playernames = []
    @scoreboards = []
    @gamemode = nil
    @player = nil
    @basegamedir = nil
    @gamedir = nil
    @valid = false
    @raw = nil

    self.init()
  end



  def init
    out = DM68.parse_file(@filename)
    @raw = YAML.load(out)

    raise out unless @raw

    @version = @raw['server_info']['protocol'].to_i
    @mapname = @raw['server_info']['mapname'].downcase

    # @playernames = []
    @scoreboards = @raw['prints']
    @basegamedir = @raw['server_info']['gamename']
    @gamedir = @raw['system_info']['fs_game']

    if @raw['server_info']['defrag_vers'].to_i > 0
      @gamemode = @raw['server_info']['df_promode'].to_i.zero? ? 'vq3' : 'cpm'
      @time = extract_time(@raw['prints'])
      @player = extract_player(@raw['prints'])
      @playernames << @player # just support one player atm
    end

    @valid = true
  end



  def time_in_msec
    return @time_in_msec unless @time_in_msec.nil?

    # time str to int
    if @time.kind_of? String
      min, sec, msec = @time.scan(/^([0-9]+):([0-9]+)\.([0-9]+)$/).flatten.map { |x| x.to_i }
      @time_in_msec = msec + sec * 1000 + min * 60 * 1000
    end
  end



  protected

  # extract player from server prints
  # supports only one player atm otherwise raises an exception
  #
  def extract_player(prints)
    players = prints.inject([]) do |arr,p|
      arr << $1 if p =~ /^(.+)\^7 reached the finish line/
      arr << $1 if p =~ /^Time performed by (.+)\^7 :/
      arr
    end

    raise "Not only one player was found #{players.inspect}." if players.length != 1

    players.first
  end

  # extract time from server prints
  # supports only one time atm otherwise raises an exception
  #
  def extract_time(prints)
    times = prints.inject([]) do |arr,p|
      arr << $1 if p =~ /(\^[0-9]+:[\^:0-9]+)/
      arr
    end

    raise "Not only one time was found #{times.inspect}." if times.length != 1

    time = plain_text(times.first)

    time = "0:#{time}" if time.count(":") < 2
    "%02d:%02d.%03d" % time.split(":").map(&:to_i)
  end

  def plain_text(text)
    text.gsub(/\^\d/, '')
  end
end

