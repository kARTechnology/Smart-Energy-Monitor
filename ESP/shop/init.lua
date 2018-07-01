enduser=0
pin1=3
pin2=4
gpio.mode(pin1, gpio.OUTPUT)
gpio.write(pin1, gpio.HIGH)
gpio.mode(pin2, gpio.OUTPUT)
gpio.write(pin2, gpio.HIGH)
print("init begin") 
--dofile("wifi.lua")  
 
------------------
print("Startup will resume momentarily")
boottime=0
tmr.create():alarm(5000, tmr.ALARM_SINGLE, function()
    sntp.sync(nil,function(sec, usec, server, info) print('sync', sec, usec, server)  boottime=rtctime.get() end,function() print('failed to sync time!')end)
    dofile("functions.lua") 
    dofile("webpage.lua")   
    dofile("http.lua") 
end)

if(enduser==1) then
    tmr.create():alarm(1000, tmr.ALARM_SINGLE, function()
    enduser_setup.start(
      function()
        print("Connected to wifi as:" .. wifi.sta.getip())
      end,
      function(err, str)
        print("enduser_setup: Err #" .. err .. ": " .. str)
        end
    );     
    end)    
    tmr.create():alarm(60000, tmr.ALARM_SINGLE, function()
    print("endusersetup stopped")
    enduser_setup.stop()
    end)
else
    print("endusersetup disabled")
end

print("init end")
