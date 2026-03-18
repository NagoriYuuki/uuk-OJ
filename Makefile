# Makefile for UUK-OJ

# 镜像名称
BACKEND_IMG := uuk-oj-backend
JUDGER_IMG := uuk-oj-judger

.PHONY: all build up down logs clean restart rebuild frontend

# 默认目标
all: up

# 1. 编译前端
frontend:
	@echo ">>> Building Frontend..."
	cd frontend && npm install && npm run build

# 2. 构建 Docker 镜像
build: frontend
	@echo ">>> Packaging RootFS..."
	# 如果压缩包不存在，或者文件夹比压缩包新，则重新打包
	cd src/uuk-judger && sudo tar -czf rootfs.tar.gz rootfs/
	@echo ">>> Building Docker Images..."
	docker-compose build

# 3. 启动所有服务
up:
	@echo ">>> Starting Services..."
	docker-compose up -d
	@echo ">>> System is running at http://localhost:18080"

# 4. 停止服务
down:
	@echo ">>> Stopping Services..."
	docker-compose down

# 5. 查看日志
logs:
	docker-compose logs -f

# 6. 重新构建并启动 (修改代码后用这个)
rebuild: down build up

# 7. 清理所有数据
clean: down
	@echo ">>> Cleaning Data Volumes..."
	docker volume rm uuk-oj_mysql_data || true
	# docker volume rm uuk-oj_kafka_data || true

# 8. 单独查看评测机日志
logs-judger:
	docker-compose logs -f judger

# 9. 进入后端容器 (调试用)
shell-backend:
	docker-compose exec backend /bin/bash