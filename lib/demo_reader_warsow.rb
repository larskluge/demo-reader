class DemoReaderWarsow
  attr_reader :filename, :version, :mapname, :time, :playernames, :scoreboards, :game, :gamemode, :player, :basegamedir, :gamedir, :valid


  def initialize(filename)
    @filename = filename
    @game = "Warsow"
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

    self.init()
  end



  def init
    file = File.new(@filename, 'r')

    return if file.stat.size < 10

    @version = detect_version(file)

    # just support some wd* file versions
    return unless [8, 9, 10, 11].include? @version

    # skip 4 bytes "svs.spawncount"
    file.pos += 4

    # skip 2 bytes "cl.snapFrameTime"
    file.pos += 2 if [10, 11].include? @version

    # skip 4 bytes "cl.baseServerTime"
    file.pos += 4 if @version == 10


    # FS_BaseGameDirectory
    @basegamedir = file.gets("\0").chop

    # FS_GameDirectory
    @gamedir = file.gets("\0").chop

    # skip short "playernum"
    file.pos += 2


    # mapname
    @mapname = file.gets("\0").chop.downcase

    content = file.gets nil
    file.close




    # detect scoreboard

    regex = /scb \"([^\"]+)/
    matchdata = regex.match(content)

    while matchdata
      @scoreboards.push matchdata[1]
      matchdata = regex.match(matchdata.post_match)
    end



    # detect game mode

    if @version == 11
      @gamemode = gamemode_wd11(content)
    else
      regex = /&([^ ]+) /

      gamemodes = []
      @scoreboards.each do |scb|
        matchdata = regex.match(scb)
        gamemodes.push matchdata[1]
      end
      gamemodes.uniq!
      if gamemodes.length.zero?
        @gamemode = 'unknown'
      else
        if gamemodes.length == 1
          @gamemode = gamemodes.first
          @gamemode.chop! if ['races', 'dms', 'ctfs'].include? @gamemode
        else
          @gamemode = "multiple gamemodes found: #{gamemodes.join(', ')}!"
        end
      end
    end



    # detect time by sent message string with time from server
    if ['race', 'unknown'].include?(@gamemode)
      matches = []
      regex = /(server record|race finished)(?:!.*(?:current|times\^7 ))?:[0-9:\. ]* (\d+):(\d+)\.(\d+)/im
      matchdata = regex.match(content)

      while matchdata
        matches.push("%02d:%02d.%03d" % [$2,$3,$4].map(&:to_i))
        matchdata = regex.match(matchdata.post_match)
      end

      if matches.length > 0
        @time = matches.sort.first
        @gamemode = 'race' if @gamemode == 'unknown'
      end
    end



    #detect all player names
    matches = []
    regex = /cs ([0-9]+) \"\\name\\([^\0]*)\\hand/
    rest_content = content
    matchdata = regex.match(rest_content)
    first_no = matchdata[1].to_i

    while matchdata
      # damit werden doppelte eintraege durch den letzten aktuellen ueberschrieben
      matches[matchdata[1].to_i - first_no] = matchdata[2]
      rest_content = matchdata.post_match
      matchdata = regex.match(rest_content)
    end

    if matches.length > 0
      # save player names only
      @playernames = []
      matches.each_with_index { |player, number|
        #playernames.push [number, player].join(': ')
        @playernames[number] = player
      }
    end



    # detect player

    playernames = @playernames.compact.sort.uniq
    if playernames.length == 1
      @player = playernames.first
    else
      if @time && @gamemode == 'race' && !@scoreboards.empty? && !playernames.empty?
        min, sec, msec = @time.scan(/^([0-9]+):([0-9]+)\.([0-9]+)$/).flatten.map { |x| x.to_i }
        min = 9 if min > 9                                                      # scoreboard does not support race times > 9:55:999; max minute value is 9 !
        t = "#{min}#{sec}#{msec}"
        regex = Regexp.new("&p ([0-9]+) #{t}")
        playerids = @scoreboards.join('').scan(regex).flatten.uniq
        if playerids.length == 1
          @player = @playernames[playerids.first.to_i]
        end
      end
    end

    @valid = true
  end



  def detect_version(file)
    # try to detect version 8, 9 or 10
    #
    file.pos = 0

    file.pos += 4 # skip 4 byte message length
    file.pos += 1  # skip 1 byte "svc_serverdata"

    # version
    # reads 4 bytes and decodes the little-endian format
    version = file.read(4).unpack('V')[0]

    return version if [8, 9, 10].include? version


    # try to detect version 11
    #
    file.pos = 0

    file.pos += 1  # skip 1 byte "svc_demoinfo"

    file.pos += 4  # skip 4 byte "initial server time"
    file.pos += 4  # skip 4 byte "demo duration in milliseconds"
    file.pos += 4  # skip 4 byte "file offset at which the final -1 was written"

    file.pos += 1  # skip 1 byte "svc_serverdata"

    file.pos += 4  # skip 4 byte "UNKNOWN! - not documented in source, but is needed to fit demos!"

    # version
    # reads 4 bytes and decodes the little-endian format
    version = file.read(4).unpack('V')[0]

    return version if version == 11

    nil # could not detect any known version
  end


  def time_in_msec
    return @time_in_msec unless @time_in_msec.nil?

    # time str to int
    if @time.kind_of? String
      min, sec, msec = @time.scan(/^([0-9]+):([0-9]+)\.([0-9]+)$/).flatten.map { |x| x.to_i }
      @time_in_msec = msec + sec * 1000 + min * 60 * 1000
    end
  end


  def gamemode_wd11(file_content)
    if file_content =~ /cs 12 "(.*?)"/
      $1
    end
  end
end

