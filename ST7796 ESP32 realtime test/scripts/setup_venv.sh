#!/bin/bash

# Python 가상환경 설정 스크립트

echo "🐍 Python 가상환경 설정 시작..."

# venv 디렉토리가 있으면 삭제
if [ -d "venv" ]; then
    echo "기존 venv 제거 중..."
    rm -rf venv
fi

# 가상환경 생성
echo "가상환경 생성 중..."
python3 -m venv venv

# 가상환경 활성화
echo "가상환경 활성화..."
source venv/bin/activate

# pip 업그레이드
echo "pip 업그레이드..."
pip install --upgrade pip

# 패키지 설치
echo "필요한 패키지 설치 중..."
pip install -r requirements.txt

echo "✅ 가상환경 설정 완료!"
echo ""
echo "사용법:"
echo "  활성화: source venv/bin/activate"
echo "  비활성화: deactivate"
echo ""
echo "스트리밍 실행:"
echo "  python pc_sender.py <ESP32_IP>"