print("init begin")
dofile("i2c.lua") 
 ------------------
net.dns.setdnsserver("8.8.4.4", 0)
net.dns.setdnsserver("8.8.8.8", 1)


wifi_connect_event = function(T) 
  print("Connection to AP("..T.SSID..") established!")
  print("Waiting for IP address...")
  if disconnect_ct ~= nil then disconnect_ct = nil end  
end

wifi_got_ip_event = function(T) 
  -- Internet connectivity can be determined with net.dns.resolve().    
  print("Wifi connection is ready! IP address is: "..T.IP)
 -- print("Startup will resume momentarily, you have 3 seconds to abort.")
  
--  tmr.unregister(1)
 --  tmr.create():alarm(3000, tmr.ALARM_SINGLE, 
 --  function() dofile("mqtt.lua") end)
end

wifi_disconnect_event = function(T)
  if T.reason == wifi.eventmon.reason.ASSOC_LEAVE then 
    --the station has disassociated from a previously connected AP
    return 
  end
  
  for key,val in pairs(wifi.eventmon.reason) do
    if val == T.reason then
      print("Disconnect reason: "..val.."("..key..")")
      break
    end
  end 
end
 

wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, wifi_connect_event)
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, wifi_got_ip_event)
wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, wifi_disconnect_event)

print("Startup will resume momentarily, you have 5 seconds to start endusersetup.")
  
tmr.create():alarm(5000, tmr.ALARM_SINGLE, function()
if(recvMessage() == "endusersetup") 
    then 
    print("endusersetup started")
     enduser_setup.start(
      function() print("Connected to wifi as:" .. wifi.sta.getip())
      sendMessage("01")
       dofile("mqtt.lua")
       end, 
      function(err, str) print("enduser_setup: Err #" .. err .. ": " .. str)end,
      print -- Lua print function can serve as the debug callback
     )      
     else     
     print("normal startup")
     sendMessage("01")
     dofile("mqtt.lua") 
     end
end)


print("init end")
