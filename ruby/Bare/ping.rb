require '../cpx.rb'
require './config.rb'

c = CPX.new("http://arduinoproxy.tapinsystems.net:8085/cpx",@user,@deviceid);
puts c.sendCommand(c.ping())

