require '../cpx.rb'
require './config.rb'

c = CPX.new(@host,@user,@deviceid);
rc = c.sendCommand(c.call(nil,"IOTstatus",""));

puts "The status: " + rc

