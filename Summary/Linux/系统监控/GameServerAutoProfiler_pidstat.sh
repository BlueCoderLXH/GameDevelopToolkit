#!/bin/bash

# 监听进程名
GameServerName="GameServer-Linux-Test"
#GameServerName="supervisor"

# 监听总时长
ListenTime=900

# 监听失败后的超时等待时间(s)
MaxWaitTime=10

StartListenTime=$(date +%s)
LastListenTimePassBy=0

GameServerPid=""

# 监听GameServer进程启动
#while [ -z "$(pidof $GameServerName)" ]; do
while [ ${#GameServerPid} -le 0 ]; do
  GameServerPid=$(pidof $GameServerName)
    
  ListenTimePassBy=$(($(date +%s) - $StartListenTime))

  if [ $ListenTimePassBy -gt $LastListenTimePassBy ]; then
    echo "Process named 'GameServer' is not found! Listening time passby:$ListenTimePassBy"
    LastListenTimePassBy=$ListenTimePassBy
  fi

  # 超时退出
  if [ $ListenTimePassBy -ge $MaxWaitTime ]; then
    echo "Time out to waiting for process 'GameServer' launching!"
    exit 0
  fi
done

echo "Profiling PidName:$GameServerName Pid:$GameServerPid ..."

datetime=$(date "+%Y%m%d-%H%M%S")

outputFilePathRoot="/mnt/f/TGame/server_out/LinuxServer/Game/Saved/Profiling/"
#outputFilePathRoot="/mnt/f/TGame/server_out/Test/"
originalFile="${outputFilePathRoot}GameServerCpuUsage-Pid${GameServerPid}_${datetime}.txt"

# 监听GameServer启动后前60秒的cpu占用数据
pidstat -u -p $GameServerPid 1 $ListenTime >$originalFile

# ---------- 采集数据输出到文件 ----------
lineNumber=0
validDataLineNumber=0

maxCPUUsage=0
avgCPUUsage=0

while read lineString; do
  # 跳过空行
  if [ ${#lineString} -le 1 ]; then
    continue
  fi

  ((lineNumber++))

  lineStrArray=($lineString)

  echo "lineStrArray[0]:${lineStrArray[0]}"

  # 输出数据
  if [ ${lineStrArray[0]} != "Average:" ] && [ $lineNumber -gt 2 ] && [ $lineNumber -lt $((ListenTime + 2)) ]; then
    cpuUsage=$(echo "${lineStrArray[7]}" | bc -l)

    ((validDataLineNumber++))

    avgCPUUsage=$(echo "$avgCPUUsage+$cpuUsage" | bc -l)

    # 计算MaxCpuUsage
    result=$(echo "$cpuUsage>$maxCPUUsage" | bc -l)
    if [ $result -eq 1 ]; then
      maxCPUUsage=$cpuUsage
    fi
  fi
done < <(cat "$originalFile")

# 计算AvgCPUUsage
avgCPUUsage=$(echo "scale=2;$avgCPUUsage/$validDataLineNumber" | bc -l)

echo "" >>$originalFile
echo "AvgCPUUsage: $avgCPUUsage" >>$originalFile
echo "MaxCPUUsage: $maxCPUUsage" >>$originalFile

echo "Profiling PidName:$GameServerName Pid:$GameServerPid Done"
