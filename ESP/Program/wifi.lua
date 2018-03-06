print("wifi setup start");
wifi_connect_event = function(T) 
  print("Wifi - Connection to AP("..T.SSID..") established!")
end

wifi_got_ip_event = function(T) 
    print("Wifi - IP address is: "..T.IP)
end

wifi_disconnect_event = function(T)
  if T.reason == wifi.eventmon.reason.ASSOC_LEAVE then     --the station has disassociated from a previously connected AP
    return 
  end
  
  for key,val in pairs(wifi.eventmon.reason) do
    if val == T.reason then
      print("Wifi - Disconnect reason: "..val.."("..key..")")
      break
    end
  end 
end

wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, wifi_connect_event)
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, wifi_got_ip_event)
wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, wifi_disconnect_event)
print("wifi setup end");
