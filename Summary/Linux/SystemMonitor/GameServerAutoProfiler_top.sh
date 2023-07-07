#!/bin/bash

# 监听进程名
GameServerName="GameServer-Linux-Test"

# 监听总时长
ListenTime=60

# 监听失败后的超时等待时间(s)
MaxWaitTime=10

StartListenTime=$(date +%s)
LastListenTimePassBy=0

# 监听GameServer进程启动
while [ -z "$(pidof $GameServerName)" ]; do
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

GameServerPid=$(pidof $GameServerName)

echo "Profiling PidName:$GameServerName Pid:$GameServerPid ..."

datetime=$(date "+%Y%m%d-%H%M%S")

outputFilePathRoot="/mnt/f/TGame/server_out/LinuxServer/Game/Saved/Profiling/"
#outputFilePathRoot=""
originalFile="${outputFilePathRoot}[Temp]GameServerCpuUsage_$datetime.txt"

# 监听GameServer启动后前60秒的cpu占用数据
top -p $GameServerPid -b -d 1 -n $ListenTime >$originalFile

# ---------- 采集数据输出到文件 ----------
lineNumber=0
validDataLineNumber=0

targetFile="${outputFilePathRoot}GameServerCpuUsage_$datetime.txt"

maxCPUUsage=0
avgCPUUsage=0

while read lineString; do
  # 跳过空行
  if [ ${#lineString} -le 1 ]; then
    continue
  fi

  ((lineNumber++))

  # 输出标题1
  if [ $lineNumber -eq 1 ]; then
    echo "$lineString" >>$targetFile
    echo "" >>$targetFile
  fi

  # 输出标题2
  if [ $lineNumber -eq 6 ]; then
    echo "$lineString" >>$targetFile
  fi

  # 输出数据
  if [ $(($lineNumber % 7)) -eq 0 ]; then
    # echo "[$((lineNumber/7))}]:${lineString}" >> ${targetFile}

    echo "$lineString" >>$targetFile

    lineStrArray=($lineString)
    cpuUsage=$(echo "${lineStrArray[8]}" | bc -l)

    # 排除0
    result=$(echo "$cpuUsage>0" | bc -l)
    if [ $result -eq 1 ]; then
      ((validDataLineNumber++))

      avgCPUUsage=$(echo "$avgCPUUsage+$cpuUsage" | bc -l)

      # 计算MaxCpuUsage
      result=$(echo "$cpuUsage>$maxCPUUsage" | bc -l)
      # echo "cpuUsage:$cpuUsage>maxUsage:$maxCPUUsage result:$result"
      if [ $result -eq 1 ]; then
        maxCPUUsage=$cpuUsage
      fi
    fi
  fi

  # echo "[${lineNumber} - ${#lineString}]:${lineString}"
done < <(cat "$originalFile")

# 计算AvgCPUUsage
avgCPUUsage=$(echo "scale=2;$avgCPUUsage/$validDataLineNumber" | bc -l)

echo "" >>$targetFile
echo "MaxCPUUsage: $maxCPUUsage" >>$targetFile
echo "AvgCPUUsage: $avgCPUUsage" >>$targetFile

# 删除临时文件
rm -f $originalFile
