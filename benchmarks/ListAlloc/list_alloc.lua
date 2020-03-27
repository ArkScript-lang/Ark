table.reduce = function (list, fn) 
    local acc
    for k, v in ipairs(list) do
        if 1 == k then
            acc = v
        else
            acc = fn(acc, v)
        end 
    end 
    return acc 
end

local function allocate()
  data = {}
  for i=1,1000 do
    table.insert(data, 0)
  end
end

local socket=require 'socket'

results = {}
for i=1,125 do
  start = socket.gettime()
  allocate()
  table.insert(results, (socket.gettime() - start) * 1000)
end

local function quantile(t, q)
  assert(t ~= nil, "No table provided to quantile")
  assert(q >= 0 and q <= 1, "Quantile must be between 0 and 1")
  table.sort(t)
  local position = #t * q + 0.5
  local mod = position % 1

  if position < 1 then 
    return t[1]
  elseif position > #t then
    return t[#t]
  elseif mod == 0 then
    return t[position]
  else
    return mod * t[math.ceil(position)] +
           (1 - mod) * t[math.floor(position)] 
  end 
end 

local function median(t)
  assert(t ~= nil, "No table provided to median")
  return quantile(t, 0.5)
end

function standardDeviation(t, m)
  local vm
  local sum = 0
  local count = 0
  local result
  
  for k,v in pairs(t) do
    if type(v) == 'number' then
      vm = v - m
      sum = sum + (vm * vm)
      count = count + 1
    end
  end
  
  result = math.sqrt(sum / (count-1))
  
  return result
end

sum = table.reduce(results, function (a, b) return a + b end)
mean = sum / 125
print("Mean time:", mean, "ms")

print("Median time:", median(results), "ms")
print("Stddev:", standardDeviation(results, mean), "ms")