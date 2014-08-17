require '../cpx.rb'
require './config.rb'

c = CPX.new(@host,@user,@deviceid);
rc = c.sendCommand(c.call(nil,"IOTReadTemperature",""));

puts "The temp: " + rc

