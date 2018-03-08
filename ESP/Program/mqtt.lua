print("mqtt setup start");
sendMessage("01")
connected=0
r1=0
r2=0
r3=0
kwh1=0
kwh2=0


m = mqtt.Client("clientid", 30, "A1E-a3nUoARivVvZb5G0ZucYlX4WPeFyTB", "")
 
m:on("message", 
 function(client, topic, data) 
  print("--received message")
  print(topic .. ":" ) 
  if data ~= nil then
    print(data) 
        if( string.match(topic, "s1") ) then    sendMessage("1".. data); r1=data
    elseif( string.match(topic, "s2") ) then    sendMessage("2".. data); r2=data
    elseif( string.match(topic, "s3") ) then    sendMessage("3".. data); r3=data
    elseif( string.match(topic, "kwh1") ) then 
  if(kwh1==0) then  kwh1=data end
     
    elseif( string.match(topic, "kwh2") ) then   
  if(kwh2==0) then kwh2=data    end
     
    end 
  end
 end
)
  

function handle_mqtt_error(client, reason) 
 print("--Disconnected - failed reason: " .. reason) 
 connected=0
 tmr.create():alarm(10 * 5000, tmr.ALARM_SINGLE, do_mqtt_connect)
end

function do_mqtt_connect()
    m:connect("things.ubidots.com",1883, 0, 
    function(client)
        print("--Connected to Ubidots!")  
        connected=1
        
            client:subscribe(
            {
              ["/v1.6/devices/sem/s1/lv"]=0,
              ["/v1.6/devices/sem/s2/lv"]=0,
              ["/v1.6/devices/sem/s3/lv"]=0,
              ["/v1.6/devices/sem/kwh1/lv"]=0,
              ["/v1.6/devices/sem/kwh2/lv"]=0
            },function(client) print("--subscribe success") end) 
    end,
    handle_mqtt_error)
end

do_mqtt_connect()
print("mqtt setup end");

timeinterval=5

tmr.alarm(0,timeinterval*1000,1,function()
if connected==1 then getValues() end
end)


 
function getValues()
    msg = recvMessage()
    if(msg=="Waiting for net") then 
        sendMessage("01") 
        sendMessage("1"..r1) 
        sendMessage("2"..r2) 
        sendMessage("3"..r3) 
    else
    if(msg~="") then 
        val={}    
        val.v,val.c1,val.c2,val.m = msg:match("([^,]+),([^,]+),([^,]+),([^,]+)")
        
        if(kwh1~=0) then
            kwh1=kwh1+((val.c1*timeinterval)/3600)/1000 
            val.kwh1=kwh1
        end
        if(kwh2~=0) then 
            kwh2=kwh2+((val.c2*timeinterval)/3600)/1000 
            val.kwh2=kwh2 
            val.kwh1=kwh1 
        end
        
        if((tonumber(val.c2)>50 or tonumber(val.c1) > 50 )and val.m=="0") then 
            print("motion not detected and power being consumed.....")
            val.m=1
        else 
            val.m=0 
        end 

        if(tonumber(val.v)>275) then 
            print("high voltage - turnng off switches.....")
            val.s1=0
            val.s2=0
            val.s3=0
        end
        
        if(val~="[]") then
            val=sjson.encode(val) 
            print(val)
            m:publish("/v1.6/devices/sem", val, 0, 0, function(client) print("sent") end) 
        end
      end
end
end