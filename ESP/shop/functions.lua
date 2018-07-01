
 function explode(div,str) -- credit: http://richard.warburton.it
  if (div=='') then return false end
  local pos,arr = 0,{}
  -- for each divider found
  for st,sp in function() return string.find(str,div,pos,true) end do
    table.insert(arr,string.sub(str,pos,st-1)) -- Attach chars left of current divider
    pos = sp + 1 -- Jump past current divider
  end
  table.insert(arr,string.sub(str,pos)) -- Attach chars right of last divider
  return arr
end
 
 function saveSchedule(state)
    schfd = file.open("scheduleState", "w+")
    if schfd then        
          schfd:write(state)
          schfd:close(); schfd = nil print("saveSchedule",state)
          if(state==1) then startCron() else cron.reset() end
    end         
end

function getSchedule()
    local state=0
    schfd = file.open("scheduleState", "r")
    if schfd then
      state=schfd:read()
      schfd:close(); schfd = nil 
    end
    print("getSchedule",state)
    return tonumber(state)
end

function saveGpio(pin,state)
    pinfd = file.open("pinState"..pin, "w+")
    if pin then
      pinfd:write(state)
      pinfd:close(); pinfd = nil print("saveGpio",pin,state)
      gpio.write(pin,  tonumber(state))
    end
end

function readGpio(pin)
    local state=0
    pinfd = file.open("pinState"..pin, "r")
    if pinfd then
      state=pinfd:read()
      pinfd:close(); pinfd = nil print("readGpio",pin,state)
    end
    return state
end

function saveCron(cronData) 
     fd = file.open("cronData", "w+")
        if fd then
          fd:write(cronData)
          fd:close()  print("saveCron",cronData) cronData=nil
        end   
end

function readCron() 
    local cronData=""
    cronfd = file.open("cronData", "r")
    if cronfd then
      cronData=cronfd:read()
      cronfd:close(); cronfd = nil --print("readCron",cronData)
    end
    if(cronData~=nil) then return cronData else return "" end
end


function verifyandStartCron() 
    encoded=string.sub(encoded,string.find(encoded,"val=")+4,-1) 
    encoded=string.gsub(encoded, "+", " ")
    encoded=string.gsub(encoded, "%%2F", "/") 
    encoded=string.gsub(encoded, "%%2C", ",") 
    encoded=string.gsub(encoded, "%%0D%%0A", "\r\n")
    
    t=explode("\r\n",encoded)
    cron.reset()
    local validatedLines=""
    for k,v in pairs(t) do 
        if(v~='') then
            min,hour,dayofmon,mon,dayofweek,state, pin =v:match("(.+) (.+) (.+) (.+) (.+) (.+) (.+)")
            sch= min.." "..hour.." ".. dayofmon.." "..mon.. " " ..dayofweek
                if(pin=="1" and state=="1") then cron.schedule(sch, function(e) print("pin1,on")  saveGpio(pin1,1) end) if(validatedLines=="") then validatedLines=v else validatedLines=validatedLines.."\r\n"..v end
            elseif(pin=="1" and state=="0") then cron.schedule(sch, function(e) print("pin1,off")  saveGpio(pin1,0) end) if(validatedLines=="") then validatedLines=v else validatedLines=validatedLines.."\r\n"..v end
            elseif(pin=="2" and state=="1") then cron.schedule(sch, function(e) print("pin2,on")   saveGpio(pin2,1) end) if(validatedLines=="") then validatedLines=v else validatedLines=validatedLines.."\r\n"..v end
            elseif(pin=="2" and state=="0") then cron.schedule(sch, function(e) print("pin2,off")   saveGpio(pin2,0)end) if(validatedLines=="") then validatedLines=v else validatedLines=validatedLines.."\r\n"..v end
            end 
        end
    end 
    t=nil
    encoded=nil
    saveCron(validatedLines)
end

 function startCron() 
    if file.open("cronData") then
        cronData=file.read()
        file.close()   
        
    t=explode("\r\n",cronData)
    cron.reset()

    for k,v in pairs(t) do 
        if(v~='') then
            min,hour,dayofmon,mon,dayofweek,state, pin =v:match("(.+) (.+) (.+) (.+) (.+) (.+) (.+)")
            sch= min.." "..hour.." ".. dayofmon.." "..mon.. " " ..dayofweek
                if(pin=="1" and state=="1") then cron.schedule(sch, function(e) print("pin1,on")   saveGpio(pin1,1) end) 
            elseif(pin=="1" and state=="0") then cron.schedule(sch, function(e) print("pin1,off")  saveGpio(pin1,0) end) 
            elseif(pin=="2" and state=="1") then cron.schedule(sch, function(e) print("pin2,on")    saveGpio(pin2,1) end)
            elseif(pin=="2" and state=="0") then cron.schedule(sch, function(e) print("pin2,off")   saveGpio(pin2,0) end)
            end 
        end
    end
end
print("CronJob Started")
end
 