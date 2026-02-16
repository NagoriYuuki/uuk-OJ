# 关于 rootfs 的配置说明


`uuk-OJ` 的 GitHub release 本身不包含完整的 rootfs（用于存放运行环境的子文件系统）  
需要自行配置

```bash
#拉取一个完整的Ubuntu镜像
docker run -itd --name oj-base ubuntu:22.04 bash 

#切换到容器内
docker exec -it oj-base bash 

apt-get update

#安装Python、Java、C++运行环境
apt-get install -y python3 openjdk-17-jdk gcc g++ make 

exit

#创建文件子系统
mkdir src/uuk-judger/rootfs 

# 把容器的文件系统打包导出，并解压到指定目录
docker export oj-base | tar -x -C src/uuk-judger/rootfs

```

现在 `uuk-OJ/src/uuk-judger/rootfs` 内就存在一个完整的Linux系统根目录结构了。