require '../cpx.rb'
require './config.rb'

c = CPX.new(@host,@user,@deviceid);
puts c.sendCommand(c.redisSet(nil,"xxx","100",0));
puts c.sendCommand(c.redisGet(nil,"xxx",0));

