# ESP32 비디오 스트리밍 프로젝트 Makefile

# 변수 설정
PYTHON = python3
CLIENT_DIR = client
VENV = $(CLIENT_DIR)/venv
VENV_PYTHON = $(VENV)/bin/python
VENV_PIP = $(VENV)/bin/pip
ESP32_IP ?= 192.168.1.100

.PHONY: help venv install clean test stream

# 기본 타겟
help:
	@echo "ESP32 비디오 스트리밍 프로젝트"
	@echo ""
	@echo "사용 가능한 명령어:"
	@echo "  make venv       - 가상환경 생성"
	@echo "  make install    - 패키지 설치"
	@echo "  make test       - 연결 테스트 (ESP32_IP=xxx.xxx.xxx.xxx make test)"
	@echo "  make stream     - 스트리밍 시작 (ESP32_IP=xxx.xxx.xxx.xxx make stream)"
	@echo "  make clean      - 가상환경 및 캐시 제거"
	@echo ""
	@echo "예시:"
	@echo "  ESP32_IP=192.168.1.100 make test"
	@echo "  ESP32_IP=192.168.1.100 make stream"

# 가상환경 생성
venv:
	@echo "🐍 가상환경 생성 중..."
	@$(PYTHON) -m venv $(VENV)
	@echo "✅ 가상환경 생성 완료"

# 패키지 설치
install: venv
	@echo "📦 패키지 설치 중..."
	@$(VENV_PIP) install --upgrade pip
	@$(VENV_PIP) install -r $(CLIENT_DIR)/requirements.txt
	@echo "✅ 패키지 설치 완료"

# 연결 테스트
test: venv
	@echo "🔍 ESP32 연결 테스트 (IP: $(ESP32_IP))"
	@$(VENV_PYTHON) $(CLIENT_DIR)/test_connection.py $(ESP32_IP)

# 스트리밍 시작
stream: venv
	@echo "📹 스트리밍 시작 (IP: $(ESP32_IP))"
	@$(VENV_PYTHON) $(CLIENT_DIR)/pc_sender.py $(ESP32_IP)

# 클린업
clean:
	@echo "🧹 정리 중..."
	@rm -rf $(VENV)
	@find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@find . -type f -name "*.pyc" -delete 2>/dev/null || true
	@echo "✅ 정리 완료"