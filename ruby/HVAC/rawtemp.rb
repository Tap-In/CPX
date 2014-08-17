require '../cpx.rb'
require './config.rb'

c = CPX.new(@host,@user,@deviceid);
rc = c.sendCommand(c.analogRead(nil,68));

puts "Raw temp: " + rc

