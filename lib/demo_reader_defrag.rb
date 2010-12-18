require 'yaml'
require 'dm68'
require 'active_support/core_ext/object'

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

    @scoreboards = @raw['prints']
    @basegamedir = @raw['server_info']['gamename']
    @gamedir = @raw['system_info']['fs_game']

    if @raw['server_info']['defrag_vers'].to_i > 0
      @gamemode = @raw['server_info']['df_promode'].to_i.zero? ? 'vq3' : 'cpm'

      time_hint = extract_time_from_filename(@filename)
      @time = extract_time(@raw['prints'], time_hint)

      @player = extract_player(@raw['prints'], @time)
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

  def guessable_time(time)
    time = time.sub(/^[0:\.]*/, "") # remove empty min, sec, etc
    parts = time.split(/\.|:/).map(&:to_i)

    format_str = case parts.size
    when 3 then "%d:%02d:%03d"
    when 2 then "%02d:%03d"
    end

    format_str % parts
  end

  # extract player from server prints
  # supports only one player atm otherwise raises an exception
  #
  def extract_player(prints, time)
    time = guessable_time(time)

    players = prints.inject([]) do |arr,p|
      arr << $1 if p =~ /^(.+)\^7 reached the finish line in \^\d#{time}/
      arr << $1 if p =~ /^Time performed by (.+)\^7 : \^\d#{time}/
      arr
    end

    raise "No player was found." if players.empty?
    raise "Not only one player was found #{players.inspect}." if players.size > 1

    players.first
  end


  def extract_time_from_filename(filename)
    $1 if File.basename(filename) =~ /(\d{2}\.\d{2}\.\d{3})/
  end

  # extract time from server prints
  # supports only one time atm otherwise raises an exception
  #
  def extract_time(prints, time_hint)
    times = prints.inject([]) do |arr,p|
      arr << normalize_time(plain_text($1)) if p =~ /(\^[0-9]+:[\^:0-9]+)/
      arr
    end

    raise "No time was found." if times.empty?
    return times.first if times.one?

    # multiple times found
    # try to guess time by hint
    #
    time_hint = normalize_time(time_hint) if time_hint.present?
    return time_hint if times.member?(time_hint)

    raise "Not only one time was found #{times.inspect}."
  end

  def plain_text(text)
    text.gsub(/\^\d/, '')
  end

  def normalize_time(time_str)
    parts = time_str.split(/\.|:/)
    parts.unshift(0) if parts.size < 3

    "%02d:%02d.%03d" % parts.map(&:to_i)
  end
end

