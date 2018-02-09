print("init begin")
dofile("i2c.lua") 
dofile("wifi.lua") 
------------------

print("Startup will resume momentarily, you have 10 seconds to start endusersetup.")
  
tmr.create():alarm(10000, tmr.ALARM_SINGLE, function()
if(recvMessage() == "endusersetup") 
    then 
        print("endusersetup started")
        enduser_setup.start(
        function() 
            print("endusersetup success")
            print("Connected to wifi as:" .. wifi.sta.getip())
            dofile("mqtt.lua")
        end, 
        function(err, str) print("enduser_setup: Err #" .. err .. ": " .. str)end,
            print -- Lua print function can serve as the debug callback
        )      
    else     
        print("normal startup")
        dofile("mqtt.lua") 
    end
end)

print("init end")
